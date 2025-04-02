#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
int screen_width = 128;
int screen_height = 64;

Adafruit_SSD1306 display(screen_width, screen_height, &Wire, -1);

const int pressurePin = A0;
void setup() {
 
  //Serial.begin(9600);
  Serial.begin(115200);

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.display();
  delay(300);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(F("Hello World"));

  display.clearDisplay();

  display.display();
  
  delay(2000);

}

void loop() {

  int i = 0;
  int totalValue = 0;
  int avgValue = 0;

  for (i; i < 20; i++) {
    delay(5);
    int sensorValue = analogRead(pressurePin);
    
    totalValue += sensorValue;
  }
  avgValue = totalValue / 20;

  float voltage = avgValue * (5.0/ 1024.0);

  float baseline = 0.5;

  float vMax = 4.5; 

  float offset  = 1.5;
  float barStep = 68.27;
  float pressure = avgValue / barStep - offset;

  float bar = pressure - 0.68;
  float psi = (pressure * 14.503773773) - 10;
  

  testDisplayPressure(psi, bar, voltage);
  // plotData(sensorValue, bar, psi, voltage);
  plotAvgData(avgValue, psi, bar, voltage);
}

// To display to screen
void testDisplayPressure(float psi, float bar, float voltage) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(15, 0);
  display.println(F("Current Pressure: "));
  display.println(F(""));

  display.setCursor(40, 25);
  display.print(psi);
  display.println(F(" psi"));

  display.setCursor(40, 35);
  display.print(voltage);
  display.println(F(" V"));

  display.setCursor(40, 45);
  display.print(bar);
  display.println(F(" bar"));

  display.display();

  delay(100);
}

// void plotData(float sensorValue, float psi, float bar, float voltage) {
//   Serial.println(psi);
// }

void plotAvgData(float avgSensorValue, float avgPsi, float avgBar, float avgVoltage) {
  Serial.println(avgSensorValue);
}


