// Author:  Oleksandr Shepetko
// Email:   a@shepetko.com
// License: MIT

package geoip

import (
	"encoding/json"

	"github.com/ashep/d5y/internal/httpcli"
)

type GeoIP struct {
	cli   *httpcli.Client
	cache map[string]*Data
}

type Data struct {
	City        string  `json:"city,omitempty"`
	CountryCode string  `json:"countryCode,omitempty"`
	CountryName string  `json:"country,omitempty"`
	IP          string  `json:"ip,omitempty"`
	Latitude    float64 `json:"lat,omitempty"`
	Longitude   float64 `json:"lon,omitempty"`
	RegionCode  string  `json:"region,omitempty"`
	RegionName  string  `json:"regionName,omitempty"`
	Timezone    string  `json:"timezone,omitempty"`
}

func (d *Data) String() string {
	b, err := json.Marshal(d)
	if err != nil {
		return err.Error()
	}

	return string(b)
}

func New() *GeoIP {
	return &GeoIP{
		cli:   httpcli.New(),
		cache: make(map[string]*Data),
	}
}

func (g *GeoIP) Get(addr string) (*Data, error) {
	d, ok := g.cache[addr]
	if ok {
		return d, nil
	}

	d = &Data{}
	err := g.cli.GetJSON("http://ip-api.com/json/"+addr, d)
	if err != nil {
		return nil, err
	}

	g.cache[addr] = d

	return d, nil
}
