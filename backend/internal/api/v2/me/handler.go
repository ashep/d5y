package me

import (
	"encoding/json"
	"net/http"
	"time"

	"github.com/rs/zerolog"

	"github.com/ashep/d5y/internal/auth"
	"github.com/ashep/d5y/internal/geoip"
	"github.com/ashep/d5y/internal/httputil"
	"github.com/ashep/d5y/internal/tz"
	"github.com/ashep/d5y/internal/weather"
)

type ResponseTimestamp struct {
	TZ     string `json:"tz"`
	TZData string `json:"tz_data"`
	Value  int64  `json:"value"`
}

type ResponseGeo struct {
	Country  string `json:"country"`
	Region   string `json:"region"`
	City     string `json:"city"`
	Timezone string `json:"timezone"`
}

type Response struct {
	Timestamp *ResponseTimestamp `json:"timestamp,omitempty"`
	Geo       *ResponseGeo       `json:"geo,omitempty"`
	Weather   weather.Data       `json:"weather,omitempty"`
}

type Handler struct {
	geoIP   *geoip.Service
	weather *weather.Client
	l       zerolog.Logger
}

func New(gi *geoip.Service, wth *weather.Client, l zerolog.Logger) *Handler {
	return &Handler{
		geoIP:   gi,
		weather: wth,
		l:       l,
	}
}

func (h *Handler) Handle(w http.ResponseWriter, r *http.Request) {
	tZone := r.URL.Query().Get("tz")

	res := &Response{
		Timestamp: &ResponseTimestamp{
			Value: time.Now().Unix(),
		},
	}

	if tZone != "" {
		res.Timestamp.TZ = tZone
		res.Timestamp.TZData = tz.ToPosix(tZone)
	}

	rAddr, err := httputil.RemoteAddr(r)
	if err != nil {
		h.l.Info().
			Str("method", r.Method).
			Str("uri", r.RequestURI).
			Str("ua", r.Header.Get("User-Agent")).
			Str("client_id", auth.CtxToken(r.Context())).
			Msg("request")

		h.l.Error().Err(err).Msg("remote address get failed")

		h.writeResponse(res, w)
		return
	}

	geo := geoip.FromCtx(r.Context())
	if geo == nil {
		h.l.Info().
			Str("method", r.Method).
			Str("uri", r.RequestURI).
			Str("remote", rAddr).
			Str("ua", r.Header.Get("User-Agent")).
			Str("client_id", auth.CtxToken(r.Context())).
			Msg("request")

		h.l.Error().Err(err).Msg("geoip get failed")

		h.writeResponse(res, w)
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
		Str("client_id", auth.CtxToken(r.Context())).
		Msg("request")

	res.Geo = &ResponseGeo{
		Country:  geo.CountryName,
		Region:   geo.RegionName,
		City:     geo.City,
		Timezone: geo.Timezone,
	}

	if tZone == "" {
		res.Timestamp.TZ = geo.Timezone
		res.Timestamp.TZData = tz.ToPosix(geo.Timezone)
	}

	wData, err := h.weather.GetForIPAddr(rAddr)
	if err != nil {
		h.l.Error().Err(err).Msg("weather get failed")
	} else {
		res.Weather = wData
	}

	h.writeResponse(res, w)
}

func (h *Handler) writeResponse(r *Response, w http.ResponseWriter) {
	d, err := json.Marshal(r)
	if err != nil {
		h.l.Error().Err(err).Msg("response marshal error")
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	if _, err = w.Write(d); err != nil {
		h.l.Error().Err(err).Msg("response write error")
		return
	}

	h.l.Info().RawJSON("data", d).Msg("response")
}
