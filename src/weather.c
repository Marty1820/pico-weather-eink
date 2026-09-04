#include "weather.h"
#include "config/secrets.h"
#include "network.h"

#include "pico/printf.h"

#include <string.h>

#define WEATHER_DATA_SIZE 2048

bool weather_fetched = false;
char weather_ascii_data[WEATHER_DATA_SIZE];

// Full HTTP response (headers + body)
static char response_buffer[4096];

// Strip UTF-8 continuation sequences and any byte >= 0x80
// Which can't be rendered by Font16
static void clean_special_chars(char *str) {
  char *src = str;
  char *dst = str;

  while (*src) {
    // Skip degree symbol (UTF-8: 0xC2 0xB0) and arrows
    if ((unsigned char)*src >= 128) {
      src++; // skip non-ASCII
      continue;
    }
    *dst++ = *src++;
  }
  *dst = '\0';
}

bool fetch_weather_data(void) {
  // Build the weather URL path
  char path[256];
  snprintf(path, sizeof(path), "/%.4f,%.4f?%s", (double)LATITUDE,
           (double)LONGITUDE, OPTS);

  printf("Fetching weather for: %s\n", path);

  // Perform HTTP GET
  if (!http_get("wttr.in", path, response_buffer, sizeof(response_buffer))) {
    printf("HTTP GET failed\n");
    weather_fetched = false;
    return false;
  }

  // Strip HTTP headers
  char *body = response_buffer;
  char *header_end = strstr(response_buffer, "\r\n\r\n");
  if (header_end) {
    body = header_end + 4;
  }

  size_t clean_len = strlen(body);
  // Copy to weather_ascii_data
  if (clean_len >= sizeof(weather_ascii_data)) {
    clean_len = WEATHER_DATA_SIZE - 1;
  }
  memcpy(weather_ascii_data, body, clean_len);
  weather_ascii_data[clean_len] = '\0';

  // Clean special characters from output
  clean_special_chars(weather_ascii_data);

  printf("\n--- WEATHER DATA ---\n%s\n--- END ---\n", weather_ascii_data);
  weather_fetched = true;
  return true;
}

const char *weather_get_data(void) { return weather_ascii_data; }
