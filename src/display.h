#ifndef DISPLAY_H
#define DISPLAY_H

#include "pico/stdlib.h"

// Init hardware, allocate framebuffers, prepare the Paint library.
bool display_init(void);

// Clear buffers, render the given text
bool display_render(const char *text);

// Put panel to sleep
void display_sleep(void);

#endif
