void set_pressure(int target_pressure);
int get_pressure();
void update();
void start_brew(int mode);
void set_maxpressure(int pressure);
void set_preinfusetime(int time);

int maxpressure = 172;
int pre_infuse_time;


void set_pressure(int target_pressure) {
  if (target_pressure >= maxpressure) {
    target_pressure = maxpressure;
  }
  
}
int get_pressure() {
  return calculatePressure();

}
void update() {

}
void start_brew(int mode) {
  switch(mode) {
    case 1:
      //statement 1
      break;
    case 2:
      // do something
      break;
    default: 
      // do nothing
      break;
  }

}
void set_maxpressure(int max_pressure) {
  maxpressure = max_pressure;

}
void set_preinfusetime(int time) {
  pre_infuse_time = time;
}

void setup() {
  // put your setup code here, to run once:
     Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(get_pressure());

}
