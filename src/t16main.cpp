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

#include <RP2040_PWM.h>
#include "TM1637.h"
#include <pio_encoder.h>
#include <display_driver.h>

/*
   
Future hardware:
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

#define PIN_POWER_OFF   22

#define PIN_SPEAKER     6

// how long to spend in settings mode with no input before switching off
#define IDLE_POWER_OFF_INTERVAL  (10 * 60 * 1000)

PioEncoder encoder(PIN_ENCODER_A);  // B must be A + 1
RP2040_PWM * speaker;

unsigned long power_off_time = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);

  encoder.begin();
  
  clock_disp.begin();  
  clock_disp.clearScreen();
    
  display_init();

  srand(lwip_get_random());

  speaker = new RP2040_PWM(PIN_SPEAKER, 1000, 0);
 
  //set_random_pixels(60);

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


int last_speaker_freq = -1;
void set_speaker_freq(int freq) {
  if (last_speaker_freq == freq) {
    return;
  }
  if (freq == 0) {
    speaker->setPWM(PIN_SPEAKER, 1000, 0);
  } else {
    // very low duty cycle.. "10" (10%) is enough to be pretty loud.
    speaker->setPWM(PIN_SPEAKER, freq, 5);
  }
  last_speaker_freq = freq;
}


enum {
  T_TICK_UP,
  T_TICK_DOWN,
  T_EXPIRED,
  T_SET_TIME,
} timer_mode = T_SET_TIME;

enum {
  M_TIME,  // default, time is shown
  M_SET_STEP,
} menu_mode = M_TIME;
  

// timer value, seconds
int time_left = 0;
// millis() timestamp when this changes
unsigned long next_tick = 0;
// how many seconds does each pixel represent
int pixel_time = 10;


unsigned long tone_state = 0;
void maybe_update_speaker() { 
  if (!tone_state) {
    set_speaker_freq(0);
  } else { // play the tone
    unsigned int elapsed = millis() - tone_state;
    static int melody[] = {
      523, 523, 587, 523, 698, 659, 
    };
    
    if ((elapsed % 300) < 50) { set_speaker_freq(0); }
    else {
      unsigned int num = elapsed / 300;
      if (num < (sizeof(melody)/sizeof(melody[0]))) {
        set_speaker_freq(melody[num]);
      } else {
        tone_state = 0;
        set_speaker_freq(0);
      };
    }
  }
}


void handle_buttons() {
  // debounce green
  static unsigned long green_down_since = 0;
  bool green_down = false;
  if (digitalRead(PIN_BTN_GREEN)) {
    green_down_since = 0;
  } else if (!green_down_since) {
    green_down_since = millis();
  } else if (millis() < (green_down_since + 50)) {
    green_down = true;
  }

  static unsigned long blue_down_since = 0;
  bool blue_down = false;
  if (digitalRead(PIN_BTN_BLUE)) {
    blue_down_since = 0;
  } else if (!blue_down_since) {
    blue_down_since = millis();
  } else if (millis() < (blue_down_since + 50)) {
    blue_down = true;
  }

  static bool green_was_down = false;
  if (green_down && !green_was_down) {
    // green button controls timer mode
    switch(timer_mode) {
    case T_EXPIRED:
    case T_TICK_UP:
    case T_TICK_DOWN: // was ticking -> pause, set time
      timer_mode = T_SET_TIME;
      menu_mode = M_TIME;
      break;
    case T_SET_TIME:
      if (time_left == 0) {
        timer_mode = T_TICK_UP;
      } else {
        timer_mode = T_TICK_DOWN;
      }
      menu_mode = M_TIME;
      next_tick = millis();
      break;
    }
  };
  green_was_down = green_down;
  
  static bool blue_was_down = false;
  if (blue_down && !blue_was_down) {
    // blue button controls menu mode
    switch (menu_mode) {
    case M_TIME: menu_mode = M_SET_STEP; break;
    default:
      menu_mode = M_TIME;
    }
  };
  blue_was_down = blue_down;

  if (green_down || blue_down) {
    power_off_time = 0; // any input resets power-off timer
    tone_state = 0; // any input turns tone off
  }
}


void handle_encoder() {
  // our encoder gives us 4 counts per detent
  int this_encoder = encoder.getCount() / 4;
  static int last_encoder = 0;
  int delta_encoder = this_encoder - last_encoder;
  last_encoder = this_encoder;

  if (delta_encoder == 0) {
    return;
  }
  power_off_time = 0;  
  tone_state = 0; // any input turns tone off
    
  switch (menu_mode) {
  case M_SET_STEP:
    // TODO
    break;
  default:
    if (timer_mode == T_SET_TIME) {
      // when pixels are small, always do one step == one pixel. But when they are large, provide finer steps.
      int scroll_step = (pixel_time < 60) ? pixel_time :
        (time_left >= 600) ? 60 : 15;
      while (delta_encoder < 0) {
        delta_encoder++;
        time_left = ((time_left - 1) / scroll_step) * scroll_step;
        if (time_left < 0) { time_left = 0; }
      }
      while (delta_encoder > 0) {
        delta_encoder--;
        time_left = ((time_left / scroll_step) + 1) * scroll_step;
      }
    }
    break;
  }
}


void update_leds() {   
  unsigned long partial = (millis() + 2000 - next_tick) % 1000;
  switch (timer_mode) {
  case T_TICK_UP:
  case T_TICK_DOWN: 
    digitalWrite(PIN_LED_GREEN, partial < 100); // blink  once per second
    break;
  case T_EXPIRED:
    digitalWrite(PIN_LED_GREEN, (partial % 250) < 125); // blink quickly nonstop
    break;
  case T_SET_TIME:
    digitalWrite(PIN_LED_GREEN, true);
  }

  digitalWrite(PIN_LED_BLUE, (menu_mode != M_TIME));
}


void update_maybe_power_off() {
  if (timer_mode == T_TICK_UP || timer_mode == T_TICK_DOWN) {
    // if timer is ticking or inputs, don't power off
    power_off_time = 0;
  } else if (!power_off_time) {
    // activity happened, reset timer
    power_off_time = millis() + IDLE_POWER_OFF_INTERVAL;
  } else if (power_off_time < millis()) {
    // power off time
    digitalWrite(PIN_POWER_OFF, 1);    
    // should not reach here, but in case it didn't work for some reason, reset.
    delay(100);
    digitalWrite(PIN_POWER_OFF, 0);
    power_off_time = 0;
  }
}  


void maybe_refresh_256px_display() {
  int pixels = (time_left + pixel_time - 1) / pixel_time;

  if (timer_mode == T_TICK_DOWN && ((time_left % pixel_time) == 1)) {
    // last pixel blinks before disappearing
    unsigned long partial = 1000 - (millis() + 2000 - next_tick) % 1000;    
    if ((partial > 50 && partial < 100) ||
        (partial > 150 && partial < 200)) {
      pixels--;
    }
  };
  
  static int last_pixels = -1;
  if (pixels != last_pixels) {
    set_spiral_pixels(pixels);
    last_pixels = pixels;
  };
}

void maybe_refresh_7seg_display() {    
  char buff[20] = {};
  if (time_left < 3600) {
    // less than 1 hour -> show minutes + seconds, 2 digits
    sprintf(buff, "%02d%02d", (time_left / 60), (time_left % 60));
  } else {
    // more than 1 hour -> show hours + minutes, 1 digit
    sprintf(buff, "%2d%02d", (time_left / 3600), ((time_left / 60)% 60));
  }
  if (timer_mode == T_SET_TIME && power_off_time) {
    // blink on idle input
    unsigned long idle = (millis()  + IDLE_POWER_OFF_INTERVAL - power_off_time );
    if ((idle > 500) & ((idle % 500) > 250)) {
      sprintf(buff, "      ");
    }
  }
  bool colon = (timer_mode != T_TICK_UP) || (timer_mode != T_TICK_DOWN) || (time_left & 1);

  // Force change invisible character because there is a bug: display won't refresh if
  // only colon changes but string stays the same.
  buff[4] = colon ? ':' : ' ';
  buff[5] = 0;

  static String last_display = "ZZZZZZ";
  if (last_display == buff) {
    return;
  }
  last_display = String(buff);
  
  // Re-enable pins if they were disabled before (see below)
  pinMode(CLOCK_DIO, OUTPUT);    
  pinMode(CLOCK_CLK, OUTPUT);
  delay(1); 
    
  clock_disp.setBrightness(7); // 0..7
  if (colon) {
    clock_disp.colonOn();
  } else {
      clock_disp.colonOff();
  }
  clock_disp.display(last_display);

  // We want to tri-state control pins between runs, to enable soft shut-off
  // the TM1637 library we have uses "mI2C.h" driver, which uses regular digitalWrite
  // calls. So we can just tri-state the pins.
  pinMode(CLOCK_CLK, INPUT_PULLUP);
  pinMode(CLOCK_DIO, INPUT);
}


void loop() {
  handle_buttons();
  handle_encoder();

  update_leds();
  maybe_update_speaker();
  
  // tick handling
  switch (timer_mode) {
  case T_SET_TIME:
  case T_EXPIRED:
    break;
  case T_TICK_UP:
  case T_TICK_DOWN:
    if (next_tick < millis()) {
      next_tick += 1000;
      if (timer_mode == T_TICK_UP) {
        time_left++;
      } else {
        time_left--;
        if (time_left == 0) {
          timer_mode = T_EXPIRED;
          tone_state = millis(); // start music playback
        }
      }
    }
    break;
  }

  update_maybe_power_off();
  
  maybe_refresh_7seg_display();
  maybe_refresh_256px_display();
}    
