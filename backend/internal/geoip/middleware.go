package geoip

import (
	"context"
	"net/http"

	"github.com/rs/zerolog"

	"github.com/ashep/d5y/internal/httputil"
)

var ctxKey = struct{}{}

func WrapHTTP(next http.HandlerFunc, svc *Service, l zerolog.Logger) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		rAddr, err := httputil.RemoteAddr(r)
		if err != nil {
			l.Error().Err(err).Msg("get remote address failed")
			next.ServeHTTP(w, r)
			return
		}

		data, err := svc.Get(rAddr)
		if err != nil {
			l.Error().Err(err).Msg("get geoip data failed")
			next.ServeHTTP(w, r)
			return
		}

		next.ServeHTTP(w, r.Clone(context.WithValue(context.Background(), ctxKey, data)))
	}
}

func FromCtx(ctx context.Context) *Data {
	d, ok := ctx.Value(ctxKey).(*Data)
	if !ok {
		return nil
	}

	return d
}
