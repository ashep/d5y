package auth

import (
	"context"
	"net/http"
	"strings"
)

var httpCtxKey = struct{}{}

func WrapHandler(next http.HandlerFunc) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		tok := strings.TrimPrefix(r.Header.Get("Authorization"), "Bearer")
		if tok == "" {
			w.WriteHeader(http.StatusUnauthorized)
			return
		}

		tok = strings.TrimSpace(tok)
		r = r.Clone(context.WithValue(context.Background(), httpCtxKey, tok))

		next.ServeHTTP(w, r)
	})
}

func Token(ctx context.Context) string {
	tok, ok := ctx.Value(httpCtxKey).(string)

	if !ok {
		return ""
	}

	return tok
}
