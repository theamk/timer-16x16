Timer with 16x16 dot display
----------------------------


Display and input section
------------------

Parts:
- Raspberry PI PICO
- 4x 8x8 dot displays, based on MAX7219 chipset
- 1x 4-digit LED indicator, driven by TM1637

Point-to-point wiring, pinout:
```
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
  GP8  IO - PLANNED: IMU SDA  (w/2.2K pull-up)
  GP9  IO - IMU SCL  (w/2.2K pull-up)

  GP10  I - encoder A (w/2.2K pull-up)
  GP11  I - encoder B (w/2.2K pull-up)
  GP12  I - button green (w/2.2K pull-up)
  GP13  I - button blue (w/2.2K pull-up)

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
```


Power control
-------------

For simpler design, feel free to skip. Connect Vin
pins of display to pico's VSYS, and you'd have a USB-powered device.

Or get Pimoroni's lipo SHIM, I have not tried it but specs look good.

I wanted rechargable battery, so I went with:
- Generic 14300 3.7V lipo cell. Says 1800mAh, but I don't believe it.
- Holder for it (1x AA)
- Generic lipo charger/protection board
- Pololu's soft power switch
- A momentary button

```
Power connections:
  USB is exposed outside the box
  VBUS goes to charger in
         (so USB input can be used to charge battery)
  VSYS is connected to switch output via diode
  3V3_EN is connnected to switch output via resistor
         (low=shutdown, so when device is off but USB is supplying power. pico is off)


Charger/protection board is generic TP4056 model
It came with 1A charge current, but I modified it to ~500mA current

Note that LEDs draw directly from battery, so the actual charging current is less
when display is on. Current use:
- pixels brightness 1: 80 to 240 mA 

The current is set by resistor labeled R3
The stock R3 was 122, which is 1.2K, which is ~1A
I replaced it with 2201, which means 2.2K, which gave me ~0.56A limit

 PICO:VBUS               GND
   |                      |
[ VIN+                   VIN- ]
[                             ]
[ charger / protection board  ]
[ (upside-down to expose LED) ]
[ modded to decrease Icharge  ]
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
     +---PICO:3V3_EN   (so switch works even for USB input)
     +--[R:10K]--GND        
     +--Vcc of LED displays
     |
     \--|>|--PICO:VSYS
```

