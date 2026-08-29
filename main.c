#include "pico/printf.h"
#include "pico/stdlib.h"
#include "wifi.h"

#ifndef LED_DELAY_MS
#define LED_DELAY_MS 5000
#endif

int main() {
  // Initialize stdio
  stdio_init_all();
  sleep_ms(2000);

  // Initialize Wi-Fi
  if (!wifi_init_and_connect()) {
    printf("Wi-Fi initialization failed. Entering error loop.\n");
    // Blink LED rapidly on error
    while (true) {
      cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
      sleep_ms(200);
      cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
      sleep_ms(200);
    }
  }

  // Main loop
  while (true) {
    // Toggle LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(LED_DELAY_MS);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    sleep_ms(LED_DELAY_MS);
  }
}
