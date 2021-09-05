# Cronus backend server

## Setup

Fill `.env` file with values:


Set service port number:

```dotenv
EXT_PORT=9000
```

Set openweathermap.org API key:

```dotenv
WEATHER_API_KEY=yourkeystring
```

## Run

```shell
docker-compose up --build -d
```
