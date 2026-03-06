#define F_CPU 16000000UL

//#include <Arduino.h>  // For debugging.
#include <avr/io.h>
#include <util/delay.h>

#define passive_buzzer PINB0 // passive buzzer pin, portB pin 0
#define active_buzzer PINB1 // active buzzer pin, portB pin 1



// put function declarations here:
void playSound(float frequency, float duty_cycle, unsigned long play_time);




int main(void)
{
    // For debugging
    //init();
    //Serial.begin(9600);

    DDRB = 0xFF; // All port B pins set to output.
    
    // Frequency's are an octave starting from C5
    //float frequency[] = {523.25, 587.33, 659.26, 698.46, 783.99, 88.00, 987.77};
    float frequency[] = {523.25};
    float *ptr_frequency = &frequency[0];

    // Duty cycle  is the ratio of  PWM  time_signal_high/period.
    // The bigger the duty cycle the louder the buzzer.
//    float duty_cycle = 0.1*1e6; // 1/10 of a cycle/period in us.
    //float duty_cycle = 3.4028235e1; // 1/10 of a cycle/period in us.
    float duty_cycle = 0.5; // 1/10 of a cycle/period in us.

    // There are 1e6us in a second. `That is 1*10^6us in a second. 
    //int play_time = 30*1e6; // play sound in us 30s
    //int play_time = 3.4028235e38; // play sound in us 30s
    //int play_time = 1e6; // play sound in us 30s
    //long play_time = 30*1e6
    unsigned long play_time = 1e7;
    

    while (1) {

        // Play all frequencys in frequency array.
        for (unsigned char i=0; i<sizeof(frequency)/sizeof(float); i++) {

            playSound(*(ptr_frequency+i), duty_cycle, play_time);            
        }
        
    }
    


    return 0;
    
}



// Put function definitions here:


void playSound(float frequency, float duty_cycle, unsigned long play_time){

    float period_us = (1/frequency)*1e6; // Period in microseconds.
    float time_signal_high_us = (duty_cycle * period_us); // in us
    float time_signal_low_us = (period_us - time_signal_high_us); // in us
    unsigned long cycles;
    
//    for (int j=0; j<(play_time/period_us); j++){
    cycles = ( (unsigned long)(play_time/period_us) );
    for (unsigned long j=0; j<cycles; j++){
        //    for (int j=0; j<play_time; j++){

        // Play for one Period forech high-low combinations
        PORTB = PORTB | (1<<passive_buzzer); // Set output of portB pin0 to high
        for (int i=0; i<time_signal_high_us; i++) {
            _delay_us(1);
        }
        PORTB = PORTB & ~(1<<passive_buzzer); // Set output of portB pin0 to low
        for (int i=0; i<time_signal_low_us; i++){
            _delay_us(1);
        }

    }
    
}
