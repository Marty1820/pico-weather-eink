#include "pico/cyw43_arch.h"
#include "pico/printf.h"
#include "pico/stdlib.h"

#ifndef LED_DELAY_MS
#define LED_DELAY_MS 5000
#endif

// Perform initialisation
int pico_led_init(void) { return cyw43_arch_init(); }

// Turn the led on or off
void pico_set_led(bool led_on) {
  // Ask the wifi "driver" to set the GPIO on or off
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
}

int main() {
  // Needed for console connection.
  sleep_ms(2000);
  stdio_init_all();

  int rc = pico_led_init();
  hard_assert(rc == PICO_OK);

  while (true) {
    printf("Hello from the PicoW!\n");
    pico_set_led(true);
    sleep_ms(LED_DELAY_MS);
    pico_set_led(false);
    sleep_ms(LED_DELAY_MS);
  }
}
