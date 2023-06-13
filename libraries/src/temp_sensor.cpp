/**
 * @file  temp_sensor.cpp
 *
 * @brief Temp sensor lib for DHT 11
 *
 * @author Thomaz Furukawa <thomazakira@usp.br>
 *
 * @date 08/2021
 * 
 * @copyright MIT License
 */


#include "temp_sensor.hpp"

/*****************************************
 * Class Methods Bodies Definitions
 *****************************************/

Temp_sensor::Temp_sensor(uint8_t pin, uint8_t type, uint8_t count) {
    (void)count; // Workaround to avoid compiler warning.
    this-> pin=pin;
    this-> type=type;
    #ifdef __AVR
    _bit = digitalPinToBitMask(pin);
    _port = digitalPinToPort(pin);
    #endif
    _maxcycles =
        microsecondsToClockCycles(1000); // 1 millisecond timeout for
                                        // reading pulses from DHT sensor.
    // Note that count is now ignored as the DHT reading algorithm adjusts itself
    // based on the speed of the processor.DHT dht(pin, type);

}
bool Temp_sensor::begin() {
    // set up the pins!
    pinMode(pin, INPUT_PULLUP);
    // Using this value makes sure that millis() - lastreadtime will be
    // >= MIN_INTERVAL right away. Note that this assignment wraps around,
    // but so will the subtraction.
    _lastreadtime = millis() - MIN_INTERVAL;
    DEBUG_PRINT("DHT max clock cycles: ");
    DEBUG_PRINTLN(_maxcycles, DEC);
    pullTime = usec; 
    float h = this->readHumi();
    float t = this->readTemp();
    if (isnan(h) || isnan(t)){
      std::cout <<"Falha no sensor"<<this->type()<< std::endl;
      return false;
    } 
    else{
      std::cout <<this->type()<<"conectado com sucesso"<< std::endl;
      return true;
    }  
}
float Temp_sensor::readTemp() {
  float f = NAN;

  if (read(force)) {
    switch (type) {
    case DHT11:
      f = data[2];
      if (data[3] & 0x80) {
        f = -1 - f;
      }
      f += (data[3] & 0x0f) * 0.1;
      
      break;
    case DHT12:
      f = data[2];
      f += (data[3] & 0x0f) * 0.1;
      if (data[2] & 0x80) {
        f *= -1;
      }
      
      break;
    case DHT22:
    case DHT21:
      f = ((word)(data[2] & 0x7F)) << 8 | data[3];
      f *= 0.1;
      if (data[2] & 0x80) {
        f *= -1;
      }
      break;
    }
  }
  return f;
}

float Temp_sensor::readHumi() {
  float f = NAN;
  if (read(force)) {
    switch (type) {
    case DHT11:
    case DHT12:
      f = data[0] + data[1] * 0.1;
      break;
    case DHT22:
    case DHT21:
      f = ((word)data[0]) << 8 | data[1];
      f *= 0.1;
      break;
    }
  }
  return f;
}
