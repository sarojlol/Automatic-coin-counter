#include <Arduino.h>
#include <TMCStepper.h>
#include <pin_define.h>

void homeX();
void motor(int stepDelay, bool direction);
void motor_stop();
void tmc_setup();
void task_setup();
void pin_setup();

int stepTime = 160;

TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);
bool shaftVal = false;
bool stalled = false;

int step_delay;
bool step_dir;
bool motorStop = true;

bool startSW_flag;
bool startSW_state;
bool counting_toggle;

void stallInterruptX(){ // flag set when motor stalls
  stalled = true;
}

void setup() {
  tmc_setup();
  Serial.begin(115200);
  task_setup();
  pin_setup();
}

void loop() {
    //start button 
    static unsigned long start_filter;
    if ((millis() - start_filter) > 20)
    {
      startSW_state = digitalRead(start_SW);
      if (startSW_state == LOW &! startSW_flag)
      {
        motorStop = false;
        counting_toggle =! counting_toggle;
        startSW_flag = true;
      }
      else if (startSW_state == HIGH && startSW_flag)
      {
        startSW_flag = false;
      }
      start_filter = millis();
    }
    

    static bool start_flag = false;
    static unsigned long stall_delay;
    if (counting_toggle)
    {
      if (!stalled &! start_flag)
      {
        stall_delay = millis();
        motor(500, true);
        start_flag = true;
        Serial.println(start_flag);
      }
      if ((millis() - stall_delay) > 500)
      {
        if (stalled && start_flag)
        {
          motor_stop();
          start_flag = false;
          counting_toggle = false;
        }
      }
      else
      {
        stalled = false;
      }
    }
    else if (!counting_toggle)
    {
      motor_stop();
      start_flag = false;
    }
    if (!motorStop){
      ledcWrite(0, 128);
    }
}

// void stepper_task(void * pvparameter)
// {
//   tmc_setup();
//   for (;;)
//   {
//   }
// }
void motor_stop(){
  stalled = false;
  ledcWrite(0, 0);
  motorStop = true;
}

void motor(int speed, bool direction){
  step_dir = direction;
  
}

void task_setup(){
  //xTaskCreatePinnedToCore(stepper_task,"stepper_task", 1024, NULL, 1, NULL, 1);
}
void tmc_setup(){
  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);

  // shaft direction controlled through uart: driver.shaft(true or false)
  pinMode(STALL_PIN, INPUT);

  SERIAL_PORT.begin(115200); // HW UART drivers

  driver.begin(); // SPI: Init CS pins and possible SW SPI pins
  driver.toff(4); // Enables driver in software, changed from 5
  driver.blank_time(24);
  driver.rms_current(stepper_current); // Set motor RMS current
  driver.microsteps(2); // Set microsteps to 1/16th

  //driver.en_pwm_mode(true); // Toggle stealthChop on TMC2130/2160/5130/5160
  //driver.en_spreadCycle(false); // Toggle spreadCycle on TMC2208/2209/2224
  driver.pwm_autoscale(true); // Needed for stealthChop
  driver.semin(5);
  driver.semax(2);
  driver.sedn(0b01);
  driver.shaft(shaftVal);
  // TCOOLTHRS needs to be set for stallgaurd to work //
  driver.TCOOLTHRS(0xFFFFF); // 20bit max
  driver.SGTHRS(STALL_VALUE);
  attachInterrupt(digitalPinToInterrupt(STALL_PIN), stallInterruptX, RISING);
  digitalWrite(EN_PIN, LOW); // Enable driver in hardware
}

void pin_setup(){
  ledcAttachPin(STEP_PIN, 0);
  ledcSetup(0, 500, 8);

  pinMode(start_SW, INPUT_PULLUP);
}