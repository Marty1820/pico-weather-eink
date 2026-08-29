#include "config/secrets.h"
#include "pico/cyw43_arch.h"
#include "pico/printf.h"
#include "pico/stdlib.h"

#ifndef LED_DELAY_MS
#define LED_DELAY_MS 5000
#endif

int main() {
  // Initialize stdio
  stdio_init_all();
  sleep_ms(2000);

  printf("Starting PicoW...\n");

  // Initialize Wi-Fi driver
  if (cyw43_arch_init()) {
    printf("Failed to initialize CYW43 driver\n");
    return 1;
  }

  // Enable Station Mode (Client)
  cyw43_arch_enable_sta_mode();

  // Connect to Wi-Fi
  printf("Connecting to Wi-Fi '%s' ... \n", WIFI_SSID);
  // block until connected or timeout (30 seconds)
  int rc = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS,
                                              CYW43_AUTH_WPA2_AES_PSK, 30000);

  if (rc != 0) {
    printf("Connection failed! Error code: %d\n", rc);
    // Blink LED rapidly to indicate error
    while (true) {
      cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
      sleep_ms(200);
      cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
      sleep_ms(200);
    }
  }

  printf("Connected successfully!\n");
  // Get IP Address
  const char *ip = ipaddr_ntoa(netif_ip4_addr(netif_list));
  printf("IP Address: %s\n", ip ? ip : "Unknown");

  // Main loop
  while (true) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(LED_DELAY_MS);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    sleep_ms(LED_DELAY_MS);
  }
}
