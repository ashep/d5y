# Cronus backend server

## Pre-setup

1. Get your [https://openweathermap.org](openweathermap.org) API key.

## Run using Docker Compose

Fill `.env` file with values:

Set service port number:

```dotenv
EXT_PORT=9000
```

Set openweathermap.org API key:

```dotenv
WEATHER_API_KEY=yourkey
```

```shell
docker-compose up --build -d
```
