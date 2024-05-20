// Author:  Oleksandr Shepetko
// Email:   a@shepetko.com
// License: MIT

package weather

import (
	"fmt"

	"github.com/ashep/d5y/internal/httpcli"
)

type Client struct {
	c      *httpcli.Client
	apiKey string
}

type DataItem struct {
	Title     string  `json:"title"`
	Temp      float64 `json:"temp"`
	FeelsLike float64 `json:"feels_like"`
	Pressure  float64 `json:"pressure"`
	Humidity  float64 `json:"humidity"`
	IconURL   string  `json:"icon"`
}

type Data map[string]DataItem

type wAPIRespCondition struct {
	Code int    `json:"code"`
	Text string `json:"text"`
	Icon string `json:"icon"`
}

type wAPIRespCurrent struct {
	Temp      float64           `json:"temp_c"`
	FeelsLike float64           `json:"feelslike_c"`
	Pressure  float64           `json:"pressure_mb"`
	Humidity  float64           `json:"humidity"`
	Condition wAPIRespCondition `json:"condition"`
}

type wAPIResp struct {
	Current wAPIRespCurrent `json:"current"`
}

func New(apiKey string) *Client {
	return &Client{
		c:      httpcli.New(),
		apiKey: apiKey,
	}
}

func (c *Client) GetForIPAddr(addr string) (Data, error) {
	apiURL := fmt.Sprintf("https://api.weatherapi.com/v1/current.json?key=%s&q=%s", c.apiKey, addr)
	owRes := &wAPIResp{}

	err := c.c.GetJSON(apiURL, owRes)
	if err != nil {
		return nil, err
	}

	res := make(Data)
	res["current"] = DataItem{
		Title:     owRes.Current.Condition.Text,
		IconURL:   owRes.Current.Condition.Icon,
		Temp:      owRes.Current.Temp,
		FeelsLike: owRes.Current.FeelsLike,
		Pressure:  owRes.Current.Pressure,
		Humidity:  owRes.Current.Humidity,
	}

	return res, nil
}
