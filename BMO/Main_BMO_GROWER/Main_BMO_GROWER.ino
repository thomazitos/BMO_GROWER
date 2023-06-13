#include <Wire.h>
#include "SparkFunBME280.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DS3231.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include "TomThumb.h" // Include the custom font
// Denifing pins
#define on_pot_pin A7
#define off_pot_pin A3
#define relay_pin 2
#define btn_pin 3

// estados
#define CONFIG_LED 0
#define DATA_T 1
#define DATA_H 2

int LigaValue = 0; 
int DesligaValue = 0; 
bool Liga_LED;
unsigned long last_time_lcd=0;
unsigned long last_time_aquisition=0;
#define sample_t 3600000 // periodo de amostragem em segundos
int horario;
int estado = 0 ;
int TEMP[]={0,0,0,0,0,0,0,0,0,0,0,0,0};
int HUMI[]={0,0,0,0,0,0,0,0,0,0,0,0,0};
int T[]={-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0};
//Botão oled


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
// create the display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

BME280 bme; //Uses I2C address 0x76 (jumper closed)

// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);


void mudaEstado(){
  Serial.println("mudei");
  estado++;
  if(estado>3){
    estado=0;
   }
    
}

void setup() {
  
  
  Serial.begin(9600);
  
   // OLED 0.96
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setFont(&TomThumb);

  // RTC
  rtc.begin();
  
  // LcD
  lcd.init();                      
  lcd.backlight();
  
  // BME
  bme.setI2CAddress(0x76); //Connect to a second sensor
  if(bme.beginI2C() == false) Serial.println("bme connect failed");


  // BUTTON
  pinMode(btn_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(btn_pin), mudaEstado, FALLING);

  // RELEY
  pinMode(relay_pin,OUTPUT);

  //POTENTIOMETERS
  LigaValue = map(analogRead(on_pot_pin),0,1023,24,0);
  DesligaValue = map(analogRead(off_pot_pin),0,1023,0,24);

  shiftLeft(TEMP,13,int(bme.readTempC()));
  shiftLeft(HUMI,13,int(bme.readFloatHumidity()));
}

void loop() {
  
  logicaLED();
  if(millis()-last_time_lcd>1000){
    printLCD();
    last_time_lcd=millis();
    
  }
  if(millis()-last_time_aquisition>sample_t){
    shiftLeft(TEMP,13,int(bme.readTempC()));
    shiftLeft(HUMI,13,int(bme.readFloatHumidity()));
    last_time_aquisition=millis();
    for (int i = 0; i < 13; i++) {
      Serial.print(HUMI[i]);
      Serial.print(" ");
    }
    Serial.println();
  }
  
  
  // debugPrint();
  switch(estado){
       case CONFIG_LED:
         printCONFIG();
         return;
       case DATA_T:
         printDATA_T();
         return;
       case DATA_H:
         printDATA_H();
         return;
         
             
  }
  
}

void printTIME(){
  // Display Text - Dia da semana
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(rtc.getDOWStr());
  display.print("  ");
  display.print(rtc.getDateStr());

  // Desenha borda do relógio (com cantos arredondados)
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.drawRoundRect(0, 10, 128,20, 10, WHITE);

  
  // Display Text - Hora
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(17, 13);
  display.print(rtc.getTimeStr());

  // exibe no display
  display.display();
  
  display.clearDisplay();  // Clear the buffer
  }
  
void printDATA_T(){
  plotGraph(T, TEMP, 13, "TEMP",-12,0,10,40,3);
  display.clearDisplay();  // Clear the buffer
  delay(100);
  }
  
void printDATA_H(){
  plotGraph(T, HUMI, 13, "HUMI",-12,0,0,100,4);
  display.clearDisplay();  // Clear the buffer
  delay(100);
  }
void printCONFIG(){
  
  LigaValue = map(analogRead(on_pot_pin),0,1023,0,24);
  DesligaValue = map(analogRead(off_pot_pin),0,1023,0,24);
  
  // Display FOTOPERIOD
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 6);
  display.print("     FOTOPERIODO   ");
  display.print(rtc.getTimeStr());

  // Desenha init 
  display.setTextSize(1); 
  display.setTextColor(WHITE);
  display.drawRoundRect(0, 8, 63,24, 8, WHITE);
  drawSunIcon(11,17,7);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(35, 25);
  display.print(LigaValue);
  
  // Desenha end
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.drawRoundRect(63, 8, 63,24, 8, WHITE);
  drawMoonIcon(78,18,10);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(98, 25);
  display.print(DesligaValue);

 
  // exibe no display
  display.display();
  display.clearDisplay();  // Clear the buffer

  }
void printLCD(){
  lcd.clear();
  lcd.print("  TEMP: ");
  lcd.print(bme.readTempC());
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("  HUMI: ");
  lcd.print(bme.readFloatHumidity());
  lcd.print("%");

}
void logicaLED(){
  horario=(rtc.getTimeStr()[0]-'0')*10+(rtc.getTimeStr()[1]-'0');
  //Logica para ligar ou desligar led
  if(LigaValue>DesligaValue){
    if(horario>DesligaValue && horario<LigaValue){
      Liga_LED=false;
      digitalWrite(relay_pin,LOW);
    }else{
      Liga_LED=true;
      digitalWrite(relay_pin,HIGH);
      }
    }else{
      if(horario>LigaValue && horario<DesligaValue){
        Liga_LED=true;
        digitalWrite(relay_pin,HIGH);
       }else{
        Liga_LED=false;
        digitalWrite(relay_pin,LOW);
        }
     }
}

void debugPrint(){
  // BME
  Serial.print(" HumidityB: ");
  Serial.print(bme.readFloatHumidity(), 0);
  Serial.print(" PressureB: ");
  Serial.print(bme.readFloatPressure(), 0);
  Serial.print(" TempB: ");
  Serial.print(bme.readTempC(), 2);
  
  //LED  
  Serial.print("Liga:");
  Serial.print(LigaValue);
  Serial.print("   Desliga:");
  Serial.print(DesligaValue);
  Serial.print("   LED:");
  Serial.print(Liga_LED);
  Serial.print("   Horario:");
  Serial.println(horario);

  //LED  
  Serial.print("Estado ");
  Serial.print(estado);
  
}
void shiftLeft(int arr[], int len, int value) {
  for (int i = 0; i < len - 1; i++) {
    arr[i] = arr[i+1];
  }
  
  // Put the value at the end of the array
  arr[len - 1] = value;
}

void plotGraph(int x[], int y[], int length, char y_title[],int minX ,int maxX,int minY,int maxY,int d_y) {


  display.clearDisplay();
  display.setTextColor(WHITE);

 // Set custom font (TomThumb)
  display.setFont(&TomThumb);
  // Draw the axes
  display.drawLine(12, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, WHITE);
  display.drawLine(12, SCREEN_HEIGHT - 1, 12, 1, WHITE);

  // Connect the data points
  for (int i = 1; i < length; i++) {
    if(y[i]>maxY){
      y[i]=maxY;
    }
    if(y[i]<minY){
      y[i]=minY;
    }
    int scaledX1 = map(x[i - 1], minX, maxX, 12, SCREEN_WIDTH - 1);
    int scaledY1 = map(y[i - 1], minY, maxY, 1, SCREEN_HEIGHT - 2);
    int scaledX2 = map(x[i], minX, maxX, 12, SCREEN_WIDTH - 1);
    int scaledY2 = map(y[i], minY, maxY, 1, SCREEN_HEIGHT - 2);
    display.drawLine(scaledX1, SCREEN_HEIGHT - scaledY1 - 1,scaledX2, SCREEN_HEIGHT - scaledY2 - 1, WHITE);
  }
  drawDottedLine(12, SCREEN_HEIGHT - map(y[12], minY, maxY, 1, SCREEN_HEIGHT - 2) - 1,SCREEN_WIDTH - 1, SCREEN_HEIGHT - map(y[12], minY, maxY, 1, SCREEN_HEIGHT - 2) - 1,3);
  // Display titles
  display.setTextSize(1);
  display.setCursor(25, 6);
  display.print(y_title);
  
// Display axis values and divide Y into 5 parts and X into 10 parts
  for (int i = 1; i <= d_y; i++) {
    int yPos = SCREEN_HEIGHT - map(map(i,0,d_y,minY,maxY), minY, maxY, 1, SCREEN_HEIGHT - 2) - 1;
    display.setCursor(0, yPos+6);
    display.print(map(i,0,d_y,minY,maxY));
    display.drawLine(11, yPos,14, yPos, WHITE);
  }

  display.display();
}
void drawDottedLine(int startX, int startY, int endX, int endY, int dotSize) {
  // Calculate the distance between the start and end points
  float dx = endX - startX;
  float dy = endY - startY;
  float dist = sqrt(dx*dx + dy*dy);

  // Calculate the number of dots needed based on the distance and the dot size
  int numDots = dist / (dotSize * 2);

  // Calculate the x and y increments needed for each dot
  float xIncrement = dx / numDots;
  float yIncrement = dy / numDots;

  // Loop through each dot and draw it
  for (int i = 0; i < numDots; i++) {
    // Calculate the x and y coordinates of the current dot
    int x = startX + i * xIncrement;
    int y = startY + i * yIncrement;

    // Draw a dot at the current coordinates
    display.drawPixel(x, y, WHITE);
  }
}


void drawSunIcon(int x, int y, int size) {
  // Draw the sun rays
  int rayLength = size / 2;
  for (int i = 0; i < 8; i++) {
    float angle = i * PI / 4;
    int startX = x + size / 2 + rayLength * cos(angle);
    int startY = y + size / 2 + rayLength * sin(angle);
    int endX = x + size / 2 + 2 * rayLength * cos(angle);
    int endY = y + size / 2 + 2 * rayLength * sin(angle);
    display.drawLine(startX, startY, endX, endY, WHITE);
  }
  
  // Draw the sun face
  int faceSize = size / 2;
  int faceX = x + size / 2;
  int faceY = y + size / 2;
  display.fillCircle(faceX, faceY, faceSize, WHITE);
}

void drawMoonIcon(int x, int y, int size) {
  // Draw the moon crescent
  int crescentSize = size / 2;
  int crescentX = x + size / 4;
  int crescentY = y + size / 4;
  display.fillCircle(crescentX, crescentY, crescentSize, WHITE);
  display.fillCircle(crescentX + size / 2, crescentY, crescentSize, BLACK);
  
}
