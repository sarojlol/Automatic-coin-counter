#include <Arduino.h>
#include <stepper.h>
#include <TMCStepper.h>
#include <pin_define.h>

TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);

int stepTime = 160;
bool stalled = false;

void stallInterruptX(){ // flag set when motor stalls
    stalled = true;
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
    // TCOOLTHRS needs to be set for stallgaurd to work //
    driver.TCOOLTHRS(0xFFFFF); // 20bit max
    driver.SGTHRS(STALL_VALUE);
    attachInterrupt(digitalPinToInterrupt(STALL_PIN), stallInterruptX, RISING);
    digitalWrite(EN_PIN, HIGH); // Enable driver in hardware
    ledcAttachPin(STEP_PIN, 0);
    ledcSetup(0, 500, 8);
}

int step_delay;
bool step_dir;
bool motorStop = true;
unsigned long stall_delay;
bool filtered_stalled;

void motor(int speed, bool direction){
    step_dir = direction;
    ledcChangeFrequency(0, speed, 8);
    stall_delay = millis();
    motorStop = false;
}

void motor_stop(){
    filtered_stalled = false;
    stalled = false;
    ledcWrite(0, 0);
    digitalWrite(EN_PIN, HIGH);
    motorStop = true;
}

void motor_run(){
    driver.shaft(step_dir);
    if (!motorStop)
    {
        digitalWrite(EN_PIN, LOW);
        ledcWrite(0, 128);
        if ((millis() - stall_delay) > 100){
            if (stalled){
                filtered_stalled = true;
            }
        }
        else 
        {
            filtered_stalled = false;
            stalled = false;
        }
    }
}

bool motor_stalled(){
    return filtered_stalled;
}