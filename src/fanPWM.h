extern int pwm1;
extern int pwm2;
extern int pwmInit1;
extern int pwmInit2;
extern int pwmManual1;
extern int pwmManual2;

// Function to initialize the PWM fan
void initPWMfan();
void setPwmInit1(int pwm);
void setPwmInit2(int pwm);
void setManual1(int pwm);
void setManual2(int pwm);

// Function to set the PWM value
// Input: an integer representing the PWM value, a boolean to force the update
void setPWMvalue(int channel, int pwm);
int getPWMvalue(int channel);