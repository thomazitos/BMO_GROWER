 #include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const uint8_t sun_icon[] PROGMEM = {
  0b00011000,
  0b00011000,
  0b01111110,
  0b11111111,
  0b11111111,
  0b01111110,
  0b00011000,
  0b00011000,
};

const uint8_t moon_icon[] PROGMEM = {
  0b01100000,
  0b11110000,
  0b11111100,
  0b11111110,
  0b11111111,
  0b01111111,
  0b00111110,
  0b00001100,
};

void drawSunIcon(int x, int y) {
  display.drawBitmap(x, y, sun_icon, 8, 8, WHITE);
}

void drawMoonIcon(int x, int y) {
  display.drawBitmap(x, y, moon_icon, 8, 8, WHITE);
}

void setup() {
  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.display();
}

void loop() {
  display.clearDisplay();

  // Draw sun icon at position (10, 2)
  drawSunIcon(10, 2);

  // Draw moon icon at position (100, 2)
  drawMoonIcon(100, 2);

  // Update the display
  display.display();

  delay(1000);
}
