package v1

import (
	"encoding/json"
	"log"
	"net/http"
	"time"

	"github.com/ashep/d5y/geoip"
	"github.com/ashep/d5y/weather"
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
	geoIP   *geoip.GeoIP
	weather *weather.Client
}

func New(gi *geoip.GeoIP, wth *weather.Client) *Handler {
	return &Handler{
		geoIP:   gi,
		weather: wth,
	}
}

func (h *Handler) Handle(w http.ResponseWriter, r *http.Request) {
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

	geo, err := h.geoIP.Get(rAddr)
	if err != nil {
		log.Printf("geoip error: %v", err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	log.Printf("%s %s remote=%q country=%q region=%q city=%q tz=%q ua=%q",
		r.Method, r.RequestURI, rAddr, geo.CountryName, geo.RegionName, geo.City, geo.Timezone, r.Header.Get("User-Agent"))

	tz, err := time.LoadLocation(geo.Timezone)
	if err != nil {
		log.Printf("failed to determine time zone: %v", err)
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

	log.Printf("%s", d)
}
