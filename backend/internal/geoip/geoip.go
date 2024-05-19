// Author:  Oleksandr Shepetko
// Email:   a@shepetko.com
// License: MIT

package geoip

import (
	"encoding/json"
	"sync"

	"github.com/ashep/d5y/internal/httpcli"
)

type Service struct {
	cli   *httpcli.Client
	cache map[string]*Data
	mux   *sync.Mutex
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

func New() *Service {
	return &Service{
		cli:   httpcli.New(),
		cache: make(map[string]*Data),
		mux:   &sync.Mutex{},
	}
}

func (s *Service) Get(addr string) (*Data, error) {
	s.mux.Lock()
	defer s.mux.Unlock()

	d, ok := s.cache[addr]
	if ok {
		return d, nil
	}

	d = &Data{}
	err := s.cli.GetJSON("http://ip-api.com/json/"+addr, d)
	if err != nil {
		return nil, err
	}

	s.cache[addr] = d

	return d, nil
}
