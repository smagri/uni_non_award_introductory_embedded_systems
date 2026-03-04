#define F_CPU 16000000UL

//#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>

// Able to change the delay time globally with just one change here.
#define delay_time_ms 1000


void pattern1(){
    // Every second bit of portB left to right
    for (uint8_t i=0; i<5; i++) {
        PORTB |= (1 << i);
        _delay_ms(delay_time_ms);
        i++;
    }

    // So the next pattern comes out cleanly.  On the Arduino
    // port registers hold their value until you change them.
    PORTB = 0x00;
}

void pattern2(){
    //Serial.println("dbg: in pattern2()");
    // Every second bit of portB right to left

     // Note the int8_t, signed so we can  handle -2 as index of i and
     // use it to terminate the loop.
    for (int8_t i=5; i>0; i--) {
        //Serial.println("dbg: in for loop pattern2()");
        //Serial.print("i=");
        //Serial.println(i);
        PORTB |= (1 << i);
        _delay_ms(delay_time_ms);
        i--;
    }

    PORTB = 0x00;
}

void pattern3(){
    // All LEDs on then all LEDs off.
    PORTB |= 0x3F;
    _delay_ms(delay_time_ms);
    PORTB &= ~(0x3F); // PORTB anded with 0, anything anded with 0 is 0
    _delay_ms(delay_time_ms);
}

void pattern4(){
    // MSB then LSB on at same time of PORTB, then cleared.
    PORTB |= 0x20;
    _delay_ms(delay_time_ms);
    PORTB |= 0x01;
    _delay_ms(delay_time_ms);
    PORTB &= ~(0x3F);
    
}


void pattern5(){
    // Toggle portC LEDs on and off 3 times
    
    PORTC &= ~(0x3F);
     
    for (uint8_t i=0; i<3; i++) {
        // Toggle PC0
        PORTC |= (1 << PC0); // PC0 = 0 from header files
        _delay_ms(delay_time_ms/2); // play with global delay
        PORTC &= ~(1 << PC0); // PORTC = PORTC & 0, anything anded with 0 is 0.
                  
        // Toggle PC1
        PORTC |= (1 << PC1);  // PC` = 1 from header files
        _delay_ms(delay_time_ms/2);
        PORTC &= ~(1 << PC1);
    }

    // So the next pattern comes out cleanly.  On the Arduino
    // port registers hold their value until you change them.
    PORTC &= ~(0x3F);
}


int main( )
{

    // Implements diverse  8 bit LED chaser using ports on the arduino uno rev3.
    
    DDRB = 0x3F;
    DDRC = 0x03;

    // So the input pins are not set  to high due to the arduino input
    // pins being tied high  by default, even if in DDRx  it is set to
    // an inpur.  We explicity make them start as active LOW.
    PORTB = 0x00;
    PORTC = 0x00;

    //init();
    //Serial.begin(9600);


    while (1) {

        pattern3();  // All LEDs on then all LEDs off.
        pattern4();  // MSB then LSB on at same time of PORTB, then cleared.
        pattern1();  // Every second bit of portB left to right
        pattern5();  // Toggle portC LEDs on and off 3 times
        pattern2();  // Every second bit of portB right to left
    }


}
