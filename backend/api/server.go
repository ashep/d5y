// Author:  Oleksandr Shepetko
// Email:   a@shepetko.com
// License: MIT

package api

import (
	"context"
	"log"
	"net/http"

	handlerV1 "github.com/ashep/d5y/api/v1"
	handlerV2 "github.com/ashep/d5y/api/v2"
	"github.com/ashep/d5y/geoip"
	"github.com/ashep/d5y/weather"
)

type Server struct {
	s *http.Server
}

func New(addr, weatherAPIKey string) *Server {
	mux := http.NewServeMux()

	gi := geoip.New()
	wth := weather.New(weatherAPIKey)

	hv1 := handlerV1.New(gi, wth)
	hv2 := handlerV2.New(gi, wth)

	mux.HandleFunc("/", hv1.HandleRoot)
	mux.HandleFunc("/v2", hv2.HandleRoot)

	return &Server{s: &http.Server{Addr: addr, Handler: mux}}
}

func (s *Server) Run() {
	go func() {
		err := s.s.ListenAndServe()
		log.Printf("%v", err)
	}()

	log.Printf("server started")
}

func (s *Server) Shutdown(ctx context.Context) {
	log.Printf("shutting down the server…")

	err := s.s.Shutdown(ctx)
	if err != nil {
		log.Printf("server shutdown error: %v", err)
	}

	log.Printf("server stopped")
}
