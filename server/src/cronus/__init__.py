"""Cronus Server Init
"""
__author__ = 'Alexander Shepetko'
__email__ = 'a@shepetko.com'
__license__ = 'MIT'

from fastapi import FastAPI, Request
from datetime import datetime
from pytz import timezone
from .util import get_ip_addr_info, get_weather_info

app = FastAPI()


def _get_client_ip_addr(request: Request) -> str:
    ip_addr = request.client.host
    if 'cf-connecting-ip' in request.headers:
        ip_addr = request.headers['cf-connecting-ip']
    elif 'x-forwarded-for' in request.headers:
        ip_addr = request.headers['x-forwarded-for'].split(',')[0].strip()

    return ip_addr


@app.get('/api/1/time')
async def root(request: Request) -> dict:
    ip_info = get_ip_addr_info(_get_client_ip_addr(request))
    print(ip_info)
    now = datetime.now(timezone(ip_info.get("time_zone", "UTC") or "UTC"))
    return {
        "year": now.year - 2000,
        "month": now.month,
        "day": now.day,
        "dow": now.weekday(),
        "hour": now.hour,
        "min": now.minute,
        "sec": now.second,
    }


@app.get('/api/1/weather')
async def get_weather(request: Request) -> dict:
    ip_info = get_ip_addr_info(_get_client_ip_addr(request))
    return get_weather_info(ip_info.get("latitude", 0), ip_info.get("longitude", 0))
