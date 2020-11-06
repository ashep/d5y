"""Cronus helper functions
"""
__author__ = 'Alexander Shepetko'
__email__ = 'a@shepetko.com'
__license__ = 'MIT'

from requests import request


def get_ip_addr_info(ip_addr: str) -> dict:
    r = request("GET", f"https://freegeoip.app/json/{ip_addr}")

    if r.ok:
        return r.json()

    return {}


def get_weather_info(lat: float, lng: float) -> dict:
    r = request("GET", f"https://www.metaweather.com/api/location/search?lattlong={lat},{lng}")
    if not (r.ok and r.json()):
        return {}

    r = request("GET", f"https://www.metaweather.com/api/location/{r.json()[0]['woeid']}")
    if not (r.ok and r.json()):
        return {}

    d = r.json()
    w_info = d.get("consolidated_weather")[0]  # type: dict
    if not w_info:
        return {}

    w_info.update({
        "woeid": d.get("woeid", ""),
        "city": d.get("title", ""),
        "latt_long": d.get("latt_long", ""),
        "sun_rise": d.get("sun_rise", ""),
        "sun_set": d.get("sun_set", ""),
    })

    return w_info
