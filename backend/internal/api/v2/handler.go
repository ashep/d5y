package v2

import (
	"net/http"

	"github.com/rs/zerolog"

	"github.com/ashep/d5y/internal/api/v2/me"
	"github.com/ashep/d5y/internal/geoip"
	"github.com/ashep/d5y/internal/weather"
)

type Handler struct {
	me *me.Handler
}

func New(geoIPCli *geoip.Service, weatherCli *weather.Client, l zerolog.Logger) *Handler {
	return &Handler{
		me: me.New(geoIPCli, weatherCli, l),
	}
}

func (h *Handler) HandleMe(w http.ResponseWriter, r *http.Request) {
	h.me.Handle(w, r)
}
