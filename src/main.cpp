#include <Arduino.h>
#include <pin_define.h>
#include <stepper.h>

void task_setup();
void pin_setup();

bool startSW_flag;
bool startSW_state;
bool counting_toggle = true;
int start_flag;
int stall_count;

int sensor_pin[4] = {baht1_pin, baht2_pin, baht5_pin, baht10_pin};
bool sensor_data[4];
bool sensor_flag[4];
unsigned long sensor_filter[4];
int raw_baht[4] = {-1, -1, -1, -1};

void setup() 
{
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
        //if motor stalled again then run forward
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
      //after reverse handle
      else if (start_flag == 3){
        if (stall_count >= 4)
        {
          stall_count = 5;
          motor_stop();
          counting_toggle = false;
          startSW_flag = true; 
        }
        else if ((millis() - afterStall_delay) > 1000)
        {
          if (stall_count <=5)
          {
            stall_count = 0;
            start_flag = 0;
          }
        }
      }
    }
    if (stall_count >= 5)
    {
    digitalWrite(motor_led, LOW);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    digitalWrite(motor_led, HIGH);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    }
  }
}

void button_handle(void * pvparameter)
{
  for (;;)
  {
    //handle push button
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

    //stop counting
    else if (!counting_toggle && startSW_flag)
    {
      motor_stop();
      digitalWrite(motor_led, LOW);
      start_flag = 0;
    }
    vTaskDelay(20 / portTICK_PERIOD_MS); //debounce
  }
}

void sensor_handle(void * pvparameter)
{
  for(;;)
  {
    for (int i=0; i<4; i++){sensor_data[i] = digitalRead(sensor_pin[i]);} //read all sensor state

    //1 baht
    if ((!sensor_data[0]) &! (sensor_flag[0]))
    {
      raw_baht[0] += 1;
      sensor_filter[0] = millis();
      sensor_flag[0] = true;
    }
    else if ((sensor_data[0]) && (sensor_flag[0]))
    {
      if ((millis() - sensor_filter[0]) > 10)
      {
        sensor_flag[0] = false;
      }
    }

    //2 baht
    if ((!sensor_data[1]) &! (sensor_flag[1]))
    {
      raw_baht[1] += 2;
      sensor_filter[1] = millis();
      sensor_flag[1] = true;
    }
    else if ((sensor_data[1]) && (sensor_flag[1]))
    {
      if ((millis() - sensor_filter[1]) > 10)
      {
        sensor_flag[1] = false;
      }
    }

    //5 baht
    if ((!sensor_data[2]) &! (sensor_flag[2]))
    {
      raw_baht[2] += 5;
      sensor_filter[2] = millis();
      sensor_flag[2] = true;
    }
    else if ((sensor_data[2]) && (sensor_flag[2]))
    {
      if ((millis() - sensor_filter[2]) > 10)
      {
        sensor_flag[2] = false;
      }
    }

    //10 baht
    if ((!sensor_data[3]) &! (sensor_flag[3]))
    {
      raw_baht[3] += 10;
      sensor_filter[3] = millis();
      sensor_flag[3] = true;
    }
    else if ((sensor_data[3]) && (sensor_flag[3]))
    {
      if ((millis() - sensor_filter[3]) > 10)
      {
        sensor_flag[3] = false;
      }
    }

    for (int i=0; i<4; i++){raw_baht[i] = max(raw_baht[i], 0);} //limit raw baht data to minimum of 0
    //Serial.print(raw_baht[1]);
    //Serial.print(raw_baht[2]);
    Serial.println(raw_baht[0]);
    vTaskDelay(10 / portTICK_PERIOD_MS); //delay for other tasks to continue
  }
}

void task_setup()
{
  xTaskCreatePinnedToCore(button_handle,"button_handle", 1024, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(motor_handle,"motor_handle", 1024, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(sensor_handle,"sensor_handle", 10000, NULL, 3, NULL, 1);
}

void pin_setup()
{
  pinMode(motor_led, OUTPUT);
  pinMode(start_SW, INPUT_PULLUP);
  for (int i=0; i<4; i++){pinMode(sensor_pin[i], INPUT);}
}