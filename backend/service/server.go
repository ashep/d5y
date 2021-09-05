// Author:  Alexander Shepetko
// Email:   a@shepetko.com
// License: MIT

package service

import (
	"context"
	"github.com/ashep/cronus/geoip"
	"github.com/ashep/cronus/weather"
	"log"
	"net/http"
)

type Service struct {
	server  *http.Server
	geoIP   *geoip.GeoIP
	weather *weather.Weather
}

func New(addr, weatherAPIKey string) *Service {
	mux := http.NewServeMux()

	s := &Service{
		server: &http.Server{
			Addr:    addr,
			Handler: mux,
		},
		geoIP:   geoip.New(),
		weather: weather.New(weatherAPIKey),
	}

	mux.HandleFunc("/", s.RootHandler)

	return s
}

func (s *Service) Run() {
	go func() {
		err := s.server.ListenAndServe()
		log.Printf("%v", err)
	}()

	log.Printf("server started")
}

func (s *Service) Shutdown(ctx context.Context) {
	log.Printf("shutting down server...")

	err := s.server.Shutdown(ctx)
	if err != nil {
		log.Printf("server shutdown error: %v", err)
	}

	log.Printf("server stopped")
}
