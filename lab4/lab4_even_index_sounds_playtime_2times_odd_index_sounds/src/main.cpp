#define F_CPU 16000000UL

//#include <Arduino.h>  // For debugging.
#include <avr/io.h>
#include <util/delay.h>

// passive buzzer pin, portB pin 0 PINB0, when you follow the header
// files it is PB0 pin which just equates to 0.
#define passive_buzzer PINB0
#define active_buzzer PINB1 // active buzzer pin, portB pin 1



// put function declarations here:
void playSound(float frequency, float duty_cycle, unsigned long play_time);




int main(void){
    // For debugging
    //init();
    //Serial.begin(9600);

    DDRB = 0xFF; // All port B pins set to output.
    
    // Frequency's are  an octave starting  from C5.
    float frequency[] = {523.25, 587.33, 659.26, 698.46, 783.99, 880.00, 987.77};

    // Frequency's are an octave starting from C4 aka middle C.
    //float frequency[]={261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88};
    float *ptr_frequency = &frequency[0];

    // Duty  cycle is  the  ratio of  PWM time_signal_high/period.   The
    // bigger the duty cycle the louder the buzzer.
    //
    // I like to think of the duty cycle as a percentage.  Thus 0.1 duty
    // cycle means (0.1 *  100) %, or 10%.  Thus the  time the signal is
    // high is 10% of the period.
    float duty_cycle = 0.1; // 1/10 of the period of the sinal is high

    // There are 1e6us in a second. `That is 1*10^6us in a second. 
    // unsigned long play_time = 10*1e6; // play for 10 seconds
    // unsigned long play_time = 10*1e6; // play for 10 seconds
    // unsigned long play_time = 2*1e6; // play for 2 seconds
    // unsigned long play_time = 1*1e6; // play for 1 seconds

    // When the delay is set to 10 seconds we are really getting the time sum of:
    // _delay_us1()
    // loop increment
    // compare and branch
    // However, at shorter durations it is less obvious.
    // unsigned long play_time = 10*1e6; // play for 0.5 seconds

    // Later the microseconds are removed when deviding by the period in
    // microseconds

//   unsigned long play_time = 0.5*1e6; // play for 0.5 seconds each frequency.
    unsigned long play_time = 0.5*1e6; // play for 2 seconds
    // play for 0.5 seconds each odd index frequency.
    unsigned long play_time_odd = play_time;

    // play for 1 seconds each even index frequency.
    unsigned long play_time_even = play_time_odd*2;

    
    // Play frequencys forever.
    while (1){

        // i  is  the index  of  the  array  of frequencys,  Play  all
        // frequencys in octave from C4, middle C, C major.
        for (unsigned int i=0; i<sizeof(frequency)/sizeof(float); i++){

            if (i%2 == 0){
                // Even indexes

                // Double  the  playtime  for even  indexed  frequency
                // array members.
                play_time = play_time_even;
            }
            else{
                // Odd indexes

                // Leave the playtime as it is.
                play_time = play_time_odd;
            }

            playSound(*(ptr_frequency+i), duty_cycle, play_time);            
        }

    } // end: while
    

    return 0; 
} // end: main"()"



// Put function definitions here:


void playSound(float frequency, float duty_cycle, unsigned long play_time){

    float period_us = (1/frequency)*1e6; // Period in microseconds.
    unsigned int time_signal_high_us = (duty_cycle * period_us); // in us
    unsigned int time_signal_low_us = (period_us - time_signal_high_us); //in us

    // Number of times our for loop needs to excute.
    unsigned long cycles_for_playtime; 
                
    // cycles  is how  many  periods  fit into  the  whole playing  time
    // required.  As  one  invocation  of  our for  loop  below  is  one
    // period. in length.
    cycles_for_playtime = ( (unsigned long)(play_time/period_us) );

    for (unsigned long j=0; j<cycles_for_playtime; j++){

        // Play for one Period forech high-low combinations

        ////////////////////////////////////////////////////////////////////////
        //  The actual  delay per  iteration is  slightly more  than 1
        //  µs,(ie _delay_us(1))  because the CPU also  executes: Loop
        //  increment,  comparison, branch.   So  the tone  will be  a
        //  little lower in frequency than expected, as the period per
        //  frequency is a little longer.
        ////////////////////////////////////////////////////////////////////////
        
        PORTB |= (1<<passive_buzzer); // Set output of portB pin0 to high
        for (unsigned int i=0; i<time_signal_high_us; i++){
            _delay_us(1);
        }

        PORTB &= ~(1<<passive_buzzer); // Set output of portB pin0 to low
        for (unsigned int i=0; i<time_signal_low_us; i++){
            _delay_us(1);
        }
    }
    
} // end: playSound()
