
//tmc2209 config
#define stepper_current             700 //mA
#define STALL_PIN                   26 // ESP diag pin is attached to
#define SERIAL_PORT                 Serial2 // HardwareSerial port for ESP
#define DRIVER_ADDRESS              0b00 // TMC2209 Driver address according to MS1 and MS2
#define R_SENSE                     0.11f // Match to your driver
#define EN_PIN                      14 // Enable
#define STEP_PIN                    27 // Step
// higher value of STALL_VALUE increases stall sensitivity
// diag pin pulsed HIGH when SG_RESULT falls below 2*STALL_VALUE
// must be in StealthChop Mode for stallguard to work
// Value of TCOOLTHRS must be greater than TSTEP & TPWMTHRS
#define STALL_VALUE                 50// [0..255]

//motor speed config
#define forware_speed               500
#define after_jamm_forward_speed    300
#define reverse_speed               500
#define jamm_dir_delay              500

//buttons and led pins
#define start_SW                    12
#define save_button                 13
#define motor_led                   19

//coin sensors pins
#define baht1_pin                   25
#define baht2_pin                   33
#define baht5_pin                   32
#define baht10_pin                  35

//lcd config
#define I2C_SDA                     21
#define I2C_SCL                     22

