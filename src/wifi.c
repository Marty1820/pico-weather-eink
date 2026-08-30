#include "pico/printf.h"

#include "config/secrets.h"
#include "wifi.h"

static bool g_wifi_connected = false;
static const char *s_ip_str = NULL;

bool wifi_init_and_connect(void) {
  printf("Starting PicoW...\n");

  // Initialize Wi-Fi driver
  if (cyw43_arch_init()) {
    printf("Failed to initialize CYW43 driver\n");
    return false;
  }

  // Enable Station Mode
  cyw43_arch_enable_sta_mode();

  // Connect to Wi-Fi
  printf("Connecting to Wi-Fi '%s' ...\n", WIFI_SSID);

  // Block until connected or timeout (30 seconds)
  int rc = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS,
                                              CYW43_AUTH_WPA2_AES_PSK, 30000);

  if (rc != 0) {
    printf("Connection failed! Error code: %d\n", rc);
    return false;
  }

  printf("Connected successfully!\n");

  // Get IP Address
  // Note: netif_list is a global from LWIP.
  const ip_addr_t *ip_addr = netif_ip4_addr(netif_list);
  if (ip_addr) {
    s_ip_str = ipaddr_ntoa(ip_addr);
    printf("IP Address: %s\n", s_ip_str);
  } else {
    printf("Could not determine IP address.\n");
    s_ip_str = "Unknown";
  }

  g_wifi_connected = true;
  return true;
}

const char *wifi_get_ip_address(void) {
  return s_ip_str ? s_ip_str : "Not Connected";
}
