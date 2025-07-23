#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>

// PORTB |= (1 << PB0); // HIGH
// PORTB &= ~(1 << PB1); // LOW

int main(void) {
  DDRB |= (1 << PB1) | (1 << PB0); // 設定 PB0, PB1 為輸出

  uint16_t counter = 0;

  while (1) {
    // 正半週：PB0 高
    PORTB |= (1 << PB0);
    PORTB &= ~(1 << PB1);
    _delay_us(12);

    // 死區
    PORTB |= (1 << PB0);
    PORTB |= (1 << PB1);
    _delay_us(2);

    // 負半週：PB1 高    
    PORTB &= ~(1 << PB0);
    PORTB |= (1 << PB1);
    _delay_us(4);

    // 死區
    PORTB |= (1 << PB0);
    PORTB |= (1 << PB1);
    _delay_us(2);

    // 每跑一組為 30μs → 約 33.3kHz
    counter++;
    // 10 次 * 30μs ≈ 300μs ≈ 3333hz
    // 25 次 * 30μs ≈ 750μs ≈ 600Hz
    // 55 次 * 30μs ≈ 1650μs ≈ 300Hz
    // 110 次 * 30μs ≈ 1650μs ≈ 150Hz
    // 165 次 * 30μs ≈ 1650μs ≈ 100Hz
    if (counter >= 165) { 
      // 暫停 400μs（關閉兩端）
      //PORTB |= (1 << PB0);
      //PORTB |= (1 << PB1);
      //_delay_us(400);
      counter = 0;
    }
  }
}
