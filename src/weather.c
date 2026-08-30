#include "weather.h"
#include "config/secrets.h"
#include "network.h"

#include <stdio.h>
#include <string.h>

bool weather_fetched = false;
char weather_ascii_data[2048];

static char response_buffer[4096]; // Full HTTP response

bool fetch_weather_data(void) {
  // Build the weather URL path
  char path[256];
  snprintf(path, sizeof(path), "/%.4f,%.4f?%s", (double)LATITUDE,
           (double)LONGITUDE, OPTS);

  printf("Fetching weather for: %s\n", path);

  // Perform HTTP GET
  if (!http_get("wttr.in", path, response_buffer, sizeof(response_buffer))) {
    printf("HTTP GET failed\n");
    return false;
  }

  // Strip HTTP headers
  char *header_end = strstr(response_buffer, "\r\n\r\n");
  if (!header_end) {
    printf("No valid HTTP headers found\n");
    return false;
  }

  char *clean_data = header_end + 4;
  size_t clean_len = strlen(clean_data);

  // Copy to weather_ascii_data
  if (clean_len >= sizeof(weather_ascii_data)) {
    clean_len = sizeof(weather_ascii_data) - 1;
  }
  strncpy(weather_ascii_data, clean_data, clean_len);
  weather_ascii_data[clean_len] = '\0';

  printf("\n--- WEATHER DATA ---\n%s\n--- END ---\n", weather_ascii_data);
  weather_fetched = true;
  return true;
}
