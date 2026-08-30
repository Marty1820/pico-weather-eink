#ifndef WEATHER_H
#define WEATHER_H

#include "pico/stdlib.h"

// Fetch weather data from wttr.in (HTTP)
bool fetch_weather_data(void);

// Global results
extern char weather_ascii_data[2048];
extern bool weather_fetched;

#endif
