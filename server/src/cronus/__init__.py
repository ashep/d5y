"""Cronus Server Init
"""
__author__ = 'Alexander Shepetko'
__email__ = 'a@shepetko.com'
__license__ = 'MIT'

from fastapi import FastAPI, Request
from datetime import datetime
from pytz import timezone
from .util import get_ip_address_info, get_weather_info

app = FastAPI()


@app.get('/api/1/time')
async def root(request: Request) -> dict:
    ip_info = get_ip_address_info(request.headers.get("x-forwarded-for", request.client.host))
    now = datetime.now(timezone(ip_info.get("time_zone", "UTC") or "UTC"))
    return {
        "year": now.year,
        "month": now.month,
        "day": now.day,
        "wday": now.weekday(),
        "hour": now.hour,
        "min": now.minute,
        "sec": now.second,
    }


@app.get('/api/1/weather')
async def get_weather(request: Request) -> dict:
    ip_info = get_ip_address_info(request.headers.get("x-forwarded-for", request.client.host))
    return get_weather_info(ip_info.get("latitude", 0), ip_info.get("longitude", 0))
