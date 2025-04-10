#include <max6675.h>
#include <SPI.h>
#include <Wire.h>

int Temp_Clk = 18;
int Temp_Cs = 5;
int Temp_So = 19;


MAX6675 thermocouple(Temp_Clk, Temp_Cs, Temp_So);

float readTemperature() {
  return thermocouple.readCelsius();
}


void setup() {
  Serial.begin(115200);
}

void loop(){
  Serial.println(readTemperature());
  delay(500);
}