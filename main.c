#include "pico/printf.h"
#include "pico/stdlib.h"

#include "weather.h"
#include "wifi.h"

#include "DEV_Config.h"
#include "EPD_2in9b_V3.h"
#include "GUI_Paint.h"
#include "font16.c"

// Configuration Constants
#define SLEEP_DURATION_MS 3600000 // 1 hour
#define MAX_LINE_LENGTH 26        // Characters across display
#define LED_ERROR_BLINK_MS 200
#define STARTUP_DELAY_MS 1000
#define TEMP_BUFFER_SIZE 2048

// Global pointers for buffers (malloced)
static UBYTE *BlackImage = NULL;
static UBYTE *RedImage = NULL;

static void display_error_pattern(void) {
  // Blink LED rapidly on error
  while (true) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(LED_ERROR_BLINK_MS);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    sleep_ms(LED_ERROR_BLINK_MS);
  }
}

static bool initialize_display(void) {
  printf("Initializing Display...\n");
  if (DEV_Module_Init() != 0) {
    printf("DEV_Module_Init failed\n");
  }
  EPD_2IN9B_V3_Init();
  printf("EPD Initialized\n");
  return true;
}

static void cleanup(void) {
  if (BlackImage != NULL) {
    free(BlackImage);
    BlackImage = NULL;
  }
  if (RedImage != NULL) {
    free(RedImage);
    RedImage = NULL;
  }
}

int main(void) {
  // Initialize stdio
  stdio_init_all();
  sleep_ms(STARTUP_DELAY_MS);

  // Initialize Wi-Fi
  if (!wifi_init_and_connect()) {
    printf("Wi-Fi initialization failed. Entering error loop.\n");
    display_error_pattern();
    return -1;
  }

  // Initialize the Display
  if (!initialize_display()) {
    printf("Display initialization failed.\n");
    return -1;
  }

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
  Paint_NewImage(BlackImage, EPD_2IN9B_V3_WIDTH, EPD_2IN9B_V3_HEIGHT, 90,
                 WHITE);
  Paint_NewImage(RedImage, EPD_2IN9B_V3_WIDTH, EPD_2IN9B_V3_HEIGHT, 90, WHITE);

  printf("Initialization complete. Starting main loop.\n");

  // Main loop
  while (true) {
    // Toggle LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    printf("Fetching weather data...\n");

    // Fetch Weather (wttr.in - HTTP)
    if (fetch_weather_data()) {
      EPD_2IN9B_V3_Init();
      // Clear Buffers
      Paint_SelectImage(BlackImage);
      Paint_Clear(WHITE);

      Paint_SelectImage(RedImage);
      Paint_Clear(WHITE);

      // Prepare to draw
      int x = 0;
      int y = 0;
      int line_height = Font16.Height; // Usually 16

      // Mutable copy of string
      char temp_buffer[TEMP_BUFFER_SIZE];
      strncpy(temp_buffer, weather_ascii_data, sizeof(temp_buffer) - 1);
      temp_buffer[sizeof(temp_buffer) - 1] = '\0';

      // Draw Line by Line
      char *line = strtok(temp_buffer, "\n");
      while (line != NULL) {
        // Skip empty lines
        if (strlen(line) > 0) {
          Paint_DrawString_EN(x, y, line, &Font16, WHITE, BLACK);
          y += line_height;
        }
        line = strtok(NULL, "\n");
      }

      // Update
      EPD_2IN9B_V3_Display(BlackImage, RedImage);

      EPD_2IN9B_V3_Sleep();
      printf("Weather displayed.\n");
    } else {
      printf("Weather Failed.\n");
    }

    // LED off
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);

    // Sleep
    printf("Sleeping for %d ms\n", SLEEP_DURATION_MS);
    sleep_ms(SLEEP_DURATION_MS);
  }
  // Unreachable
  cleanup();
  return 0;
}
