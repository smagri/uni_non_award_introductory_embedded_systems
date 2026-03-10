//#include <Arduino.h> // for debugging

// From week4 lecture notes
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#define bitSet(reg, n) (reg |= 1 << n)
#define bitRead(reg, n) (reg >> n & 1)
#define bitClear(reg, n) (reg &= ~(1 << n))
#define bitInverse(reg, n) (reg ^= (1 << n))

#define led_onboard 5
#define button_pin PD3


// Button pin debounceing.  We execute a  delay of ms(amount has to be
// determined emperically  because every  button is different)  on the
// transition  of the  button  form high-low  and  low-high.  On  each
// transition  type  the onboard  led  is  toggled, high-low(OFF),  or
// low-high(ON).

int main(void)
{

    // For debugging:
    // init();
    // Serial.begin(9600);
    
    // Pin 3 on port  D, PD3 is set as an input.  Then  set it high as
    // that enables its pullup resistor on the arduino pin.
    bitClear(DDRD, button_pin);
    bitSet(PORTD, button_pin);

    // Set  pin 5  on port  B,  connected to  the onboard  LED on  the
    // arduino uno  r3.  This makes pin  5 an output.  Then  it is set
    // high.
    bitSet(DDRB, led_onboard);
    bitSet(PORTB, led_onboard);

    bool button_status_old = 1;
    bool button_status;
    
    
    while (1) {

        // What  value is  on the  input pin  PD3, where  we have  the
        // button  connected.   It is  intially  high.   Wait for  the
        // transition from  high-low to  toggle the LED  off, changing
        // from its previous state.   low-high transition turns it on,
        // opposite to its previous state.   So every edge toggles the
        // LED state.
        button_status = bitRead(PIND, button_pin);

        // Keep reading button_status with the previous command till
        // it transitions from high-low or low-high.
        if(button_status != button_status_old){

            // Button has transitioned from high-low or low-high.

            // Wait a while due to ringing signal on transition
            // high-low or low-high.
            
            // You need to tune this delay based on the unique button
            // you have.  Mechanical switches roughly bounce for
            // 5-20ms.
            _delay_ms(10);

            button_status = bitRead(PIND, button_pin);

            // If now after the wait for switch bouncing to stop on
            // the transition and the pin has really changed state.
            if (button_status != button_status_old){
                button_status_old = button_status;

                {
                    // With alternate pressing  of the button conected
                    // to  PD3  the LED  turns  on  or off,  basically
                    // opposite to the previous state.
                    bitInverse(PORTB, led_onboard);
                }
            }

        } // end: if(button_status != button_status_old){

        
    } // end: while(1)
       
    return 0;
}
