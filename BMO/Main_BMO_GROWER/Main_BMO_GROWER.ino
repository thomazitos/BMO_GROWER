/*
    BMO GROWER
*/
#include <Wire.h>
#include "SparkFunBME280.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DS3231.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include "TomThumb.h" // Include the custom font


#include "WiFiEsp.h"
#include "SoftwareSerial.h"
#include "secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros


/***********************************************************************
 Definição dos Pinos
 ***********************************************************************/
#define on_pot_pin A7
#define off_pot_pin A3
#define relay_pin 13
#define btn_pin 3                   // botão para mudar estado

/***********************************************************************
 Variáveis Globais
 ***********************************************************************/
// Estados
#define CONFIG_LED 0
#define DATA_T 1
#define DATA_H 2

// Config Display OLED
#define SCREEN_WIDTH 128            // OLED display width, in pixels
#define SCREEN_HEIGHT 32            // OLED display height, in pixels
#define OLED_RESET     -1           // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C         // 0x3D for 128x64, 0x3C for 128x32

// Ciclo Circadiano
int horario;                        // horário atual
int LigaValue = 0;                  // hora para ligar LED (formato 24h)
int DesligaValue = 0;               // hora para desligar LED (formato 24h)
bool Liga_LED;                      // TRUE: ligado, FALSE: desligado

// Config Temperatura e Umidade
#define sample_t 3600000            // periodo de amostragem em segundos (1h)
unsigned long last_time_lcd = 0;
unsigned long last_time_aquisition = 0;
int TEMP[]={0,0,0,0,0,0,0,0,0,0,0,0,0};
int HUMI[]={0,0,0,0,0,0,0,0,0,0,0,0,0};
int T[]={-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0};

int estado = 0 ;

// Config WiFi
char ssid[] = "Inovalab";            // your network SSID (name)
char pass[] = "inovalabpsi21";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiEspClient  client;

// ThingSpeak
#define sample_wifi 30000       // mandar dados de 30 em 30s
unsigned long last_time_wifi = 0;
unsigned long myChannelNumber = 2190503;
const char * myWriteAPIKey = "ERLQAPMEJONRCPKI";

/***********************************************************************
 Componentes
 ***********************************************************************/
// OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// RTC
DS3231  rtc(SDA, SCL);              // Init the DS3231 using the hardware interface

// LCD
LiquidCrystal_I2C lcd(0x27,16,2);   // Set the LCD address to 0x27 for a 16 chars and 2 line display

// BME (sensor de umidade e temperatura)
BME280 bme;                         // Uses I2C address 0x76 (jumper closed)

// WiFi module
SoftwareSerial Serial1(10, 11); // RX, TX

/***********************************************************************
 mudaEstado
 Função para tratar a interrupção do botão --> mudar de estado.
 Parametros de entrada: (int) array
                        (int) tamanho do array
                        (int) valor a ser adicionado
 Retorno: nenhum
 ***********************************************************************/
void mudaEstado() {
  Serial.println("mudei");
  estado++;
  if (estado>3) {
    estado=0;
  }   
}

/***********************************************************************
 shiftLeft
 Adiciona valor no final do array e retira o primeiro valor para manter 
 o seu tamanho.
 Parametros de entrada: (int) array
                        (int) tamanho do array
                        (int) valor a ser adicionado
 Retorno: nenhum
 ***********************************************************************/
void shiftLeft(int arr[], int len, int value) {
  for (int i = 0; i < len - 1; i++) {
    arr[i] = arr[i+1];
  }
  arr[len-1] = value;
}

/***********************************************************************
 logicaLED
 Analisa horário atual para identificar horário de claro e escuro conforme
 as configurações colocadas.
 Parametros de entrada: nenhum
 Retorno: nenhum
 ***********************************************************************/
void logicaLED() {
  // pega horário atual (retorna em string -> transforma para int)
  horario = (rtc.getTimeStr()[0] - '0')*10 + (rtc.getTimeStr()[1] - '0');

  if (LigaValue > DesligaValue) {
    if  (horario > DesligaValue && horario < LigaValue) {
      // período de escuro
      Liga_LED = false;
      digitalWrite(relay_pin, LOW);
    }
    else {
      // período de claro
      Liga_LED = true;
      digitalWrite(relay_pin, HIGH); 
    }
  }
  else {
    if (horario > LigaValue && horario < DesligaValue) {
      // período de claro
      Liga_LED = true;
      digitalWrite(relay_pin, HIGH);
    }
    else {
      // período de escuro
      Liga_LED = false;
      digitalWrite(relay_pin, LOW);
    }
  }
}

/***********************************************************************
 printLCD
 Exibe os valores atuais de temperatura e umidade no LCD.
 Parametros de entrada: nenhum
 Retorno: nenhum
 ***********************************************************************/
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

/***********************************************************************
 printCONFIG
 Modo de exibição de configuração do ciclo circadiano.
 Parametros de entrada: nenhum
 Retorno: nenhum
 ***********************************************************************/
void printCONFIG(){
  
  // recebe dados dos potenciometros colocados pelo usuário
  LigaValue = map(analogRead(on_pot_pin), 0, 1023, 0, 24);
  DesligaValue = map(analogRead(off_pot_pin), 0, 1023, 0, 24);
  
  // Display FOTOPERIOD (linha 1)
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

/***********************************************************************
 printCONFIG
 Modo de exibição do gráfico de temperatura no tempo.
 Parametros de entrada: nenhum
 Retorno: nenhum
 ***********************************************************************/
void printDATA_T(){
  plotGraph(T, TEMP, 13, "TEMP", -12, 0, 10, 40, 3);
  display.clearDisplay();  // Clear the buffer
  delay(100);
}

/***********************************************************************
 printCONFIG
 Modo de exibição do gráfico de umidade no tempo.
 Parametros de entrada: nenhum
 Retorno: nenhum
 ***********************************************************************/
void printDATA_H(){
  plotGraph(T, HUMI, 13, "HUMI", -12, 0, 0, 100, 4);
  display.clearDisplay();  // Clear the buffer
  delay(100);
}

/***********************************************************************
 plotGraph
 Plotar o gráfico no display OLED dados os parâmetros de entrada.
 Parametros de entrada: (int) array tempo
                        (int) array dados
                        (int) numero de dados
                        (char) array titulo
                        (int) valor mínimo de x
                        (int) valor máximo em x
                        (int) valor mínimo de y
                        (int) valor máximo em y
 Retorno: nenhum
 ***********************************************************************/
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
    if (y[i] > maxY) {
      y[i] = maxY;
    }
    if (y[i] < minY) {
      y[i] = minY;
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
    display.print(map(i, 0, d_y, minY, maxY));
    display.drawLine(11, yPos, 14, yPos, WHITE);
  }

  display.display();
}

/***********************************************************************
 drawDottedLine
 Desenhar linha tracejada no display OLED dados posições inicial e final.
 Parametros de entrada: (int) posicao inicial x
                        (int) posicao inicial y
                        (int) posicao final x
                        (int) posicao final y
                        (int) tamanho do ponto
 Retorno: nenhum
 ***********************************************************************/
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

/***********************************************************************
 drawSunIcon
 Desenha o ícone de Sol no display OLED.
 Parametros de entrada: (int) posicao x
                        (int) posicao y
                        (int) tamanho
 Retorno: nenhum
 ***********************************************************************/
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

/***********************************************************************
 drawMoonIcon
 Desenha o ícone de Lua no display OLED.
 Parametros de entrada: (int) posicao x
                        (int) posicao y
                        (int) tamanho
 Retorno: nenhum
 ***********************************************************************/
void drawMoonIcon(int x, int y, int size) {
  // Draw the moon crescent
  int crescentSize = size / 2;
  int crescentX = x + size / 4;
  int crescentY = y + size / 4;
  display.fillCircle(crescentX, crescentY, crescentSize, WHITE);
  display.fillCircle(crescentX + size / 2, crescentY, crescentSize, BLACK);
}

/************************************************************************
 Main
 Loop principal de controle que executa a maquina de estados
 Parametros de entrada: nenhum
 Retorno: nenhum
*************************************************************************/
void setup() {
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);

  // initialize serial for ESP module
  Serial1.begin(9600);

  // initialize ESP module
  WiFi.init(&Serial1);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }

    // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // ThingSpeak
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  
  // OLED 0.96
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setFont(&TomThumb);

  // RTC
  rtc.begin();
  
  // LCD
  lcd.init();                      
  lcd.backlight();
  
  // BME
  bme.setI2CAddress(0x76); //Connect to a second sensor
  if(bme.beginI2C() == false) Serial.println("bme connect failed");

  // BUTTON
  pinMode(btn_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(btn_pin), mudaEstado, FALLING);

  // RELEY
  pinMode(relay_pin, OUTPUT);

  // POTENTIOMETERS
  LigaValue = map(analogRead(on_pot_pin), 0, 1023, 24, 0);      // 0 -> 1023 : 24 -> 0
  DesligaValue = map(analogRead(off_pot_pin), 0, 1023, 0, 24);  // 0 -> 1023 : 0 -> 24

  shiftLeft(TEMP, 13, int(bme.readTempC()));                    // adiciona leitura de temperatura ao array
  shiftLeft(HUMI, 13, int(bme.readFloatHumidity()));            // adiciona leitura de umidade ao array

}

void loop() {
  
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");
  }

  // Set claro e escuro
  logicaLED();

  // Atualização do LCD
  if (millis() - last_time_lcd > 1000) {      // a cada 1s
    printLCD();
    last_time_lcd = millis();
  }

  if (millis() - last_time_aquisition > sample_t) {
    // adiciona ultimo leitura de temperatura e umidade
    shiftLeft(TEMP, 13, int(bme.readTempC()));
    shiftLeft(HUMI, 13, int(bme.readFloatHumidity()));
    
    last_time_aquisition = millis();

    for (int i = 0; i < 13; i++) {
      Serial.print(HUMI[i]);
      Serial.print(" ");
    }

    Serial.println();
  }

  // Envio de dados WiFi
  if (millis() - last_time_wifi > sample_wifi) {
    ThingSpeak.setField(1, int(bme.readTempC()));
    ThingSpeak.setField(1, int(bme.readFloatHumidity()));
    last_time_wifi = millis();
  }

  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  
  // debugPrint();
  switch(estado){
    case CONFIG_LED:      // configuração do ciclo circadiano
      printCONFIG();
      return;
    case DATA_T:          // exibição do gráfico de temp x t
      printDATA_T();
      return;
    case DATA_H:          // exibição do gráfico de umid x t
      printDATA_H();
      return;      
  }
  
}
/*
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
*/

/*
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
*/





