package tz

import (
	_ "embed"
	"encoding/json"
	"log"
)

//go:embed tz.json
var raw []byte

var tz map[string]string

func ToPosix(s string) string {
	if tz == nil {
		load()
	}

	r := tz[s]
	if r == "" {
		r = "UTC0"
	}

	return r
}

func load() {
	tz = make(map[string]string)

	if err := json.Unmarshal(raw, &tz); err != nil {
		log.Printf("failed to unmarshal tz data")
		return
	}

}
