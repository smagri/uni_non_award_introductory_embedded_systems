#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/boot.h>
#include <arv/timers.h>
#include <iom328p.h>

// This program will blink the LED connected to pin 5 of PORTB on and
// off every second.

// This is a comment added while using emacs ver 30, does it show up
// in vscode - YES

int main(void){
  DDRB = 0b00100000;
  while (1) {
    PORTB = 0b00100000;
    _delay_ms(1000);
    PORTB = 0b00000000;
    _delay_ms(1000);

  } // END: while(1)

} // END:: main()

