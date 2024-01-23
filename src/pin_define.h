
//tmc2209 config
#define stepper_current             900 //mA
#define STALL_PIN                   19 // ESP diag pin is attached to
#define SERIAL_PORT                 Serial2 // HardwareSerial port for ESP
#define DRIVER_ADDRESS              0b00 // TMC2209 Driver address according to MS1 and MS2
#define R_SENSE                     0.11f // Match to your driver
#define EN_PIN                      18 // Enable
#define STEP_PIN                    5 // Step
// higher value of STALL_VALUE increases stall sensitivity
// diag pin pulsed HIGH when SG_RESULT falls below 2*STALL_VALUE
// must be in StealthChop Mode for stallguard to work
// Value of TCOOLTHRS must be greater than TSTEP & TPWMTHRS
#define STALL_VALUE                 10// [0..255]

//motor speed config
#define forware_speed               400
#define after_jamm_forward_speed    300
#define reverse_speed               200
#define jamm_dir_delay              50

//buttons and led pins
#define start_SW                    12
#define save_button                 13
#define reste_SW                    27
#define motor_led                   14

//coin sensors pins
#define baht1_pin                   26
#define baht2_pin                   32
#define baht5_pin                   33
#define baht10_pin                  25

//lcd config
#define I2C_SDA                     21
#define I2C_SCL                     22

//bettery monitor pin
#define battery_pin                 34
