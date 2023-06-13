#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "TomThumb.h" // Include the custom font
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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
  int x[] = {-4, -3, -2, -1, 0};
  int y[] = {10, 20, 30, 40, 30};
  int length = sizeof(x) / sizeof(x[0]);
  char y_title[] = "TEMP";

  plotGraph(x, y, length, y_title);
  delay(1000);
}

void plotGraph(int x[], int y[], int length, char y_title[]) {
  int minX = -4, maxX = 0, minY = 10, maxY = 40;


  display.clearDisplay();
  display.setTextColor(WHITE);

 // Set custom font (TomThumb)
  display.setFont(&TomThumb);
  // Draw the axes
  display.drawLine(10, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, WHITE);
  display.drawLine(10, SCREEN_HEIGHT - 1, 10, 1, WHITE);

  // Connect the data points
  for (int i = 1; i < length; i++) {
    int scaledX1 = map(x[i - 1], minX, maxX, 11, SCREEN_WIDTH - 1);
    int scaledY1 = map(y[i - 1], minY, maxY, 7, SCREEN_HEIGHT - 2);
    int scaledX2 = map(x[i], minX, maxX, 11, SCREEN_WIDTH - 1);
    int scaledY2 = map(y[i], minY, maxY, 7, SCREEN_HEIGHT - 2);
    display.drawLine(scaledX1, SCREEN_HEIGHT - scaledY1 - 1, scaledX2, SCREEN_HEIGHT - scaledY2 - 1, WHITE);
  }
  // Display titles
  display.setTextSize(1);
  display.setCursor(20, 6);
  display.print(y_title);
  
// Display axis values and divide Y into 5 parts and X into 10 parts
  for (int i = 0; i <= 3; i++) {
    int yPos = map(i, 0, 4, SCREEN_HEIGHT , 0)-1;
    display.setCursor(1, yPos);
    display.print(minY*(i+1));
  }

  display.display();
}
