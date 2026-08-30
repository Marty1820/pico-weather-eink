#include "EPD_2in9b_V3.h"
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

  printf("Initializing Display...\n");
  if (DEV_Module_Init() != 0) {
    printf("Display init failed!\n");
    while (true) {
    } // Halt
  }

  EPD_2IN9B_V3_Init();
  EPD_2IN9B_V3_Clear(); // Clear screen to white/black

  printf("Display ready.\n");

  // Main loop
  while (true) {
    // Toggle LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    // Fetch Weather (wttr.in - HTTP)
    if (fetch_weather_data()) {
      printf(weather_ascii_data);
    } else {
      printf("Weather Failed.\n");
    }

    // LED off
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    // Sleep 1hr
    printf("Sleeping for 1 hr\n");
    sleep_ms(3600000);
  }
}
