#define F_CPU 16000000UL

//#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>


// Implements an 8 bit LED chaser using ports on the arduino uno rev3.

int main(void)
{
    //init();
    //Serial.begin(9600);
    
    DDRB = 0x3F;
    DDRC = 0x03;

    unsigned char led_portB_on = 0;
    unsigned char led_portC_on = 0;
    uint16_t delay_time_ms = 500;
    bool flag_left2right = 1;
    unsigned char led_right_to_left_portC_on = 0;
    unsigned char led_right_to_left_portB_on = 0;
    unsigned char leds_right_to_left_portC = 1;
    unsigned char leds_right_to_left_portB = 0;

    while (1) {

        if (flag_left2right){
            // Turn on LEDs left2right

            // Turn on portB LEDs left2right
            if (led_portB_on < 6){
                PORTB |= 1 << led_portB_on;
                led_portB_on++;
                _delay_ms(delay_time_ms);
            }
            PORTB = 0x00;

            // Turn Turn on portC LEDs
            if (led_portB_on == 6){
                if (led_portC_on < 2){
                    PORTC |= 1 << led_portC_on;
                    led_portC_on++;
                    _delay_ms(delay_time_ms);
                }     //Serial.println("Going Backwards");
            }
            //PORTC = 0x00;  // Turn current led off before next led is put on.
            PORTC = 0x00;  // Turn current led off before next led is put on.
        } // end:: if (flag_left2right)

        // Both port's have had all LEDS left2right.  So stup flags
        // and varibles to the correct state for moving right to left.
        if ( (led_portB_on == 6) && (led_portC_on == 2) ){
            //Serial.println("Test to set Going Backwards");
            flag_left2right = 0; // Time to go backwards

            leds_right_to_left_portC = 1;
            leds_right_to_left_portB = 0;

            led_right_to_left_portC_on = 0;
            led_right_to_left_portB_on = 0;
            
            led_portB_on = 0;
            led_portC_on = 0;
        }



        if (!flag_left2right){

            if (leds_right_to_left_portC){
                PORTC |= (0x02 >> led_right_to_left_portC_on);
                _delay_ms(delay_time_ms);
                if (led_right_to_left_portC_on < 1){
                    led_right_to_left_portC_on++;
                }
                else{
                    leds_right_to_left_portC = 0;
                    leds_right_to_left_portB = 1;
                    //led_right_to_left_portC_on = 0;
                }
            }
            PORTC = 0x00;


            if (leds_right_to_left_portB){
                //Serial.println("Going Backwards");
                PORTB |= (0x20 >> led_right_to_left_portB_on);
                _delay_ms(delay_time_ms);
                led_right_to_left_portB_on++;
                if (led_right_to_left_portB_on == 6){
                    //_     //Serial.println("Going Forwards Again");
                    flag_left2right = 1; // Time to go forwards Again
                }
                // PORTB = 0x00;
            }
            PORTB = 0x00;

        } // end:: if (!flag_left2right)
                

    } // end: while

}
