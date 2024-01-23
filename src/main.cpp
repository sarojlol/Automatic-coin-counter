#include <Arduino.h>
#include <pin_define.h>
#include <stepper.h>
#include <LiquidCrystal_I2C.h>
#include <EEprom.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

void task_setup();
void pin_setup();
void lcd_setup();
void home_screen();
void restore_data();
void reset_screen();
void save_screen();
void coinFull_screen(int coin);

bool startSW_flag;
bool startSW_state;
int counting_stage;
int start_flag;
int stall_count;

int sensor_pin[4] = {baht1_pin, baht2_pin, baht5_pin, baht10_pin};
bool sensor_data[4];
bool sensor_flag[4];
unsigned long sensor_filter[4];
long raw_baht[4];
long limit_count[4] = {1, 1, 1, 1};
long total;
int last_total; 

int battery_percentage;

void setup() 
{
  Serial.begin(115200);
  pin_setup();
  tmc_setup();
  eeprom_setup();
  delay(10);
  restore_data();
  lcd_setup();
  task_setup();
}

void loop() {

}

void motor_handle(void * pvparameter){
  for(;;)
  {
    motor_run();
    static unsigned long afterStall_delay;
    static unsigned long reverse_delay;
    if (counting_stage == 1)
    {
      //notmal operation
      if (!motor_stalled() && start_flag == 0)
      {
        motor(forware_speed, true);
        digitalWrite(motor_led, HIGH);
        start_flag = 1;
      }
      //initial jamming handle
      else if ((motor_stalled()) && (start_flag == 1 || start_flag == 3))
      {
        motor_stop();
        vTaskDelay(jamm_dir_delay / portTICK_PERIOD_MS);
        reverse_delay = millis();
        motor(reverse_speed, false);
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
          motor(after_jamm_forward_speed, true);
          stall_count ++;
          afterStall_delay = millis();
          start_flag = 3;
        }
        //else just reverse to a point and resume
        else if ((millis() - reverse_delay) > 1000)
        {
          motor_stop();
          vTaskDelay(jamm_dir_delay / portTICK_PERIOD_MS);
          motor(after_jamm_forward_speed, true);
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
          counting_stage = 2;
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Jammed!");
          lcd.setCursor(0,1);
          lcd.print("Please clear stuck");
          lcd.setCursor(0,2);
          lcd.print("coin then press");
          lcd.setCursor(0,3);
          lcd.print("Start/Stop button");
          save_data(raw_baht[0], raw_baht[1], raw_baht[2], raw_baht[10], total);
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
    if (counting_stage == 0){
      digitalWrite(motor_led, LOW);
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
      switch (counting_stage)
      {
      case 0:
        counting_stage = 1;
        lcd.clear();
        home_screen();
        break;
      case 1:
        counting_stage = 0;
        digitalWrite(motor_led, LOW);
        motor_stop();
        lcd.clear();
        home_screen();
        break;
      case 2:
        counting_stage = 0;
        digitalWrite(motor_led, LOW);
        lcd.clear();
        home_screen();
        
        break;
      }
      stall_count = 0;
      start_flag = 0;
      startSW_flag = true;
    }
    else if (startSW_state == HIGH && startSW_flag)
    {
      startSW_flag = false;
    }

    static bool save_flag;
    static bool reset_flag;
    unsigned long reset_delay;
    if (counting_stage == 0)
    {
      if (digitalRead(save_button) && save_flag)
      {
        save_data(raw_baht[0], raw_baht[1], raw_baht[2], raw_baht[10], total);
        save_screen();
        save_flag = false;
      }
      else if (!digitalRead(save_button) &! save_flag)
      {
        save_flag = true;
      }

      if (digitalRead(reste_SW) && reset_flag)
      {
        reset_data();
        restore_data();
        reset_screen();
        reset_flag = false;
      }
      if (!digitalRead(reste_SW) &! reset_flag)
      {
        reset_flag = true;
      }
    }

    //stop counting
    else if ((counting_stage == 0 || counting_stage == 2) && startSW_flag)
    {
      motor_stop();
      digitalWrite(motor_led, LOW);
      start_flag = 0;
    }

    //coin is full
    if ((raw_baht[0] >= 33 * limit_count[0]))
    {
      coinFull_screen(1);
      limit_count[0]++;
    }
    else if ((raw_baht[1] >= 33 * limit_count[1]))
    {
      coinFull_screen(2);
      limit_count[1]++;
    }
    else if ((raw_baht[2] >= 33 * limit_count[2]))
    {
      coinFull_screen(5);
      limit_count[2]++;
    }
    else if ((raw_baht[3] >= 33 * limit_count[3]))
    {
      coinFull_screen(10);
      limit_count[3]++;
    }
    vTaskDelay(20 / portTICK_PERIOD_MS); //debounce
  }
}

void sensor_handle(void * pvparameter)
{
  for(;;)
  {
    if (counting_stage == 1)
    {
      for (int i=0; i<4; i++){sensor_data[i] = digitalRead(sensor_pin[i]);} //read all sensor state
      //1 baht
      Serial.println(sensor_data[2]);
      if ((sensor_data[0]) &! (sensor_flag[0]))
      {
        raw_baht[0] ++;
        lcd.setCursor(0,0);
        lcd.print("1:" + String(raw_baht[0]) + " ");
        sensor_flag[0] = true;
        sensor_filter[0] = millis();
      }
      else if ((!sensor_data[0]) && (sensor_flag[0]))
      {
        if ((millis() - sensor_filter[0]) > 70)
        {
          sensor_flag[0] = false;
        }
      }

      //2 baht
      if ((sensor_data[1]) &! (sensor_flag[1]))
      {
        raw_baht[1] ++;
        sensor_filter[1] = millis();
        lcd.setCursor(10,0);
        lcd.print("2:" + String(raw_baht[1]) + " ");
        sensor_flag[1] = true;
      }
      else if ((!sensor_data[1]) && (sensor_flag[1]))
      {
        if ((millis() - sensor_filter[1]) > 150)
        {
          sensor_flag[1] = false;
        }
      }

      //5 baht
      if ((sensor_data[2]) &! (sensor_flag[2]))
      {
        raw_baht[2] ++;
        sensor_filter[2] = millis();
        lcd.setCursor(0,1);
        lcd.print("5:" + String(raw_baht[2]) + " ");
        sensor_flag[2] = true;
      }
      else if ((!sensor_data[2]) && (sensor_flag[2]))
      {
        if ((millis() - sensor_filter[2]) > 70)
        {
          sensor_flag[2] = false;
        }
      }

      //10 baht
      if ((sensor_data[3]) &! (sensor_flag[3]))
      {
        raw_baht[3] ++;
        sensor_filter[3] = millis();
        lcd.setCursor(10,1);
        lcd.print("10:" + String(raw_baht[3]) + " ");
        sensor_flag[3] = true;
      }
      else if ((!sensor_data[3]) && (sensor_flag[3]))
      {
        if ((millis() - sensor_filter[3]) > 70)
        {
          sensor_flag[3] = false;
        }
      }

      total = raw_baht[0] + (raw_baht[1] * 2) + (raw_baht[2] * 5) + (raw_baht[3] * 10);
      if (last_total != total){
        lcd.setCursor(0, 2);
        lcd.print("Total:" + String(total) + " ");
        last_total = total;
      }

      if ((raw_baht[0] >= 9999999) || (raw_baht[1] >= 99999999) || (raw_baht[2] >= 9999999) || (raw_baht[3] >= 9999999) || (total >= 99999999999999))
      {
        lcd.setCursor(1, 0);
        lcd.print("Limit reached!!");
        motor_stop();
        counting_stage = 99;
        save_data(raw_baht[0], raw_baht[1], raw_baht[2], raw_baht[10], total);
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS); //delay for other tasks to continue
  }
}

void battery_task(void * pvparameter)
{
  for (;;)
  {
    int raw_battery = analogRead(battery_pin);
    battery_percentage = map(raw_battery, 0, 3313, 0, 100);
    if ((counting_stage == 1) || (counting_stage == 0))
    {
      lcd.setCursor(8, 3);
      lcd.print("Battery:" + String(battery_percentage) + "%");
    }
    if (battery_percentage <= 30){
      motor_stop();
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      lcd.setCursor(4, 1);
      lcd.print("Battery LOW");
      counting_stage = 99;
      save_data(raw_baht[0], raw_baht[1], raw_baht[2], raw_baht[10], total);
    }
    vTaskDelay(20000 / portTICK_PERIOD_MS);
  }
}

void coinFull_screen(int coin){
  motor_stop();
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  lcd.setCursor(0, 1);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(String(coin) + " baht coin is full");
  lcd.setCursor(0,1);
  lcd.print("Please clear coin");
  lcd.setCursor(0,2);
  lcd.print("Then press");
  lcd.setCursor(0,3);
  lcd.print("Start/Stop button");
  counting_stage = 2;
  startSW_flag = true;
  save_data(raw_baht[0], raw_baht[1], raw_baht[2], raw_baht[10], total);
}

void home_screen()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("1:" + String(raw_baht[0]) + " ");
  lcd.setCursor(10,0);
  lcd.print("2:" + String(raw_baht[1]) + " ");
  lcd.setCursor(0,1);
  lcd.print("5:" + String(raw_baht[2]) + " ");
  lcd.setCursor(10,1);
  lcd.print("10:" + String(raw_baht[3]) + " ");
  lcd.setCursor(0, 2);
  lcd.print("Total:" + String(total) + " ");
  lcd.setCursor(8, 3);
  lcd.print("Battery:" + String(battery_percentage) + "%");
}

void reset_screen()
{
  lcd.clear();
  lcd.setCursor(6,1);
  lcd.print("Value Is");
  lcd.setCursor(6,2);
  lcd.print("Reseted");
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  home_screen();
}

void save_screen()
{
  lcd.clear();
  lcd.setCursor(6,1);
  lcd.print("Value Is");
  lcd.setCursor(7,2);
  lcd.print("Saved");
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  home_screen();
}

void restore_data()
{
  raw_baht[0] = EEPROMReadlong(1);
  raw_baht[1] = EEPROMReadlong(10);
  raw_baht[2] = EEPROMReadlong(20);
  raw_baht[3] = EEPROMReadlong(30);
  total = EEPROMReadlong(40);

    //coin is full
  while ((raw_baht[0] >= 33 * limit_count[0]))
  {
    limit_count[0]++;
  }
  while ((raw_baht[1] >= 33 * limit_count[1]))
  {
    limit_count[1]++;
  }
  while ((raw_baht[2] >= 33 * limit_count[2]))
  {
    limit_count[2]++;
  }
  while ((raw_baht[3] >= 33 * limit_count[3]))
  {
    limit_count[3]++;
  }
}

void task_setup()
{
  xTaskCreatePinnedToCore(button_handle,"button_handle", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(motor_handle,"motor_handle", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(sensor_handle,"sensor_handle", 10000, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(battery_task,"battery_task", 2048, NULL, 1, NULL, 1);
}

void pin_setup()
{
  pinMode(motor_led, OUTPUT);
  pinMode(start_SW, INPUT_PULLUP);
  pinMode(save_button, INPUT_PULLUP);
  pinMode(reste_SW, INPUT_PULLUP);
  pinMode(battery_pin, INPUT);
  for (int i=0; i<4; i++){pinMode(sensor_pin[i], INPUT_PULLUP);}
}

void lcd_setup()
{
  lcd.init(I2C_SDA, I2C_SCL); // initialize the lcd to use user defined I2C pins
	lcd.backlight();
	lcd.setCursor(6,1);
	lcd.print("Automatic");
	lcd.setCursor(4,2);
	lcd.print("Coin Counter");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  home_screen();
}