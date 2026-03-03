#define F_CPU 16000000UL
#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>


// Implements an 8 bit LED chaser using ports on the arduino uno rev3.

void physical_ports_to_virtual_port(char vport){

    // Turn current led off before next led is put on.
    PORTB = 0x00; 
    PORTC = 0x00;

    PORTC = (vport >> 6);
    PORTB = (0x3F & vport);
    
}



int main(void)
{
    init();
    Serial.begin(9600);

    // Set the Data Direction Registers  for ports, and select outputs
    // for the  pins we are  goint to use to turn LEDs on or off.    
    DDRB = 0x3F;
    DDRC = 0x03;
    PORTB = 0x00;
    PORTC = 0x00;

    unsigned char vport = 0x00;

    unsigned char led_left_to_right_on = 0;
    unsigned char led_right_to_left_on = 0;

    // Otherwise the CPU moves to fast  and we don't see any LEDs turn
    // on or off.
    uint16_t delay_time_ms = 500;

    // True when moving forwards and false for moving backwards.
    bool turn_on_leds_from_left2right = 1;
    bool hold_next = 0;

    
    while (1) {

        if (hold_next){
            hold_next = 0;
        }
        else if (turn_on_leds_from_left2right){
            // Turn on LEDs left2right, or forwards.
            Serial.print("dbg: left to right led_left_to_right_on=");
            Serial.println(led_left_to_right_on);
            // Turn on portB LEDs left2right.
            vport |= 3 << led_left_to_right_on;
            physical_ports_to_virtual_port(vport);
            if (led_left_to_right_on == 6){

                // Both port's have had all LEDS moved left2right.  So
                // setup  type name(args)  const;lags and  varibles to
                // the correct  state for  moving right to  left.  And
                // for the next time LEDs need to go from left2right.

                // Serial.println("dbg:Test to set Going Backwards");

                // Time to go backwards, or right to left.
                turn_on_leds_from_left2right = 0; 

                led_right_to_left_on = 6;
            
                led_left_to_right_on = 0;

            }
            led_left_to_right_on++;


        } // end:: if (turn_on_leds_from_left2right)
        else if (!turn_on_leds_from_left2right){
            // Turn on LEDs right to left, or backwards
            Serial.print("dbg: right to left led_right_to_left_on=");
            Serial.println(led_right_to_left_on);
                

            // Turn on the LEDs of portB from right to left
                
            //Serial.println("dbg: Going Backwards");
            vport |= (3 << led_right_to_left_on);
            physical_ports_to_virtual_port(vport);

            if (led_right_to_left_on == 0){
                turn_on_leds_from_left2right = 1;
                hold_next = 1;
            }
            led_right_to_left_on--;

            
        }// end:: if (!turn_on_leds_from_left2right)

        
        // Show  the high/on  output of  the LEDs  for a  little while
        // before  turning them  off.  If  we didn't  do this  the CPU
        // ticks over so fast we wouldn't see anything.
        _delay_ms(delay_time_ms);

        vport = 0x00;
        
        
    } // end: while(1)

} // end: main()
