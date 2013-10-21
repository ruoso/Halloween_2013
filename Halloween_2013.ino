#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_TSL2561.h>

#define PIN1 6
#define PIN2 7
#define NUM_LEDS 72
#define COLOR_COUNT 6
#define LIGHT_THRESHOLD 10

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(NUM_LEDS, PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(NUM_LEDS, PIN2, NEO_GRB + NEO_KHZ800);
Adafruit_LSM303_Accel accel = Adafruit_LSM303_Accel(1);
Adafruit_LSM303_Mag mag = Adafruit_LSM303_Mag(2);
Adafruit_TSL2561 tsl = Adafruit_TSL2561(TSL2561_ADDR_FLOAT, 12345);

void setup() {
  strip1.begin();
  strip2.begin();
  accel.begin();
  mag.begin();
  tsl.enableAutoGain(true);
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
}

uint32_t black = strip1.Color(0,0,0);
int current_led = 0;
int sign = 1;

sensors_event_t event_a;  
sensors_event_t event_m;
sensors_event_t event_l;
struct last_e {
  int x;
  int y;
  int z;
} last_e;

void light_event() {
  uint32_t red = strip1.Color(127,0,0);
  uint32_t green = strip1.Color(0,127,0);
  uint32_t blue = strip1.Color(0,0,127);
  for (int j = 0; j < 10; j++) {
    for (int i = 1; i <= NUM_LEDS - 3; i++) {
      strip1.setPixelColor(i-1, black);
      strip1.setPixelColor(i, red);
      strip1.setPixelColor(i+1, green);
      strip1.setPixelColor(i+2, blue);
      strip2.setPixelColor(i-1, black);
      strip2.setPixelColor(i, red);
      strip2.setPixelColor(i+1, green);
      strip2.setPixelColor(i+2, blue);
      strip1.show();
      strip2.show();
    }
  }
  strip1.setPixelColor(NUM_LEDS - 3, black);
  strip1.setPixelColor(NUM_LEDS - 2, black);
  strip1.setPixelColor(NUM_LEDS - 1, black);
  strip2.setPixelColor(NUM_LEDS - 3, black);
  strip2.setPixelColor(NUM_LEDS - 2, black);
  strip2.setPixelColor(NUM_LEDS - 1, black);
  strip1.show();
  strip2.show();
}

int light = 0;
void loop() {
  tsl.getEvent(&event_l);
  if (light) {
    if (event_l.light < LIGHT_THRESHOLD) {
      light_event();
      light = 0;
    }
  } else {
    if (event_l.light > LIGHT_THRESHOLD) {
      light_event();
      light = 1;
    }
  }
  accel.getEvent(&event_a);
  if (event_a.acceleration.z - last_e.z > 3 || event_a.acceleration.z - last_e.z < -3 ||
      event_a.acceleration.y - last_e.y > 3 || event_a.acceleration.y - last_e.y < -3 ||
      event_a.acceleration.x - last_e.x > 3 || event_a.acceleration.x - last_e.x < -3
      ) {
    mag.getEvent(&event_m);
    strip1.setPixelColor(current_led, black);
    strip2.setPixelColor(current_led, black);
    current_led += sign;
    uint32_t color = strip1.Color(event_m.magnetic.x * 4, event_m.magnetic.y * 4, event_m.magnetic.z * 4);
    strip1.setPixelColor(current_led, color);
    strip2.setPixelColor(current_led, color);
    strip1.show();
    strip2.show();
    if (current_led > NUM_LEDS - 1 || current_led < 1)
    {
      sign *= -1;
    }
  }
  last_e.x = event_a.acceleration.x;
  last_e.y = event_a.acceleration.y;
  last_e.z = event_a.acceleration.z;
}
