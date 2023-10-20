/*
This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/
#include <Arduino.h>

#include <stdlib.h>
//#include <pico/rand.h>
#include <include/lwipstack/lwipopts.h>

#include "TM1637.h"

#include <display_driver.h>

/*
Hardware:
   4x 8x8 dot displays, based on MAX7219 chipset
   
Future hardware:
   1x 4-digit LED indicator, driven by MAX7219
   rotary encoder
   accelerometer
*/

void set_random_pixels(int count);

#define CLOCK_CLK    21
#define CLOCK_DIO    20
TM1637 clock_disp(CLOCK_CLK, CLOCK_DIO);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  clock_disp.begin();
  clock_disp.clearScreen();
    
  display_init();

  srand(lwip_get_random());

  set_random_pixels(60);
}

void set_random_pixels(int count) {
  if (count > 255) { count = 255; };
  display_set_all(false);
  while (count > 0) {
    int x = random() % DISP_WIDTH;
    int y = random() % DISP_HEIGHT;
    if (!setpixel(x, y, true)) { count--; };
  }
  
  display_update();  
}

/*
// display demo
void loop() {  
  delay(50);
  digitalWrite(LED_BUILTIN, (step & 15) > 3);
  static int step = 0, x = 0, y = 0;
  step++;

  setpixel(y, x, mode);
  x+=1;
  if (x >= DISP_WIDTH) {
    x=0; y++;
    if (y >= DISP_HEIGHT) {
      y = 0; mode = !mode;
    }
  }
  display_update();
}
*/

int step = 0;
unsigned long next_update = 0;


void loop() {
  // update exactly 4 times per second
  if (millis() < next_update) { return; }
  next_update += 250;
  
  digitalWrite(LED_BUILTIN, (step & 15) > 3);
  step++;

  int time = display_count_on();
  char buff[20];
  sprintf(buff, "%02d%02d", (time / 60), (time % 60));
  clock_disp.setBrightness(7); // 0..7
  clock_disp.switchColon()->refresh();
  clock_disp.display(String(buff));
  
  if (display_count_on() == 0) {
    delay(200);
    for (int i=0; i<4; i++) {
      display_set_all(i & 1);
      display_update();
      delay(100);
    }
    set_random_pixels(120);
  }

  if ((step % 4) == 0) {
    int x, y;
    // pick random dot until we find active one.
    // TODO make more efficient
    while (1) {
      x = random() % DISP_WIDTH;
      y = random() % DISP_HEIGHT;
      if (setpixel(x, y, false)) break;
    }
    display_update();
    // blink it
    for (int i=0; i<4; i++) {
      delay(50);
      setpixel(x, y, (i & 1) == 0);
      display_update();
    }
  }

  
}
