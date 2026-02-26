#include <avr/io.h>
#include <util/delay.h>


// This program will blink the LED connected to pin 0 of PORT D on and
// off every 500ms second.

int main(void){
  //DDRB = 0b00100000;

 // Data Direction Register, sets the port for being an output(1) or
  // an input (0)
  DDRD = 0b00000001;

// LED on Pin5 should Flash every 500ms.  As Pin5 is configured as an output.
//  DDRB = 0xFF;

// Pin5 is  now configured as  an input, only  Pin4 is an  output.  So
// outputting values to Pin5 has no affect on the LED.
//
// If configured as an input with nothing connected to it Pin5 is
// high.  If it is connected to ground it is set to 0V/ground.
// DDRB = 0x10;

// LED on Pin5 should Flash every 500ms.  As Pin5 is configured as an output.
//  DDRB = 0x20;

  while (1) {
    PORTD = 0b00000001;
    //PINB = 0b00100000;
    //PORTC = 0b00100000;
    _delay_ms(500);
    PORTD = 0b00000000;
    //PINB = 0b00000000;
    //PORTC = 0b00000000;
    _delay_ms(500);
  } // END: while(1)
} // END:: main()
