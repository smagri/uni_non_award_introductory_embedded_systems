#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>


#define LED_DDR DDRB
#define LED_PORT PORTB
#define delayTime 300 // in milliseconds


// lab2_task3_led_chaser
int main(void)
{

    // Enable all  portb pins  to be  output pins as  we are  going to
    // toggle the LED's attached to them on and off.
    LED_DDR = 0xFF;

    uint8_t chase[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20};
    while (1) {

        // Chase port b pin outputs from LSB to MSB
        for (uint8_t i=0; i<6; ++i) {
            LED_PORT = chase[i];
            _delay_ms(delayTime);
         }


        // Chase port b pin outputs from MSB to LSB
        //
        // We must use a signed integer to get -1 bing a valid i value
        // in our for loop.
        for (int8_t i=5; i>=0; i--) {
            LED_PORT = chase[i];
            _delay_ms(delayTime);
         }    
    }

}
 
