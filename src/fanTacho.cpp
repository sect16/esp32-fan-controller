#include <Arduino.h>
#include <esp32-hal.h>
#include <pins_arduino.h>
#include "config.h"
#include "log.h"
#include "RunningMedian.h"

static volatile int counter_rpm[TACHOINPUTS] = {0 , 0};
int rpm[TACHOINPUTS] = {0, 0};
unsigned long millisecondsLastTachoMeasurement = 0;
const int TACHOPIN[TACHOINPUTS] = {TACHOPIN1, TACHOPIN2};
RunningMedian medianRPM1 = RunningMedian(NUMSAMPLES);
RunningMedian medianRPM2 = RunningMedian(NUMSAMPLES);
RunningMedian medianRPMs[TACHOINPUTS] = {medianRPM1, medianRPM2};

// Interrupt counting every rotation of the fan
// https://desire.giesecke.tk/index.php/2018/01/30/change-global-variables-from-isr/
void IRAM_ATTR rpm_fan1() {
  counter_rpm[0]++;
}
void IRAM_ATTR rpm_fan2() {
  counter_rpm[1]++;
}
void initTacho(void) {
  for (int i=0; i < TACHOINPUTS; i++) {
    pinMode(TACHOPIN[i], INPUT_PULLUP);
    digitalWrite(TACHOPIN[i], HIGH);
  }
  attachInterrupt(digitalPinToInterrupt(TACHOPIN[0]), rpm_fan1, FALLING);
  attachInterrupt(digitalPinToInterrupt(TACHOPIN[1]), rpm_fan2, FALLING);
  Log.printf("  Fan tacho detection sucessfully initialized.\r\n");
}

void updateTacho(void) {
  if ((unsigned long)(millis() - millisecondsLastTachoMeasurement) >= TACHOUPDATECYCLE)
  { 
    for (int i = 0; i < TACHOINPUTS; i++) {
      detachInterrupt(digitalPinToInterrupt(TACHOPIN[i]));
    }
    for (int i = 0; i < TACHOINPUTS; i++) {
      int rawRPM = counter_rpm[i] * 60 * 1000 / TACHOUPDATECYCLE / NUMBEROFINTERRUPSINONESINGLEROTATION;
      
      if (rawRPM <= MAX_RPM) {
        medianRPMs[i].add(rawRPM);
        rpm[i] = medianRPMs[i].getMedian();
      } else {
        // Changed %.0f to %d because rawRPM is an int
        Log.printf("Ignored spike on fan %d: %d RPM\r\n", i, rawRPM);
      }
      
      // Always reset the counter for the next interval
      counter_rpm[i] = 0;
    }
    
    // 3. Update the timestamp
    millisecondsLastTachoMeasurement = millis();
    
    attachInterrupt(digitalPinToInterrupt(TACHOPIN[0]), rpm_fan1, FALLING);
    attachInterrupt(digitalPinToInterrupt(TACHOPIN[1]), rpm_fan2, FALLING);
  }
}