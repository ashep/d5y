package httputil

import (
	"errors"
	"net/http"
)

func RemoteAddr(r *http.Request) (string, error) {
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
