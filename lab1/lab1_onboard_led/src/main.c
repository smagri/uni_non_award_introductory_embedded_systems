#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 16000000UL

#define ledBuiltIn PINB5 // on board LED is connected to PORTB pin 5, ie PORTB5
//#define delayTime 500 // in milliseconds
#define delayTime 1000 // in milliseconds

int main(void)
{
//  DDRB = 0xFF;
//    DDRB = 0x10;
  DDRB = 0x20;

  while (1) {
    PORTB |= 1 << ledBuiltIn;
    _delay_ms(delayTime);

    PORTB &= ~(1 << ledBuiltIn);
    _delay_ms(delayTime);
    
  }

}


