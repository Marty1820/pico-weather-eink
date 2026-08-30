#include "pico/printf.h"
#include "pico/stdlib.h"

#include "weather.h"
#include "wifi.h"

#include "DEV_Config.h"
#include "EPD_2in9b_V3.h"
#include "GUI_Paint.h"
#include "font16.c"

// Global pointers for buffers (malloced)
static UBYTE *BlackImage = NULL;
static UBYTE *RedImage = NULL;

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

  // Initialize the Display
  printf("Initializing Display...\n");
  if (DEV_Module_Init() != 0) {
    printf("DEV_Module_Init failed\n");
    return -1;
  }
  EPD_2IN9B_V3_Init();
  printf("EPD Initialiezed\n");

  // Allocate Buffers
  UWORD Imagesize =
      ((EPD_2IN9B_V3_WIDTH % 8 == 0) ? (EPD_2IN9B_V3_WIDTH / 8)
                                     : (EPD_2IN9B_V3_WIDTH / 8 + 1)) *
      EPD_2IN9B_V3_HEIGHT;

  printf("Allocating %d bytes for buffers...\n", Imagesize);
  BlackImage = (UBYTE *)malloc(Imagesize);
  RedImage = (UBYTE *)malloc(Imagesize);

  if (!BlackImage || !RedImage) {
    printf("Memory allocation failed!\n");
    return -1;
  }

  // Create Paint Objects
  // Parameters: Image, Width, Height, Rotate, Color
  // Rotate: 0=0deg, 1=90deg, 2=180deg, 3=270deg
  // Color: WHITE (0xFF) or BLACK (0x00)
  Paint_NewImage(BlackImage, EPD_2IN9B_V3_WIDTH, EPD_2IN9B_V3_HEIGHT, 0, WHITE);
  Paint_NewImage(RedImage, EPD_2IN9B_V3_WIDTH, EPD_2IN9B_V3_HEIGHT, 0, WHITE);

  // Clear Buffers
  Paint_SelectImage(BlackImage);
  Paint_Clear(WHITE);

  Paint_SelectImage(RedImage);
  Paint_Clear(WHITE);

  // Main loop
  while (true) {
    // Toggle LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    printf("Draw String\n");

    // Fetch Weather (wttr.in - HTTP)
    // if (fetch_weather_data()) {
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);

    Paint_SelectImage(BlackImage);
    // Left, top, "11 chars max", &Font16, background, text color);
    Paint_DrawString_EN(4, 0, "12345678901", &Font16, WHITE, RED);
    // Paint_DrawString_EN(10, 10, weather_ascii_data, &Font16, WHITE, BLACK);

    // Update
    EPD_2IN9B_V3_Display(BlackImage, RedImage);

    EPD_2IN9B_V3_Sleep();
    printf("Weather displayed.\n");
    // } else {
    //   printf("Weather Failed.\n");
    // }

    // LED off
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    // Sleep 1hr
    printf("Sleeping for 1 hr\n");
    // sleep_ms(3600000);
    sleep_ms(30000);
  }
}
