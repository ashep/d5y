package app

type ServerConfig struct {
	Addr string
}

type WeatherConfig struct {
	APIKey string
}

type Config struct {
	Server  ServerConfig
	Weather WeatherConfig
}
