#define stepper_current         700 //mA
#define STALL_PIN               27 // ESP diag pin is attached to
#define SERIAL_PORT Serial2 // HardwareSerial port for ESP
#define DRIVER_ADDRESS          0b00 // TMC2209 Driver address according to MS1 and MS2
#define R_SENSE                 0.11f // Match to your driver

#define EN_PIN                  13 // Enable
#define DIR_PIN                 32 // Direction
#define STEP_PIN                33 // Step

// higher value of STALL_VALUE increases stall sensitivity
// diag pin pulsed HIGH when SG_RESULT falls below 2*STALL_VALUE
// must be in StealthChop Mode for stallguard to work
// Value of TCOOLTHRS must be greater than TSTEP & TPWMTHRS
#define STALL_VALUE             50// [0..255]

#define start_SW                12