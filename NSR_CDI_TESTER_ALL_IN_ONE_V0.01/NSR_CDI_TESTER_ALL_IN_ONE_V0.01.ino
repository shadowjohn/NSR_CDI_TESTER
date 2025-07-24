#include <TimerOne.h>

#define PIN_PB0 8  // Arduino UNO Digital 腳5 PB0

volatile uint8_t state = 0; // 0=正半週，1=死區1，2=負半週，3=死區2
volatile uint16_t counter = 0;

void setup() {
  pinMode(PIN_PB0, OUTPUT);  

  // TimerOne 初始化，30us 週期 (約33.3kHz)
  Timer1.initialize(30); 
  Timer1.attachInterrupt(timerISR);
}

void timerISR() {
  switch (state) {
    case 0: // 正半週，PB0 高
      digitalWrite(PIN_PB0, HIGH);      
      Timer1.setPeriod(12); // 12us 下一階段
      state = 1;
      break;
    case 1: // 死區1，PB0 都高
      digitalWrite(PIN_PB0, HIGH);      
      Timer1.setPeriod(2);  // 2us 死區
      state = 2;
      break;
    case 2: // 負半週，PB0 低
      digitalWrite(PIN_PB0, LOW);      
      Timer1.setPeriod(4); // 4us
      state = 3;
      break;
    case 3: // 死區2，PB0 都高
      digitalWrite(PIN_PB0, HIGH);      
      Timer1.setPeriod(2); // 2us
      state = 0;

      // 每週期計數
      counter++;
      if (counter >= 165) { 
        // 這裡你可以做停波或延遲控制（非 ISR 裡做）
        counter = 0;
      }
      break;
  }
}

void loop() {
  // 你可以在這控制停波或其他功能
}