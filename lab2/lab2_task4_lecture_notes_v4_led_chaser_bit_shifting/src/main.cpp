
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

    while (1) {

        if (flag_left2right){
            if (led_portB_on < 6){
                PORTB |= 1 << led_portB_on;
                led_portB_on++;
                _delay_ms(delay_time_ms);
            }
            PORTB = 0x00; // Turn current led off before next led is put on.


            if (led_portB_on == 6){
                if (led_portC_on < 2){
                    PORTC |= 1 << led_portC_on;
                    led_portC_on++;
                    _delay_ms(delay_time_ms);
                }
            }
            PORTC = 0x00;  // Turn current led off before next led is put on.
        } // end:: if (flag_left2right)
        

        if ( (led_portB_on == 6) && (led_portC_on == 2) ){
            //Serial.println("Test to set Going Backwards");
            flag_left2right = 0; // Time to go backwardstt
            // PORTC = 0x02;
            // _delay_ms(delay_time_ms);
            // PORTC = 0x01;
            // _delay_ms(delay_time_ms);
            // PORTC = 0x00;
            for (uint8_t led_portC_on=0; led_portC_on<2; led_portC_on++){
                PORTC |= (0x02 >> led_portC_on);   // 0x02, then 0x01
                _delay_ms(delay_time_ms);
                PORTC = 0x00;
            }

            
            led_portB_on = 0;
            led_portC_on = 0;
        }
        

        if (!flag_left2right){
            //Serial.println("Going Backwards");
            PORTB |= (0x20 >> led_portB_on);
            _delay_ms(delay_time_ms);
            PORTB = 0x00;
            led_portB_on++;
            if (led_portB_on == 6){
                //Serial.println("Going Forwards Again");
                led_portB_on = 0;
                led_portC_on = 0;
                flag_left2right = 1; // Time to go forwards Again
            }
            
        } // end:: if (!flag_left2right)


    } // end: while

}
