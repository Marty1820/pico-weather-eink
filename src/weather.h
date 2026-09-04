#ifndef WEATHER_H
#define WEATHER_H

#include "pico/stdlib.h"

// Fetch weather data from wttr.in (HTTP)
bool fetch_weather_data(void);

// Pointer to internal weather data.
// Valid only after a successful fetch_weather_data() call
const char *weather_get_data(void);

#endif
