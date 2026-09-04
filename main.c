#include "pico/cyw43_arch.h"
#include "pico/printf.h"
#include "pico/stdlib.h"

#include "display.h"
#include "weather.h"
#include "wifi.h"

// Configuration Constants
#define SLEEP_DURATION_MS 3600000 // 1 hour
#define LED_ERROR_BLINK_MS 200
#define STARTUP_DELAY_MS 1000
#define HEARTBEAT_INTERVAL_MS 9900
#define HEARTBEAT_PULSE_MS 100

// Fatal error: blink LED forever
static void fatal_blink(void) {
  while (true) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(LED_ERROR_BLINK_MS);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    sleep_ms(LED_ERROR_BLINK_MS);
  }
}

// Pulse LED during sleep
static void sleep_with_heartbeat(uint32_t duration_ms) {
  uint32_t remaining = duration_ms;
  while (remaining > 0) {
    uint32_t slice =
        (remaining < HEARTBEAT_INTERVAL_MS) ? remaining : HEARTBEAT_INTERVAL_MS;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(HEARTBEAT_PULSE_MS);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);

    if (slice > HEARTBEAT_PULSE_MS) {
      sleep_ms(slice - HEARTBEAT_PULSE_MS);
    }
    remaining -= slice;
  }
}

int main(void) {
  // Initialize stdio
  stdio_init_all();
  sleep_ms(STARTUP_DELAY_MS);

  // Initialize Wi-Fi
  if (!wifi_init_and_connect()) {
    printf("Wi-Fi initialization failed\n");
    fatal_blink();
  }
  printf("Wi-Fi connection. IP: %s\n", wifi_get_ip_address());

  // Initialize the Display
  if (!display_init()) {
    printf("Display initialization failed.\n");
    fatal_blink();
  }
  printf("Initialization complete. Starting main loop.\n");

  // Main loop
  while (true) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); // activity LED

    // Fetch Weather (wttr.in - HTTP)
    if (fetch_weather_data()) {
      if (display_render(weather_get_data())) {
        display_sleep();
        printf("Weather displayed.\n");
      } else {
        printf("Render failed.\n");
      }
    } else {
      printf("Weather fetch failed.\n");
    }

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);

    // Sleep
    printf("Sleeping for %d ms\n", SLEEP_DURATION_MS);
    sleep_with_heartbeat(SLEEP_DURATION_MS);
  }
}
