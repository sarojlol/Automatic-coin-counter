#include <Arduino.h>
#include <pin_define.h>
#include <stepper.h>

void task_setup();
void pin_setup();

bool startSW_flag;
bool startSW_state;
bool counting_toggle;
int start_flag;
int stall_count;

void setup() {
  Serial.begin(115200);
  tmc_setup();
  task_setup();
  pin_setup();
}

void loop() {

}

void motor_handle(void * pvparameter){
  for(;;)
  {
    motor_run();
    static unsigned long afterStall_delay;
    static unsigned long reverse_delay;
    if (counting_toggle)
    {
      //notmal operation
      if (!motor_stalled() && start_flag == 0)
      {
        motor(forware_speed, false);
        digitalWrite(motor_led, HIGH);
        start_flag = 1;
      }
      //initial jamming handle
      else if ((motor_stalled()) && (start_flag == 1 || start_flag == 3))
      {
        motor_stop();
        vTaskDelay(jamm_dir_delay / portTICK_PERIOD_MS);
        reverse_delay = millis();
        motor(reverse_speed, true);
        start_flag = 2;
        //if motor stalled during reversing
      }
      //handle reverse
      else if (start_flag == 2){
        if (motor_stalled())
        {
          motor_stop();
          vTaskDelay(jamm_dir_delay / portTICK_PERIOD_MS);
          motor(after_jamm_forward_speed, false);
          stall_count ++;
          afterStall_delay = millis();
          start_flag = 3;
        }
        //else just reverse to a point and resume
        else if ((millis() - reverse_delay) > 1000)
        {
          motor_stop();
          vTaskDelay(jamm_dir_delay / portTICK_PERIOD_MS);
          motor(after_jamm_forward_speed, false);
          stall_count ++;
          afterStall_delay = millis();
          start_flag = 3;
        }
      }
      else if (start_flag == 3){
        if (stall_count >= 4)
        {
          motor_stop();
          counting_toggle = false; 
        }
        if ((millis() - afterStall_delay) > 1000){
          stall_count = 0;
          start_flag = 0;
        }
      }
    }
  }
}

void button_handle(void * pvparameter)
{
  for (;;)
  {
    startSW_state = digitalRead(start_SW);
    if (startSW_state == LOW &! startSW_flag)
    {
      counting_toggle =! counting_toggle;
      stall_count = 0;
      start_flag = 0;
      startSW_flag = true;
    }
    else if (startSW_state == HIGH && startSW_flag)
    {
      startSW_flag = false;
    }
    else if (!counting_toggle)
    {
      motor_stop();
      digitalWrite(motor_led, LOW);
      start_flag = 0;
    }
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

void task_setup(){
  xTaskCreatePinnedToCore(button_handle,"button_handle", 1024, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(motor_handle,"motor_handle", 1024, NULL, 1, NULL, 1);
}

void pin_setup(){
  pinMode(motor_led, OUTPUT);
  pinMode(start_SW, INPUT_PULLUP);
}