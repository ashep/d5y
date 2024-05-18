package v1

import (
	"encoding/json"
	"log"
	"net/http"
	"time"

	"github.com/ashep/d5y/geoip"
	"github.com/ashep/d5y/tz"
	"github.com/ashep/d5y/weather"
)

type ResponseTimestamp struct {
	TZ      string `json:"tz"`
	TZPOSIX string `json:"tz_posix"`

	Second int `json:"second"`
	Minute int `json:"minute"`
	Hour   int `json:"hour"`
	Dow    int `json:"dow"`
	Day    int `json:"day"`
	Month  int `json:"month"`
	Year   int `json:"year"`
}

type ResponseWeather struct {
	Temp      float64 `json:"temp"`
	FeelsLike float64 `json:"feels_like"`
	Humidity  float64 `json:"humidity"`
}

type Response struct {
	Timestamp *ResponseTimestamp `json:"timestamp,omitempty"`
	Weather   *ResponseWeather   `json:"weather,omitempty"`
}

type Handler struct {
	geoIP   *geoip.GeoIP
	weather *weather.Weather
}

func New(gi *geoip.GeoIP, wth *weather.Weather) *Handler {
	return &Handler{
		geoIP:   gi,
		weather: wth,
	}
}

func (h *Handler) HandleRoot(w http.ResponseWriter, r *http.Request) {
	rAddr := r.Header.Get("cf-connecting-ip")
	if rAddr == "" {
		rAddr = r.Header.Get("x-forwarded-for")
	}

	if rAddr == "" {
		rAddr = r.RemoteAddr
	}

	if rAddr == "" {
		log.Printf("failed to determine remote ip address")
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	geoData, err := h.geoIP.Get(rAddr)
	if err != nil {
		log.Printf("geoip error: %v", err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	log.Printf("req: addr=%s; city=%s; ua=%s;", rAddr, geoData.City, r.Header.Get("User-Agent"))

	tzLoc, err := time.LoadLocation(geoData.TimeZone)
	if err != nil {
		log.Printf("failed to determine time zone: %v", err)
	}
	t := time.Now().In(tzLoc)

	// Fix weekday number
	dow := int(t.Weekday())
	if dow == 0 {
		dow = 7
	}

	resp := Response{
		Timestamp: &ResponseTimestamp{
			TZ:      geoData.TimeZone,
			TZPOSIX: tz.ToPosix(geoData.TimeZone),

			Second: t.Second(),
			Minute: t.Minute(),
			Hour:   t.Hour(),
			Dow:    dow,
			Day:    t.Day(),
			Month:  int(t.Month()),
			Year:   t.Year() - 2000,
		},
	}

	weatherData, err := h.weather.Get(geoData.Latitude, geoData.Longitude)
	if err == nil {
		resp.Weather = &ResponseWeather{
			Temp:      weatherData.Temp,
			FeelsLike: weatherData.FeelsLike,
			Humidity:  weatherData.Humidity,
		}
	} else {
		log.Printf("failed to get weather: %v", err)
	}

	d, err := json.Marshal(resp)
	if err != nil {
		log.Printf("response marshal error: %v", err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	if _, err = w.Write(d); err != nil {
		log.Printf("response write error: %v", err)
	}

	log.Printf("rsp: addr=%s; data=%s", rAddr, d)
}
