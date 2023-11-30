#include <Arduino.h>

void tmc_setup();
void motor(int speed, bool direction);
void motor_stop();
void motor_run();
bool motor_stalled();