#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>


#define LED_DDR DDRB
#define LED_PORT PORTD
#define delayTime 300 // in milliseconds

//1111 iiii


int main(void)
{

    LED_DDR = 0xFF;

    void pattern1(){
        for (uint8_t i=0; i<8; i++) {
            LED_PORT = (i<<i);
            _delay_ms(delayTime);
        }
    }


    void pattern2(){
        for (int8_t i=7; i<i>=0; i--) {
            LED_PORT (1 << i);
        }
    }

    void pattern3(){
        for (uint8_t i; i<7; i+=2) {
            LED_PORT = (1 << i) | (1 << (i+1));
            _delay_ms(delayTime);
        }
    }

    void pattern4(){
        LED_PORT = 0xb10101010;
        _delay_ms(delayTime);
        LED_PORT = 0b01010101;
        _delay_ms(delayTime);
    }
    

    //uint8_t pattern = 1;

    // while (1) {

    //     if (pattern == 1){
    //         pattern1()
    //     }
    //     else if (pattern == 2){
    //         pattern(2)
    //     }
    //     else if (pattern3 == 3){
    //         pattern3()
    //     }
    //     else{
    //         pattern4();
    //     }

    //     pattern++;
    //     if (pattern>4){
    //         pattern = 1;
    //     }

    // } // end:while
    while (1) {
        pattern1();
        pattern2();
        pattern3();
        pattern4();
    }

}

//() Alternative while loop

