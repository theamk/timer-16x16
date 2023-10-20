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

#include <SPI.h>

/*
Hardware:
   4x 8x8 dot displays, based on MAX7219 chipset
   
Future hardware:
   1x 4-digit LED indicator, driven by MAX7219
   rotary encoder
   accelerometer
*/

SPISettings display_settings(100000, MSBFIRST, SPI_MODE0);
#define SPI1_MISO 16  // not used
#define SPI1_MOSI 19
#define SPI1_SCLK 18
#define SPI1_CS_DISP 17
//arduino::MbedSPI SPI1(SPI1_MISO, SPI1_MOSI, SPI1_SCLK);

void display_send_raw(uint8_t cmd, uint8_t data[4]) {
  digitalWrite(SPI1_CS_DISP, 0);
  SPI.beginTransaction(display_settings);
  delay(1);
  SPI.transfer(cmd); SPI.transfer(data[0]);
  SPI.transfer(cmd); SPI.transfer(data[1]);
  SPI.transfer(cmd); SPI.transfer(data[2]);
  SPI.transfer(cmd); SPI.transfer(data[3]);
  delay(1);
  SPI.endTransaction();
  digitalWrite(SPI1_CS_DISP, 1);
}

void display_send_raw_all(uint16_t cmddata) {
  uint8_t d = cmddata & 0xFF;
  uint8_t data[4] = {d, d, d, d};    
  display_send_raw(cmddata >> 8, data);
}


uint8_t framebuffer[32] = {0};

void display_update() {
  display_send_raw_all(0x0900); // no decode, confirms default
  display_send_raw_all(0x0A05); // intensity in last digit, 0..0xF
  display_send_raw_all(0x0B07); // display full 8x8, confirms default
  display_send_raw_all(0x0F00); // display test off, confrims default
  for (int i=0; i<8; i++) {
    display_send_raw(0x01 + i, &framebuffer[i*4]);
  }
  display_send_raw_all(0x0C01); // take displays out of shutdown
}

void setpixel(int x, int y, bool on) {
  if (x < 0 || x > 15) return;
  if (y < 0 || y > 15) return;
  // The formulas below depend on how the modules were assembled
  int module_idx = ((y >= 8) ? 0 : 1) + ((x >= 8) ? 2 : 0);
  int module_bit = (y & 7);
  int offset = ((x & 7) ^ 7) * 4 + module_idx;
  if (on) {
    framebuffer[offset] |= (1 << module_bit);
  } else {
    framebuffer[offset] &= ~(1 << module_bit);
  }  
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SPI1_CS_DISP, OUTPUT);
  digitalWrite(SPI1_CS_DISP, 1);

  SPI.begin();
  display_update();
}

int step = 0;

int x = 0;
int y = 0;
bool mode = true;

void loop() {  
  delay(50);
  digitalWrite(LED_BUILTIN, (step & 15) > 3);
  step++;
  
  setpixel(y, x, mode);
  x+=1;
  if (x > 15) {
    x=0; y++;
    if (y > 15) {
      y = 0; mode = !mode;
    }
  }
  display_update();
}
