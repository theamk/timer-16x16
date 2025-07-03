#ifndef _DISPLAY_DRIVER_H_
#define _DISPLAY_DRIVER_H_

#define DISP_WIDTH    16
#define DISP_HEIGHT   16

void display_init();
void display_update();

void display_set_all(bool on);

int display_count_on();

// return old pixel value
bool display_setpixel(int x, int y, bool on);


#endif
