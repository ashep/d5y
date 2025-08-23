static float lat = 0;
static float lng = 0;

void dy_cloud_set_location(float new_lat, float new_lng) {
    lat = new_lat;
    lng = new_lng;
}

float dy_cloud_get_location_lat() {
    return lat;
}

float dy_cloud_get_location_lng() {
    return lng;
}
