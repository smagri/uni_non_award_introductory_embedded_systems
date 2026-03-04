#define F_CPU 16000000UL
//#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>

// Otherwise the CPU moves to fast  and we don't see any LEDs turn
// on or off.
#define delay_time_ms 1000

// Implements an 8 bit LED chaser using ports on the arduino uno rev3.

void virtual_port_to_physical_port(char vport){

    // Turn current led off before next led is put on.
    PORTB = 0x00; 
    PORTC = 0x00;

    // Create a  vport, virtual port,  of 8  bits. And assign  them to
    // real port values.

    // Anything anded  with a  value leaves register  as is,  vport in
    // this case, wrt 0s and 1s.
    PORTB = (0x3F & vport);

    // But port  C is really  after port b in  the circuit so  we must
    // shift it 6 to the right.
    PORTC = (vport >> 6);

    
}



int main(void)
{
    //init();
    //Serial.begin(9600);

    // Set the Data Direction Registers  for ports, and select outputs
    // for the  pins we are  goint to use to turn LEDs on or off.    
    DDRB = 0x3F;
    DDRC = 0x03;
    PORTB = 0x00;
    PORTC = 0x00;

    unsigned char vport = 0x00;

    // True when moving forwards and false for moving backwards.
    bool turn_on_leds_from_left2right = 0;

    // int8_t to let led_on_index go -ve for loops to work.
    int8_t led_on_index = 6;
    
    while (1) {

        if (turn_on_leds_from_left2right){
            // Turn on LEDs left2right, or forwards.

            //Serial.print("dbg: left to right led_left_to_right_on=");
            //Serial.println(led_on_index);
            
            // Turn on portB LEDs left2right, LSB to MSB
            

            // We start  with 3  in the LSBs.   Then foreach  index we
            // create a new word and give  it to vport based on moving
            // that 3 LSBs using the next index.  So we move from left
            // to right as our index goes from 0->6.  Remember that my
            // LEDs are setup from LSB to MSB, see arduino pinouts for
            // the ports.
            vport |= 3 << led_on_index;
            virtual_port_to_physical_port(vport);
            led_on_index++;
            if (led_on_index > 6){

                // Both port's have had all LEDS moved left2right.  So
                // setup  type name(args)   and  varibles to
                // the correct  state for  moving right to  left.  And
                // for the next time LEDs need to go from left2right.

                // Serial.println("dbg:Test to set Going Backwards");

                // Time to go backwards, or right to left.
                turn_on_leds_from_left2right = 0;
                led_on_index = 6;
            }
        } // end:: if (turn_on_leds_from_left2right)
        else if (!turn_on_leds_from_left2right){
            // Turn on LEDs right to left, or backwards.

            //Serial.print("dbg: right to left led_right_to_left_on=");
            //Serial.println(led_on_index);
            //Serial.println("dbg: Going Backwards");

            // We start  with 3  in the LSBs.   Then foreach  index we
            // create a new word and give  it to vport based on moving
            // that  3 LSBs  using the  next index.   So we  move from
            // right to  left as our  index goes from  6->0.  Remember
            // that my  LEDs are  setup from LSB  to MSB,  see arduino
            // pinout for the ports.
            vport |= (3 << led_on_index);
            virtual_port_to_physical_port(vport);
            led_on_index--;

            if (led_on_index < 0){

                // All LEDs have been  moved right to left, backwards.
                // So now setup for going left to right, forwards.
                
                turn_on_leds_from_left2right = 1;
                led_on_index = 0;
            }
            
        }// end:: if (!turn_on_leds_from_left2right)

        
        // Show  the high/on  output of  the LEDs  for a  little while
        // before  turning them  off.  If  we didn't  do this  the CPU
        // ticks over so fast we wouldn't see anything.
        _delay_ms(delay_time_ms);

        vport = 0x00;
        
        
    } // end: while(1)

} // end: main()
