/*
 * NSR CDI 測試機 NSR CDI Tester
 * Author: 羽山秋人 (https://3wa.tw)
 * Author: 整理 by ChatGPT
 * MCU: ATTiny85 (Internal 16 MHz)
 * TM1637: CLK=PB0(5), DIO=PB1(6)
 * VR input: PB3 (A3)
 * PWM output: PB4 (3)
 */

#include <Arduino.h>
#include <TM1637Display.h>

#define CLK PB0
#define DIO PB1
#define VR_PIN PB3
#define OUT_PIN PB4

TM1637Display display(CLK, DIO);

const int rpm_steps[] = {
  0,500,1000,1500,2000,2500,3000,3500,4000,4500,5000,
  5500,6000,6500,7000,7500,8000,8500,9000,9500,10000,
  10500,11000,11500,12000
};
const int rpm_steps_count = sizeof(rpm_steps) / sizeof(rpm_steps[0]);

const unsigned long VR_READ_INTERVAL_MS = 300;
const unsigned long DISPLAY_UPDATE_INTERVAL_MS = 500;
const int RPM_DISPLAY_THRESHOLD = 50;

unsigned long lastVRReadTime = 0;
unsigned long lastDisplayUpdate = 0;

volatile int rpm = 0;

unsigned long periodUs = 0;
unsigned long highTimeUs = 0;
unsigned long lowTimeUs = 0;

bool isHigh = false;
unsigned long lastToggleTime = 0;

void setup() {
  pinMode(OUT_PIN, OUTPUT);
  pinMode(VR_PIN, INPUT);

  display.setBrightness(0x0f);
  display.clear();

  digitalWrite(OUT_PIN, LOW);
  isHigh = false;
  lastToggleTime = micros();
}

int readAverageVR(int samples = 8) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(VR_PIN);
    delay(1);  // 少量延遲使 ADC 穩定
  }
  return sum / samples;
}

void updateRPM(int newRPM) {
  if (newRPM <= 0) {
    rpm = 0;
    periodUs = 0;
    highTimeUs = 0;
    lowTimeUs = 0;
    digitalWrite(OUT_PIN, LOW);
    isHigh = false;
    return;
  }
  rpm = newRPM;
  float freq = rpm / 60.0f;  // 轉成Hz
  periodUs = (unsigned long)(1e6 / freq);
  // 60 =  6度 凸台時間
  // 48 = 7.5度 凸台時間
  // 24 = 15度 凸台時間
  // 12 = 30度 凸台時間
  //  6 = 60度 凸台時間
  //  3 = 120度 凸台時間
  //  2 = 180度 凸台時間
  lowTimeUs = periodUs / 12;  
  highTimeUs = periodUs - lowTimeUs;
}

void handleTriggerOutput() {
  if (rpm <= 0) {
    digitalWrite(OUT_PIN, LOW);
    isHigh = false;
    return;
  }

  unsigned long now = micros();
  unsigned long elapsed = now - lastToggleTime;

  if (isHigh) {
    if (elapsed >= highTimeUs) {
      digitalWrite(OUT_PIN, LOW);
      isHigh = false;
      lastToggleTime = now;
    }
  } else {
    if (elapsed >= lowTimeUs) {
      digitalWrite(OUT_PIN, HIGH);
      isHigh = true;
      lastToggleTime = now;
    }
  }
}

void updateDisplay() {
  unsigned long now = millis();
  static int lastDisplayedRPM = -1;
  if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL_MS) {
    if (abs(rpm - lastDisplayedRPM) >= RPM_DISPLAY_THRESHOLD || lastDisplayedRPM == -1) {
      display.showNumberDec(rpm);
      lastDisplayedRPM = rpm;
    }
    lastDisplayUpdate = now;
  }
}

void loop() {
  unsigned long now = millis();

  // 定時讀 VR 並更新 rpm
  if (now - lastVRReadTime >= VR_READ_INTERVAL_MS) {
    int vrVal = readAverageVR();
    int idx = map(vrVal, 0, 1023, 0, rpm_steps_count - 1);
    idx = constrain(idx, 0, rpm_steps_count - 1);
    updateRPM(rpm_steps[idx]);
    lastVRReadTime = now;
  }

  handleTriggerOutput();

  updateDisplay();
}
