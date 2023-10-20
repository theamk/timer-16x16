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
#include <pio_encoder.h>
#include <display_driver.h>

/*
   
Future hardware:
   rotary encoder
   accelerometer
*/

void set_random_pixels(int count);

#define CLOCK_CLK    21
#define CLOCK_DIO    20
TM1637 clock_disp(CLOCK_CLK, CLOCK_DIO);


#define PIN_LED_GREEN   15
#define PIN_LED_BLUE    14
#define PIN_BTN_GREEN   13
#define PIN_BTN_BLUE    12

#define PIN_ENCODER_A   10
#define PIN_ENCODER_B   11

PioEncoder encoder(PIN_ENCODER_A);  // B must be A + 1

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);

  encoder.begin();
  
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

void set_spiral_pixels(int count) {
  display_set_all(false);  
  int x=0, y=0; 
  int dx=0, dy=-1;
  while (count > 0) {    
    setpixel(-x + 8, y + 7, true);
    if ((x == y) || (x < 0 && x == -y) || (x > 0 && x == 1 - y)) {
      int temp = dx;
      dx = -dy;
      dy = temp;
    }
    x += dx; y += dy;
    count--;
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
unsigned long next_tick = 0;
int last_encoder = -1;

enum {
  M_TICK,
  M_EXPIRED,
  M_SET_TIME,
} mode = M_SET_TIME;

// timer value, seconds
int time_left = 120;

void loop() {

  // our encoder gives us 4 counts per detent
  int this_encoder = encoder.getCount() / 4;
  int delta_encoder = this_encoder - last_encoder;
  last_encoder = this_encoder;

  bool green_down = !digitalRead(PIN_BTN_GREEN);
  bool blue_down = !digitalRead(PIN_BTN_BLUE);

  bool refresh = false;

  if (green_down) {
    if (mode != M_TICK) {
      next_tick = millis();
    }
    mode = M_TICK;
    refresh = true;
  } else if (blue_down) {
    mode = M_SET_TIME;
    refresh = true;
  }
    
  digitalWrite(PIN_LED_GREEN, (mode == M_TICK));
  digitalWrite(PIN_LED_BLUE, (mode == M_SET_TIME));

  // input handling
  switch (mode) {
  case M_SET_TIME:
    if (delta_encoder) {
      time_left += delta_encoder;
      if (time_left < 0) { time_left = 0; }
      refresh = true;
    }
    break;
  case M_TICK:
    if (next_tick < millis()) {
      next_tick += 1000;
      time_left -= 1;
      if (time_left == 0) {
        mode = M_EXPIRED;
      }
      refresh = true;
    }
    break;
  case M_EXPIRED:
    break;
  }
      

  // display handling
  if (refresh) {
    set_spiral_pixels(time_left);
    
    char buff[20] = {};
    if (time_left >= 0) {
      sprintf(buff, "%02d%02d", (time_left / 60), (time_left % 60));
    } else {
      sprintf(buff, "-%d%02d", (-time_left / 60), (-time_left % 60));
    }

    // Re-enable pins if they were disabled before (see below)
    pinMode(CLOCK_DIO, OUTPUT);    
    pinMode(CLOCK_CLK, OUTPUT);
    delay(1); 
    
    clock_disp.setBrightness(7); // 0..7
    if ((time_left & 1) || (mode != M_TICK)) {
      clock_disp.colonOn();
      // Force change invisible character because there is a bug: display won't refresh if
      // only colon changes but string stays the same.
      buff[4] = '.';
    } else {
      clock_disp.colonOff();
      buff[4] = ' ';
    }
    buff[5] = 0;
    clock_disp.display(String(buff));

    // We want to tri-state control pins between runs, to enable soft shut-off
    // the TM1637 library we have uses "mI2C.h" driver, which uses regular digitalWrite
    // calls. So we can just tri-state the pins.
    pinMode(CLOCK_CLK, INPUT_PULLUP);
    pinMode(CLOCK_DIO, INPUT);

  }
  

  /*
  // update exactly 4 times per second
  if (millis() < next_update) { return; }
  next_update += 250;
  
  digitalWrite(LED_BUILTIN, (step & 15) > 3);
  step++;
  int time = display_count_on();  
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
  */
  
}
