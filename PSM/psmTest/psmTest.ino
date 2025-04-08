const byte valuePin = 32; // potentiometer input
const byte controlPin = 33; // triac output
const byte interruptPin = 25; // phase sensing

const unsigned int range = 127;
const unsigned int valueFactor = 4; // precalculated constant 1024 [10 bit ADC resolution] / 128 [range + 1] / 2

volatile bool zeroCross = false;

unsigned int intCount = 0;

unsigned long updateDelay = 500;
unsigned long lastUpdateTime = 0;

unsigned int value = 0;

unsigned int a = 0;
bool skip = false;

void setup() {
  pinMode(controlPin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), zeroCrossISR, RISING); // low-to-high transition = start of negative half-wave
  // EDIT actually we have no idea if its the positive or negative pulse. the pin goes high briefly at the start of both

  Serial.begin(115200);
}

IRAM_ATTR void zeroCrossISR() {
  zeroCross = true;
}

void debounceZeroCross() {
  static unsigned long firedTime;
  if (millis() < firedTime + 5) {
    zeroCross = false;
    return;
  }

  if (zeroCross) {
    firedTime = millis();
    intCount++;
    calculateSkip();
    zeroCross = false;
  }
}

void calculateSkip() {
  static bool wholeWaveFlag = false;

  if (wholeWaveFlag) {
    wholeWaveFlag = false;
    return;
  }
  else {
    wholeWaveFlag = true;

    a += value;

    if (a >= range) {
      // a -= range;
      a = 0;
      skip = false;
    } else {
      skip = true;
    }

    // if (a > range) {
    //   a = 0;
    //   skip = false;
    // }

    // updateControl();
  }
  
}

void updateControl() {
  if (skip) {
    digitalWrite(controlPin, LOW);
  } else {
    digitalWrite(controlPin, HIGH);
  }
}

void updateValue() {
  int readValue = analogRead(valuePin) / 4;
  Serial.print(readValue);
  int oldValue = value * valueFactor * 2;

  if (readValue > (oldValue + valueFactor * 3) || (readValue + valueFactor) < oldValue) { // add some hysteresis to filter out noise
    value = readValue / valueFactor / 2;
    a = 0;
  }
}



void loop() {
  unsigned long time = millis();
  static unsigned long lastTime;
  if (time > lastTime + updateDelay) {
    lastTime = time;

    updateValue();

    unsigned long dif = time - lastUpdateTime;
    unsigned long intPerSec = (intCount * 1000) / dif;
    Serial.printf(" int/sec:%i dif:%i\n", intPerSec, dif);
    
    
    // Serial.flush();

    // intCount = 0;
    // delay(20);
    // Serial.printf("after 20ms, intCount:%i", intCount);
    // Serial.println();
    

    intCount = 0;
    lastUpdateTime = time;
  }
  
  debounceZeroCross();
  updateControl();

}



