#include <Arduino.h>
#include <pin_define.h>
#include <stepper.h>

void task_setup();
void pin_setup();

bool startSW_flag;
bool startSW_state;
bool counting_toggle;

void setup() {
  tmc_setup();
  Serial.begin(115200);
  task_setup();
  pin_setup();
}

void loop() {
  motor_run();

  //start button 
  static unsigned long start_filter;
  if ((millis() - start_filter) > 20)
  {
    startSW_state = digitalRead(start_SW);
    if (startSW_state == LOW &! startSW_flag)
    {
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
  if (counting_toggle)
  {
    if (!motor_stalled() &! start_flag)
    {
      motor(500, false);
      start_flag = true;
    }
    if (motor_stalled() && start_flag)
    {
      motor_stop();
      start_flag = false;
      counting_toggle = false;
    }
  }
  else if (!counting_toggle)
  {
    motor_stop();
    start_flag = false;
  }
}

// void stepper_task(void * pvparameter)
// {
//   tmc_setup();
//   for (;;)
//   {
//   }
// }


void task_setup(){
  //xTaskCreatePinnedToCore(stepper_task,"stepper_task", 1024, NULL, 1, NULL, 1);
}

void pin_setup(){

  pinMode(start_SW, INPUT_PULLUP);
}