#ifndef NETWORK_H
#define NETWORK_H

#include "pico/stdlib.h"

// Generic HTTP GET function
// Returns true if successful; response data stored in `response_buffer`
bool http_get(const char *hostname, const char *path, char *response_buffer,
              size_t response_size);

#endif
