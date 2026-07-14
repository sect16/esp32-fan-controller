#include <Arduino.h>
#include <esp32-hal.h>
#include <esp32-hal-ledc.h>
#include "config.h"
#include "log.h"
#include "mqtt.h"
#include "tft.h"
#include <Preferences.h>

int pwm1 = 0;
int pwm2 = 0;
int pwmInit1 = 0;
int pwmInit2 = 0;
int pwmManual1 = 0;
int pwmManual2 = 0;

void setPWMvalue(int channel, int pwm);

Preferences preferences;

// https://randomnerdtutorials.com/esp32-pwm-arduino-ide/
void initPWMfan(void)
{
  ledcSetup(PWMCHANNEL1, PWMFREQ, PWMRESOLUTION);
  ledcSetup(PWMCHANNEL2, PWMFREQ, PWMRESOLUTION);
  ledcAttachPin(PWMPIN1, PWMCHANNEL1);
  ledcAttachPin(PWMPIN2, PWMCHANNEL2);

  preferences.begin("hiveMon", false);
  pwmInit1 = preferences.getInt("pwmI1", 256);
  pwmInit2 = preferences.getInt("pwmI2", 256);
  pwmManual1 = preferences.getInt("pwmM1", 256); // Use 255 instead of 256 for 8-bit resolution max
  pwmManual2 = preferences.getInt("pwmM2", 256);
  preferences.end();

  Log.printf("Reading preferences and setting values:\r\nPWM1 = %d PWM2 = %d\r\n", pwmInit1, pwmInit2);
  
  setPWMvalue(PWMCHANNEL1, pwmInit1);
  setPWMvalue(PWMCHANNEL2, pwmInit2);
  
  Log.printf("  Fan PWM successfully initialized.\r\n");
}

void setPwmInit1(int pwm)
{
  pwm = (int)fmin(256, pwm);
  preferences.begin("hiveMon", false);
  preferences.putInt("pwmI1", pwm); // max 15 letters NameSpace
  preferences.end();
}

void setPwmInit2(int pwm)
{
  pwm = (int)fmin(256, pwm);
  preferences.begin("hiveMon", false);
  preferences.putInt("pwmI2", pwm); // max 15 letters NameSpace
  preferences.end();
}

void setManual1(int pwm)
{
  pwm = (int)fmin(256, pwm);
  preferences.begin("hiveMon", false);
  preferences.putInt("pwmM1", pwm); // max 15 letters NameSpace
  preferences.end();
  pwmManual1 = pwm;
}

void setManual2(int pwm)
{
  pwm = (int)fmin(256, pwm);
  preferences.begin("hiveMon", false);
  preferences.putInt("pwmM2", pwm); // max 15 letters NameSpace
  preferences.end();
  pwmManual2 = pwm;
}

void setPWMvalue(int channel, int pwm)
{
  pwm = (int)fmin(256, pwm);
  ledcWrite(channel, pwm);
  if (channel == PWMCHANNEL1)
    pwm1 = pwm;
  else if (channel == PWMCHANNEL2)
    pwm2 = pwm;
  else Log.printf("Invalid PWM channel specified.\r\n");
}

int getPWMvalue(int channel)
{
  return ledcRead(channel);
}