#define F_CPU 16000000UL
//#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>


// Implements an 8 bit LED chaser using ports on the arduino uno rev3.

int main(void)
{
    //init();
    //Serial.begin(9600);

    // Set the Data Direction Registers  for ports, and select outputs
    // for the  pins we are  goint to use to turn LEDs on or off.    
    DDRB = 0x3F;
    DDRC = 0x03;

    unsigned char led_left_to_right_portB_on = 0;
    unsigned char led_left_to_right_portC_on = 0;
    unsigned char led_right_to_left_portB_on = 0;
    unsigned char led_right_to_left_portC_on = 0;
    unsigned char leds_right_to_left_portC = 0;
    unsigned char leds_right_to_left_portB = 1;

    // Otherwise the CPU moves to fast  and we don't see any LEDs turn
    // on or off.
    uint16_t delay_time_ms = 500;

    // True when moving forwards and false for moving backwards.
    bool turn_on_leds_from_left2right = 0;

    
    while (1) {

        
        if (turn_on_leds_from_left2right){
            // Turn on LEDs left2right, or forwards.

            // Turn on portB LEDs left2right.
            if (led_left_to_right_portB_on < 6){
                PORTB |= 1 << led_left_to_right_portB_on;
                led_left_to_right_portB_on++;
            }
            // Turn Turn on portC LEDs left2right
            else if ((led_left_to_right_portB_on == 6) && (led_left_to_right_portC_on < 2)){
                // PORTC |= 1 << led_left_to_right_portC_on;
                // PORTC |= 1 << led_left_to_right_portC_on + 1;
                // led_left_to_right_portC_on++;
                    //Serial.println("dbg: Going Backwards");
            }
            else{
                // Both port's have had all LEDS moved left2right.  So
                // setup  type name(args)  const;lags and  varibles to
                // the correct  state for  moving right to  left.  And
                // for the next time LEDs need to go from left2right.

                // Serial.println("dbg:Test to set Going Backwards");

                // Time to go backwards, or right to left.
                turn_on_leds_from_left2right = 0; 

                leds_right_to_left_portC = 1;
                leds_right_to_left_portB = 0;

            
                led_left_to_right_portB_on = 0;
                led_left_to_right_portC_on = 0;
            }
            

        } // end:: if (turn_on_leds_from_left2right)




        if (!turn_on_leds_from_left2right){
            // Turn on LEDs right to left, or backwards

            if (leds_right_to_left_portC){
                // Turn on the LEDs from right to left.
                // PORTC |= (0x02 >> led_right_to_left_portC_on);
                // PORTC |= (0x02 >> led_right_to_left_portC_on + 1);
                // if (led_right_to_left_portC_on < 1){
                //     led_right_to_left_portC_on++;
                // }
                // else{
                //     leds_right_to_left_portC = 0;
                //     leds_right_to_left_portB = 1;
                // }
            }
            else if (leds_right_to_left_portB){
                // Turn on the LEDs of portB from right to left
                
                //Serial.println("dbg: Going Backwards");
                PORTB |= (0x20 >> led_right_to_left_portB_on);
                PORTB |= (0x20 >> (led_right_to_left_portB_on + 1));
                led_right_to_left_portB_on++;
                if (led_right_to_left_portB_on == 6){
                    //Serial.println("dbg: Going Forwards Again");
                    // Time to go from left2right, or forwards.
                    turn_on_leds_from_left2right = 0;

                    led_right_to_left_portC_on = 0;
                    led_right_to_left_portB_on = 0;
                }
            }
            
        }// end:: if (!turn_on_leds_from_left2right)

        
        // Show  the high/on  output of  the LEDs  for a  little while
        // before  turning them  off.  If  we didn't  do this  the CPU
        // ticks over so fast we wouldn't see anything.
        _delay_ms(delay_time_ms);

        // Turn current led off before next led is put on.
        PORTB = 0x00; 
        PORTC = 0x00;

        
    } // end: while(1)

} // end: main()
