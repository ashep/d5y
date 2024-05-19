package tz

import (
	"bytes"
	"embed"
	_ "embed"
	"log"
	"strings"
	"sync"
)

//go:embed zoneinfo
var zoneInfo embed.FS

var cache map[string]string

var mux *sync.Mutex

func ToPosix(s string) string {
	if s == "" {
		return "UTC0"
	}

	if mux == nil {
		mux = new(sync.Mutex)
	}

	mux.Lock()
	defer mux.Unlock()

	if cache == nil {
		cache = make(map[string]string)
	}

	if res, ok := cache[s]; ok {
		return res
	}

	b, err := zoneInfo.ReadFile("zoneinfo/" + s)
	if err != nil {
		log.Printf("tz: failed to read zone data for %s: %v", s, err)
		return "UTC0"
	}

	bs := bytes.SplitN(b, []byte("\n"), -1)
	if len(bs) < 2 {
		log.Printf("tz: failed to read zone data for %s: %v", s, err)
		return "UTC0"
	}

	res := strings.TrimSpace(string(bs[len(bs)-2]))
	cache[s] = res

	return res
}
