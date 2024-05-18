package v1

import (
	"encoding/json"
	"errors"
	"log"
	"net/http"
	"time"

	"github.com/ashep/d5y/geoip"
	"github.com/ashep/d5y/tz"
	"github.com/ashep/d5y/weather"
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

	rAddr, err := remoteAddr(r)
	if err != nil {
		log.Printf("%s %s ua=%q", r.Method, r.RequestURI, r.Header.Get("User-Agent"))
		log.Printf("get remote addr failed: %v", err)
		h.writeResponse(res, w)
		return
	}

	geo, err := h.geoIP.Get(rAddr)
	if err != nil {
		log.Printf("%s %s remote=%q ua=%q", r.Method, r.RequestURI, rAddr, r.Header.Get("User-Agent"))
		log.Printf("get geo data failed: %v", err)
		h.writeResponse(res, w)
		return
	}

	log.Printf("%s %s remote=%q country=%q region=%q city=%q tz=%q ua=%q",
		r.Method, r.RequestURI, rAddr, geo.CountryName, geo.RegionName, geo.City, geo.Timezone, r.Header.Get("User-Agent"))

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
		log.Printf("failed to get weather: %v", err)
	} else {
		res.Weather = wData
	}

	h.writeResponse(res, w)
}

func remoteAddr(r *http.Request) (string, error) {
	res := r.Header.Get("cf-connecting-ip")

	if res == "" {
		res = r.Header.Get("x-forwarded-for")
	}

	if res == "" {
		res = r.RemoteAddr
	}

	if res == "" {
		return "", errors.New("no remote address found")
	}

	return res, nil
}

func (h *Handler) writeResponse(r *Response, w http.ResponseWriter) {
	d, err := json.Marshal(r)
	if err != nil {
		log.Printf("response marshal error: %v", err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	log.Printf("%s", d)

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	if _, err = w.Write(d); err != nil {
		log.Printf("response write error: %v", err)
	}
}
