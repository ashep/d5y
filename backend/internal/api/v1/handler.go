package v1

import (
	"encoding/json"
	"net/http"
	"time"

	"github.com/rs/zerolog"

	"github.com/ashep/d5y/internal/geoip"
	"github.com/ashep/d5y/internal/httputil"
	"github.com/ashep/d5y/internal/weather"
)

type Response struct {
	Second int `json:"second"`
	Minute int `json:"minute"`
	Hour   int `json:"hour"`
	Dow    int `json:"dow"`
	Day    int `json:"day"`
	Month  int `json:"month"`
	Year   int `json:"year"`

	Weather   bool    `json:"weather"`
	Temp      float64 `json:"temp"`
	FeelsLike float64 `json:"feels_like"`
}

type Handler struct {
	geoIP   *geoip.Service
	weather *weather.Client
	l       zerolog.Logger
}

func New(g *geoip.Service, w *weather.Client, l zerolog.Logger) *Handler {
	return &Handler{
		geoIP:   g,
		weather: w,
		l:       l,
	}
}

func (h *Handler) Handle(w http.ResponseWriter, r *http.Request) {
	rAddr, err := httputil.RemoteAddr(r)
	if err != nil {
		h.l.Info().
			Str("method", r.Method).
			Str("uri", r.RequestURI).
			Str("ua", r.Header.Get("User-Agent")).
			Msg("request")

		h.l.Error().Err(err).Msg("remote address get failed")

		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	geo := geoip.FromCtx(r.Context())
	if geo == nil {
		h.l.Info().
			Str("method", r.Method).
			Str("uri", r.RequestURI).
			Str("remote", rAddr).
			Str("ua", r.Header.Get("User-Agent")).
			Msg("request")

		h.l.Warn().Err(err).Msg("geoip get failed")

		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	h.l.Info().
		Str("method", r.Method).
		Str("uri", r.RequestURI).
		Str("remote", rAddr).
		Str("country", geo.CountryName).
		Str("region", geo.RegionName).
		Str("city", geo.City).
		Str("tz", geo.Timezone).
		Str("ua", r.Header.Get("User-Agent")).
		Msg("request")

	tz, err := time.LoadLocation(geo.Timezone)
	if err != nil {
		h.l.Warn().Err(err).Msg("time zone detect failed")
	}
	t := time.Now().In(tz)

	// Fix weekday number
	dow := int(t.Weekday())
	if dow == 0 {
		dow = 7
	}

	resp := Response{
		Second: t.Second(),
		Minute: t.Minute(),
		Hour:   t.Hour(),
		Dow:    dow,
		Day:    t.Day(),
		Month:  int(t.Month()),
		Year:   t.Year() - 2000,
	}

	// Add weather data
	weatherData, err := h.weather.GetForIPAddr(rAddr)
	if err == nil {
		resp.Weather = true
		resp.Temp = weatherData["current"].Temp
		resp.FeelsLike = weatherData["current"].FeelsLike
	} else {
		h.l.Error().Err(err).Msg("weather get failed")
	}

	d, err := json.Marshal(resp)
	if err != nil {
		h.l.Error().Err(err).Msg("response marshal failed")
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	if _, err = w.Write(d); err != nil {
		h.l.Error().Err(err).Msg("response write failed")
	}

	h.l.Info().RawJSON("data", d).Msg("response")
}
