#include "pico/printf.h"
#include "pico/stdlib.h"
#include "weather.h"
#include "wifi.h"

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

  printf("Wi-Fi Ready. Starting Data Collection.\n");

  // Main loop
  while (true) {
    // Toggle LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    // Fetch Weather (wttr.in - HTTP)
    if (fetch_weather_data()) {
      printf("Weather succeeded\n");
    } else {
      printf("Weather Failed.\n");
    }

    // LED off
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    // Sleep 1hr
    sleep_ms(3600000);
  }
}
