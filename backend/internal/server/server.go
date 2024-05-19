package server

import (
	"context"
	"net/http"

	"github.com/rs/zerolog"

	"github.com/ashep/d5y/internal/api/auth"
	"github.com/ashep/d5y/internal/geoip"
	"github.com/ashep/d5y/internal/weather"

	handlerV1 "github.com/ashep/d5y/internal/api/v1"
	handlerV2 "github.com/ashep/d5y/internal/api/v2"
)

type Server struct {
	s *http.Server
	l zerolog.Logger
}

func New(addr, weatherAPIKey string, l zerolog.Logger) *Server {
	mux := http.NewServeMux()

	gi := geoip.New()
	wth := weather.New(weatherAPIKey)

	hv1 := handlerV1.New(gi, wth, l)
	mux.HandleFunc("/", hv1.Handle)
	mux.HandleFunc("/v1", hv1.Handle)

	hv2 := handlerV2.New(gi, wth, l)
	mux.Handle("/v2/me", auth.WrapHandler(hv2.HandleMe))

	return &Server{
		s: &http.Server{Addr: addr, Handler: mux},
		l: l,
	}
}

func (s *Server) Run() error {
	s.l.Info().Str("addr", s.s.Addr).Msg("server starting")
	return s.s.ListenAndServe()
}

func (s *Server) Shutdown(ctx context.Context) {
	s.l.Info().Msg("shutting down the server")

	err := s.s.Shutdown(ctx)
	if err != nil {
		s.l.Error().Err(err).Msg("server shutdown failed")
	}

	s.l.Info().Msg("server stopped")
}
