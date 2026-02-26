#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>


#define LED_DDR DDRB
#define LED_PORT PORTB
#define LED_DDR1 DDRC
#define LED_PORT1 PORTC
#define delayTime 300 // in milliseconds


// lab2_task3_led_chaser  but done  with  Bit Shifting.  Which is  the
// cleanest way to do a LED chaser on an AVR.

// We use  8 bit  version of  integer type as  the AVR  controller has
// 8bits.

// 8bit LED chaser
int main(void)
{

    // Enable all  portb pins  to be  output pins as  we are  going to
    // toggle the LED's attached to them on and off.
    LED_DDR = 0x3F;
    LED_DDR1 = 0x03;
    LED_PORT = 0x00; // Make sure the port is cleared.
    LED_PORT1 = 0x00; // Make sure the port is cleared.

    while (1) {
        // Chase of LED's goes from LSB to MSB
        for (uint8_t i=0; i<6; ++i) {
            LED_PORT = (1 << i);
            _delay_ms(delayTime);
        }
        LED_PORT = 0x00;
        _delay_ms(delayTime);
        for (uint8_t i=0; i<2; ++i) {
            LED_PORT1 = (1 << i);
            _delay_ms(delayTime);
        }
        

        // Chase of LED's goes from MSB to LSB

        // I don't want to leave MSB as we move down to one
        // for (int8_t i=6; i>=0; i--) {
        //     LED_PORT = (1 << i );
        //     _delay_ms(delayTime);
        // }
        // for (uint8_t i=0; i<2; ++i) {
        //     LED_PORT1 = (0x80 << i);
        //     _delay_ms(delayTime);
        // }
        // for (int8_t i=1; i>=0; --i) {
        //     LED_PORT1 = (0x02 << i);
        //     _delay_ms(delayTime);
        // }
        LED_PORT1 = 0x00;
        _delay_ms(delayTime);
        LED_PORT1 = 0x01;
        _delay_ms(delayTime);
        LED_PORT1 = 0x00;
        _delay_ms(delayTime);
        // _delay_ms(delayTime);
        for (uint8_t i=0; i<7; i++) {
            LED_PORT = (0x80 >> i );
            _delay_ms(delayTime);
        }

        
    }


}
