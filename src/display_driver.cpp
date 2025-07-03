#include <display_driver.h>
#include <Arduino.h>

#include <SPI.h>

SPISettings display_settings(100000, MSBFIRST, SPI_MODE0);
#define SPI1_MISO 16  // not used
#define SPI1_MOSI 19
#define SPI1_SCLK 18
#define SPI1_CS_DISP 17
//arduino::MbedSPI SPI1(SPI1_MISO, SPI1_MOSI, SPI1_SCLK);

static void display_send_raw(uint8_t cmd, uint8_t data[4]) {
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

static void display_send_raw_all(uint16_t cmddata) {
  uint8_t d = cmddata & 0xFF;
  uint8_t data[4] = {d, d, d, d};
  display_send_raw(cmddata >> 8, data);
}

uint8_t framebuffer[32] = {0};

void display_update() {
  // To enable soft power-off, we tristate pins when unused. So enable them back.
  pinMode(SPI1_CS_DISP, OUTPUT);
  digitalWrite(SPI1_CS_DISP, 1);
  SPI.begin();
  delay(1);

  // docs:
  // https://www.analog.com/media/en/technical-documentation/data-sheets/MAX7219-MAX7221.pdf

  // lines were tri-stated, so send some no-ops to avoid glitches
  display_send_raw_all(0);

  // Start by setting framebuffer to the right value
  for (int i=0; i<8; i++) {
    display_send_raw(0x01 + i, &framebuffer[i*4]);
  }
  display_send_raw_all(0x0900); // no decode, confirms default
  display_send_raw_all(0x0A01); // intensity in last digit, 0..0xF
  display_send_raw_all(0x0B07); // display full 8x8, confirms default
  display_send_raw_all(0x0F00); // display test off, confirms default
  display_send_raw_all(0x0C01); // take displays out of shutdown

  // Another no-op for the road, in case tri-stated lines get loaded somehow.
  display_send_raw_all(0);

  // Now that we are done, turn things off again
  SPI.end();
  pinMode(SPI1_CS_DISP, INPUT_PULLUP);
  pinMode(SPI1_MOSI, INPUT);
  pinMode(SPI1_SCLK, INPUT);
}

bool display_setpixel(int x, int y, bool on) {
  if (x < 0 || x > 15) return false;
  if (y < 0 || y > 15) return false;
  // The formulas below depend on how the modules were assembled
  int module_idx = ((x >= 8) ? 1 : 0) + ((y >= 8) ? 0 : 2);
  int module_bit = (x & 7) ^ 7;
  int offset = (y & 7)  * 4 + module_idx;
  bool old = (framebuffer[offset] & (1 << module_bit)) != 0;
  if (on) {
    framebuffer[offset] |= (1 << module_bit);
  } else {
    framebuffer[offset] &= ~(1 << module_bit);
  }
  return old;
}

void display_show_icon(uint16_t icon[DISP_HEIGHT]) {
  display_set_all(false);
  for (int y=0; y<DISP_HEIGHT; y++) {
    uint16_t val = icon[y];
    for (int x=0; x<DISP_WIDTH; x++) {
      if ((val >> x) & 1) {
	display_setpixel(x, y, true);
      }
    }
  }
}


void display_set_all(bool on) {
  memset(framebuffer, on ? 0xff : 0, sizeof(framebuffer));
}

void display_init() {
  // We init SPI bus at the start of display_update function.
  display_update();
}


int display_count_on() {
  int cnt = 0;
  for (size_t i=0; i<sizeof(framebuffer); i++)  {
    cnt += __builtin_popcount(framebuffer[i]);
  }
  return cnt;
}
