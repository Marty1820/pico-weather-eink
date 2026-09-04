#include "pico/printf.h"

#include "DEV_Config.h"
#include "EPD_2in9b_V3.h"
#include "GUI_Paint.h"

#include "display.h"
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 26 // Characters across display
#define MAX_LINES 15       // Rows on display at Font16 height
#define TRIM_LINE_LEN 256
#define TEXT_BUFFER_SIZE 2048

static UBYTE *black_image = NULL;
static UBYTE *red_image = NULL;

bool display_init(void) {
  printf("Initializing display...\n");

  if (DEV_Module_Init() != 0) {
    printf("DEV_Module_Init failed\n");
    return false;
  }

  EPD_2IN9B_V3_Init();

  UWORD imagesize =
      ((EPD_2IN9B_V3_WIDTH % 8 == 0) ? (EPD_2IN9B_V3_WIDTH / 8)
                                     : (EPD_2IN9B_V3_WIDTH / 8 + 1)) *
      EPD_2IN9B_V3_HEIGHT;

  printf("Allocating %u bytes for buffers...\n", (unsigned)imagesize);
  black_image = (UBYTE *)malloc(imagesize);
  red_image = (UBYTE *)malloc(imagesize);
  if (!black_image || !red_image) {
    printf("Memory allocation failed!\n");
    return false;
  }

  // Rotate: 0=0deg, 1=90deg, 2=180deg, 3=270deg
  Paint_NewImage(black_image, EPD_2IN9B_V3_WIDTH, EPD_2IN9B_V3_HEIGHT, 90,
                 WHITE);
  Paint_NewImage(red_image, EPD_2IN9B_V3_WIDTH, EPD_2IN9B_V3_HEIGHT, 90, WHITE);

  return true;
}

// Trim lines to display length
static void trim_to_width(char *data, int max_width) {
  static char temp[TEXT_BUFFER_SIZE];
  static char lines[MAX_LINES][TRIM_LINE_LEN];
  int num_lines = 0;

  strncpy(temp, data, sizeof(temp) - 1);
  temp[sizeof(temp) - 1] = '\0';

  char *line = strtok(temp, "\n");
  while (line && num_lines < MAX_LINES) {
    strncpy(lines[num_lines], line, TRIM_LINE_LEN - 1);
    lines[num_lines][TRIM_LINE_LEN - 1] = '\0';
    num_lines++;
    line = strtok(NULL, "\n");
  }

  data[0] = '\0';
  for (int i = 0; i < num_lines; i++) {
    strncat(data, lines[i], max_width);
    strcat(data, "\n");
  }
}

bool display_render(const char *text) {
  if (!black_image || !red_image) {
    return false;
  }

  Paint_SelectImage(black_image);
  Paint_Clear(WHITE);
  Paint_SelectImage(red_image);
  Paint_Clear(WHITE);
  Paint_SelectImage(black_image);

  // Mutable working copy
  static char buffer[TEXT_BUFFER_SIZE];
  strncpy(buffer, text, sizeof(buffer) - 1);
  buffer[sizeof(buffer) - 1] = '\0';

  trim_to_width(buffer, MAX_LINE_LENGTH);

  const int x = 0;
  int y = 0;
  const int line_height = Font16.Height;

  char *line = strtok(buffer, "\n");
  while (line != NULL) {
    if (strlen(line) > 0) {
      Paint_DrawString_EN(x, y, line, &Font16, WHITE, BLACK);
      y += line_height;
    }
    line = strtok(NULL, "\n");
  }

  EPD_2IN9B_V3_Display(black_image, red_image);
  return true;
}

void display_sleep(void) { EPD_2IN9B_V3_Sleep(); }
