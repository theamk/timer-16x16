#ifndef _DISPLAY_DRIVER_H_
#define _DISPLAY_DRIVER_H_

#include <stdint.h>

#define DISP_WIDTH    16
#define DISP_HEIGHT   16

void display_init();
void display_update();

// Return number of pixels on
int display_count_on();

//////////////////////////////////////////////////////////
// All functions below need explicit "display_update" call
//////////////////////////////////////////////////////////

void display_set_all(bool on);

// Set one pixel. Invalid coordinagtes silently ignored.
// Return old pixel value
bool display_setpixel(int x, int y, bool on);

// Replace entire display with icon. Each uint16_t is one row, LSB on the left. 
void display_show_icon(uint16_t icon[DISP_HEIGHT]);

#endif
