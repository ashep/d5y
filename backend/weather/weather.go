// Author:  Oleksandr Shepetko
// Email:   a@shepetko.com
// License: MIT

package weather

import (
	"fmt"

	"github.com/ashep/cronus/httpcli"
)

type Weather struct {
	cli    *httpcli.Client
	apiKey string
	units  string
}

type Data struct {
	Temp      float64 `json:"temp"`
	FeelsLike float64 `json:"feels_like"`
	Pressure  float64 `json:"pressure"`
	Humidity  float64 `json:"humidity"`
}

type OWMapResponse struct {
	Main    OWMapResponseMain      `json:"main"`
	Weather []OWMapResponseWeather `json:"weather"`
}

type OWMapResponseMain struct {
	Temp      float64 `json:"temp"`
	FeelsLike float64 `json:"feels_like"`
	TempMin   float64 `json:"temp_min"`
	TempMax   float64 `json:"temp_max"`
	Pressure  float64 `json:"pressure"`
	Humidity  float64 `json:"humidity"`
}

type OWMapResponseWeather struct {
	Id          int    `json:"id"`
	Main        string `json:"main"`
	Description string `json:"description"`
	Icon        string `json:"icon"`
}

func New(apiKey string) *Weather {
	return &Weather{
		cli:    httpcli.New(),
		apiKey: apiKey,
		units:  "metric",
	}
}

func (w *Weather) Get(lat, lon float64) (*Data, error) {
	apiURL := fmt.Sprintf("https://api.openweathermap.org/data/2.5/weather?units=%s&lat=%f&lon=%f&appid=%s",
		w.units, lat, lon, w.apiKey)

	resp := &OWMapResponse{}
	err := w.cli.GetJSON(apiURL, resp)
	if err != nil {
		return nil, err
	}

	d := &Data{
		Temp:      resp.Main.Temp,
		FeelsLike: resp.Main.FeelsLike,
		Pressure:  resp.Main.Pressure,
		Humidity:  resp.Main.Humidity,
	}

	return d, nil
}
