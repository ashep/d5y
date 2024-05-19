# D5Y backend server

## Pre-setup

Get your [https://www.weatherapi.com/](weatherapi.com) API key.

## Run using Docker Compose

Fill `.env` file with values:

```dotenv
WEATHER_APIKEY=yourkey
EXT_PORT=:9000
```

Start the service:

```shell
docker-compose up --build -d
```
