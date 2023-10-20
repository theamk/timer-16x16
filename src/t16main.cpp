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
Hardware:
   4x 8x8 dot displays, based on MAX7219 chipset
   1x 4-digit LED indicator, driven by TM1637

Pinout:
  (left side)
  GP0
  GP1

  GP2
  GP3
  GP4
  GP5

  GP6
  GP7
  GP8  IO - IMU SDA  (w/4.7K pull-up)
  GP9  IO - IMU SCL  (w/4.7K pull-up)

  GP10  I - encoder A (w/4.7K pull-up)
  GP11  I - encoder B (w/4.7K pull-up)
  GP12  I - button green (w/4.7K pull-up)
  GP13  I - button blue (w/4.7K pull-up)

  GP14  O - LED green (270 ohm to GND, active high)
  GP15  O - LED blue (270 ohm to GND, active high)

  (right side)
  GP16  I - SPI RX (unused)
  GP17  O - SPI CSn to MAX7219 8x8 displays

  GP18  O - SPI SCK to MAX7219 8x8 displays
  GP19  O - SPI TX to MAX7219 8x8 displays
  GP20  O - DATA to TM1637
  GP21  O - CLK to TM1637

  GP22  O - (planned) soft power off
  GP23  I - on-board voltage regulator mode selector
  GP24  I - on-board USB voltage sensor
  GP25  O - on-board LED

  GP26
  GP27
  GP28

Power connections:
  USB is exposed outside the box
  VBUS goes to charger in
         (so USB input can be used to charge battery)
  VSYS is connected to switch output via diode
  3V3_EN is connnected to switch output via resistor
         (low=shutdown, so when device is off but USB is supplying power. pico is off)

 PICO:VBUS               GND
   |                      |
[ VIN+                   VIN- ]
[                             ]
[ charger / protection board  ]
[ (upside-down to expose LED) ]
[ (modded to 500mA Icharge)   ]
[                             ]
[  OUT+ BATT+    BATT-  OUT-  ]
    |    |         |     |
    |    \- 14300 -/    GND
    |
    |              /-PICO:GP22
  /---\   /-GND-\  |  x   x
  |   |   |     |  |  |   |
[ IN+ IN+ GND GND ON OFF CTRL ]
[                             ]
[  POLOLU mini pushshbutton   ]
[   power switch (LV 2808)    ]
[                             ]
[ OUT OUT GND GND  SW-A  SW-B ]
   |   |  |     |    |     |
   \-|-/  \-GND-/    \-BTN-/    <- power button (outside)
     |
     +--[R:10K]--PICO:3V3_EN   (switch works even for USB input)
     +--[R:10K]--GND        
     +--Vcc of LED displays
     |
     \--|>|--PICO:VSYS

   
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
    setpixel(x + 7, y + 7, true);
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

  if (green_down) {
    if (mode != M_TICK) {
      next_tick = millis();
    }
    mode = M_TICK;
  } else if (blue_down) {
    mode = M_SET_TIME;
  }
    
  digitalWrite(PIN_LED_GREEN, (mode == M_TICK));
  digitalWrite(PIN_LED_BLUE, (mode == M_SET_TIME));

  bool refresh = false;

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
    
    char buff[20];
    sprintf(buff, "%02d%02d", (time_left / 60), (time_left % 60));
    clock_disp.setBrightness(7); // 0..7
    clock_disp.switchColon()->refresh();
    clock_disp.display(String(buff));
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
