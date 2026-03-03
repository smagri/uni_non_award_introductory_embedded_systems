#define F_CPU 16000000UL

//#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>


void pattern1(){
    // Every second bit of portB left to right
    for (uint8_t i=0; i<5; i++) {
        PORTB = (1 << i);
        _delay_ms(2000);
        i++;
    }
    
}

void pattern2(){

    // Every second bit of portB right to left
    for (uint8_t i=5; i>0; i--) {
        PORTB = (1 << i);
        _delay_ms(2000);
        i--;
    }

    
}

void pattern3(){
    // All LEDs on then all LEDs off.
    PORTB = 0x3F;
    _delay_ms(1000);
    PORTB = 0x00;
    _delay_ms(500);
}

void pattern4(){
    // MSB then LSB of PORTB On
    PORTB = 0x20;
    _delay_ms(1000);
    PORTB = 0x01;
    _delay_ms(1000);
    
}




int main( )
{

    // Implements diverse  8 bit LED chaser using ports on the arduino uno rev3.
    DDRB = 0x3F;
    DDRC = 0x03;

    // So the input pins are not set to high due to the arduino input pins
    // being tied high by default.
    PORTB = 0x00;
    PORTC = 0x00;


    while (1) {

        pattern3();
        pattern4();
        pattern1();
        pattern2();

    }


}
