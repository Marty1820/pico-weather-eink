#ifndef WIFI_H
#define WIFI_H

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

// Initialize the Wi-Fi driver and connect to the networkfrom secrets.h
// Returns true on success, false on failure.
bool wifi_init_and_connect(void);

// Helper to get the current IP address string
const char *wifi_get_ip_address(void);

#endif
