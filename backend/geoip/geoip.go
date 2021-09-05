// Author:  Alexander Shepetko
// Email:   a@shepetko.com
// License: MIT

package geoip

import (
	"github.com/ashep/cronus/httpcli"
)

type GeoIP struct {
	cli   *httpcli.Client
	cache map[string]*Data
}

type Data struct {
	City        string  `json:"city,omitempty"`
	CountryCode string  `json:"country_code,omitempty"`
	CountryName string  `json:"country_name,omitempty"`
	Ip          string  `json:"ip,omitempty"`
	Latitude    float64 `json:"latitude,omitempty"`
	Longitude   float64 `json:"longitude,omitempty"`
	MetroCode   float64 `json:"metro_code,omitempty"`
	RegionCode  string  `json:"region_code,omitempty"`
	TimeZone    string  `json:"time_zone,omitempty"`
	ZipCode     string  `json:"zip_code,omitempty"`
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
	err := g.cli.GetJSON("https://freegeoip.app/json/"+addr, d)
	if err != nil {
		return nil, err
	}

	g.cache[addr] = d

	return d, nil
}
