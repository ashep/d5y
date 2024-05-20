package auth

import (
	"context"
	"net/http"
	"strings"
)

var ctxKey = struct{}{}

func WrapHTTP(next http.HandlerFunc) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		tok := strings.TrimPrefix(r.Header.Get("Authorization"), "Bearer")
		if tok == "" {
			w.WriteHeader(http.StatusUnauthorized)
			return
		}

		next.ServeHTTP(w, r.Clone(context.WithValue(context.Background(), ctxKey, strings.TrimSpace(tok))))
	}
}

func CtxToken(ctx context.Context) string {
	tok, ok := ctx.Value(ctxKey).(string)

	if !ok {
		return ""
	}

	return tok
}
