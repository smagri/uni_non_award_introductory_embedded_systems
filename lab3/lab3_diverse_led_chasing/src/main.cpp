#define F_CPU 16000000UL

//#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>

// Allows bulk change of delays with just one line of code change.
#define delay_in_ms 1000

void pattern1(){
    // Every second bit of portB left to right
    for (uint8_t i=0; i<5; i++) {
        PORTB = (1 << i);
        _delay_ms(delay_in_ms);
        i++;
    }

    // So the next pattern comes out cleanly.  On the Arduino
    // port registers hold their value until you change them.
    PORTB = 0x00;
}

void pattern2(){
    //Serial.println("dbg: in pattern2()");
    // Every second bit of portB right to left
    for (int8_t i=5; i>0; i--) {
        //Serial.println("dbg: in for loop pattern2()");
        //Serial.print("i=");
        //Serial.println(i);
        PORTB = (1 << i);
        _delay_ms(delay_in_ms);
        i--;
    }

    
}

void pattern3(){
    // All LEDs on then all LEDs off.
    PORTB = 0x3F;
    _delay_ms(delay_in_ms);
    
    PORTB = 0x00;
    _delay_ms(delay_in_ms);
}

void pattern4(){
    // MSB then LSB of PORTB On
    PORTB = 0x20;
    _delay_ms(delay_in_ms);

    PORTB = 0x01;
    _delay_ms(delay_in_ms);
    
}


void pattern5(){
    // Toggle portC LEDs on and off 3 times
    for (uint8_t i=0 ; i<3; ++i) {
        PORTC = 0x01;
        _delay_ms(delay_in_ms);
        PORTC = 0x02;
        _delay_ms(delay_in_ms);
    }

    // So the next pattern comes out cleanly.  On the Arduino
    // port registers hold their value until you change them.
    PORTC = 0x00;
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

    //init();
    //Serial.begin(9600);


    while (1) {

        pattern3();
        pattern4();
        pattern1();
        pattern5();
        pattern2();
    }


}
