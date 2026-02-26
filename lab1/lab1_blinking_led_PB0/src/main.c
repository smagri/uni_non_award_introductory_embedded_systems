#include <avr/io.h>
#include <util/delay.h>


// This program will blink the LED connected to pin 0 of PORTB on and
// off every second.

int main(void){
  //DDRB = 0b00100000;
  DDRB = 0b00000001;

// LED on Pin5 should Flash every 500ms.  As Pin5 is configured as an output.
//  DDRB = 0xFF;

// Pin5 is  now configured as  an input, only  Pin4 is an  output.  So
// outputting values to Pin5 has no affect on the LED.
// DDRB = 0x10;

// LED on Pin5 should Flash every 500ms.  As Pin5 is configured as an output.
//  DDRB = 0x20;
  while (1) {
    PORTB = 0b00000001;
    //PINB = 0b00100000;
    //PORTC = 0b00100000;
    _delay_ms(500);
    PORTB = 0b00000000;
    //PINB = 0b00000000;
    //PORTC = 0b00000000;
    _delay_ms(500);
  } // END: while(1)
} // END:: main()
