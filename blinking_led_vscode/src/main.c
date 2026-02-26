#include <avr/io.h>
#include <util/delay.h>


// This program will blink the LED connected to pin 5 of PORTB on and
// off every second.

// This is a comment added while using emacs ver 30, does it show up
// in vscode - YES

// This is another comment, does it appear in vscode - yes it does as
// long as you save the file in emacs

// Just making sure that vscode is picking up my new comment

int main(void){
  DDRB = 0b00100000;
  while (1) {
    PORTB = 0b00100000;
    _delay_ms(5000);
    PORTB = 0b00000000;
    _delay_ms(5000);
  } // END: while(1)
} // END:: main()
