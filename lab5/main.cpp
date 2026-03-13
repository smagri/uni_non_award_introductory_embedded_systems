#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <Arduino.h>

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <math.h>


// These are my version of the bit manipulation functions, so you
// don't need to know the order of precedence rules.
#define bitSet(reg, n) (reg |= (1 << n))
//#define bitSet(reg, n) (reg |= 1 << n)
#define bitRead(reg, n) ((reg >> n) & 1)
#define bitCheck(reg, n) ( ((reg) >> (n)) & 1 )
//#define bitCheck(reg, n) (reg >> n & 1)
#define bitClear(reg, n) (reg &= ~(1 << n))
#define bitInverse(reg, n) (reg ^= (1 << n))

// PB0 equates to number 0 as seen on vscode when you press control
// and mouse click on the variable, eventually it leads you to this
// definition. ie PB0->PORTB->0.
#define pin_trigger PB0
#define pin_echo PB1
#define pin_passive_buzzer PC0


// uint16_t Dmin = 10; // minimum feasible distance for ultrasonic sensor, mm.
// uint16_t Dmax = 200;// maximum feasible distance for ultrasonic sensor, mm.
// uint16_t Delay_min = 1; // maximum feasible buzzer delay, ms.
// uint16_t Delay_max = 20; // maximum feasible buzzer delay, ms.




void playSound(float frequency, float duty_cycle, unsigned long playtime_us);
//float linear_mapping(uint_16t Dmm, uint16_t Dmax, uint16_t Delay_min, uint16_t Delay_max);


// Measure  distance  to  an  obstacle using  the  HC-SR04  ultrasonic
// sensor.  Then plot data to vscode's teleplot.

int main(void)
{
    init();
    Serial.begin(9600);

    // Assign addresses of atmege328p ports to pointer address so that
    // if the ports change(we choose to use different ports) the only
    // change to our code in these 3 lines, not having to change every
    // DDRB, PORTB... ports references in our code.
    
    // Use atmega328p portB to interface to the ultrasonic sensor.

    // Volatile  type makes  sure that  the compiler  doesn't optimise
    // away the contents of the register.  It must always be read from
    // memory as  some bit  of hardware  may change  it, not  only our
    // program can change it.

    // Interface of atmega328p ports to ultrasonic sensor.
    volatile uint8_t *ddr_sonar = &DDRB; // Use Data Direction Register B.
    volatile uint8_t *port_sonar = &PORTB; // Use port B register.

    // Input register(reads actual pin voltage)
    volatile uint8_t *pin_sonar = &PINB;
    
    // Interface of atmega328p ports to the passive buzzer.
    volatile uint8_t *ddr_buzzer = &DDRC; 
    volatile uint8_t *port_buzzer = &PORTC;
    

    // Interface the the passive buzzer to arduino port.
    bitSet(*ddr_buzzer, pin_passive_buzzer);

    // Set pin  PB0, that is pin  0 of port  B to an output  port.  It
    // will generate  the trigger  signal required for  the ultrasonic
    // sensor to start measuring distances to objects.
    bitSet(*ddr_sonar, pin_trigger);

    // Set pin PB1, that is pin 1 of port B to an input port.  It will
    // receive the echo  signal from the ultrasonic  sensor.  The echo
    // signal is the  signal returned when the trigger  signal hits an
    // obstacle.
    bitClear(*ddr_sonar, pin_echo); // Set PB1 as an input port.

    // Maximum  sensor measurment  distance is  5m. v=d/t,  thus t/count
    // needs  to  be  no  more than  2*5/343  sec  =  0.0292s=29200us.
    // log2(29200) ~=15  < 16 thus  a uint16_t is sufficient  to count
    // the number  of microseconds we  wait for echo detection  in the
    // sensor.  Remember that the velocity of sound is about 343m/s at
    // 25degrees.
    //
    // log2(29200) means how many times do you take the power of 2 to
    // get 29200, this means you need approximatly 15-16bits. uint16_t
    // is 2 bytes.  So ok.  Maximum unsigned int value is 655,535.
    //
    // Counts the number of microseconds that echo signal is high.
    //
    // Note: Velocity  = distance/time,  so,
    // D(distance to  obstacle) = (time_echo_signal_is_high * V/2).
    // Which   implies   that
    // time_echo_signal_is_high = 2D/V.
    //
    // The velocity of a sound wave is 346m/s@25degreesC.
    uint16_t count = 0;

    float velocity_of_sound = 343; // Velocity of sound about 343m/s at 25degreesC.

    // Accounts for a little bit more than 5m range of sensor.
    uint16_t timeout_counter = 30000;

    unsigned long playtime_us = 100000;

; // 100ms, try 100 * 1e3 later



    while (1) {
        count = 0;
        timeout_counter = 30000;
        

        // Send the  trigger signal (low-high-low where  high needs to
        // be at  least 10us TTL)  to the ultrasonic sensor,  which it
        // needs to measure the round trip distance to the obstacle.
        bitClear(*port_sonar, pin_trigger);
        _delay_us(2);
        bitSet(*port_sonar, pin_trigger);
        _delay_us(11);
        bitClear(*port_sonar, pin_trigger);

        // We need to check if the signal on the echo pin has gone high.
        while (!bitCheck(*pin_sonar, pin_echo))
            // We wait till it's true that the echo pin has gone high
            // to exit the wait/while loop.

            // ??? Should we do a delay here so we don't spin the CPU
            // to hard.
            //_delay_us(1);
            ;


        // When  the  echo pin  has  gone  high  we can  start  timing
        // time_echo_signal_is_high in  us.  If  we don't  receive and
        // echo within the timout period in us we stop timing.
        //
        // We read the echo signal from  the pin_echo input pin of the
        // PINB input register.
        while (bitCheck(*pin_sonar, pin_echo) && timeout_counter--){
            count++;
            _delay_us(1);
        }

        // Note: Velocity  = Distance/Time,  so,
        // D(distance to  obstacle, Dmm) = (time_echo_signal_is_high * V/2).

        // Remember that the Distance value returned by the ultrasonic
        // sensor is 2D,  the round trip distance  of the trigger/echo
        // signals.
        
        // Which   implies   that time_echo_signal_is_high = 2D/V.
        //
        // The velocity of a sound wave is 346m/s@25degreesC.
        //

        // Dmm in ms.
        float Dmm = (float)count / 1.0e6 * velocity_of_sound / 2 * 1000;

        // Prepare  Dmm  for  input to  vscode  teleplot(plots  serial
        // character values output to the serial port).

        // It must be in this format for teleplot to plot values.
        Serial.print(">Dmm:");
        Serial.println(Dmm, 6); // print Dmm to precision of 6 decimal points
        
        // Delay required for HC-SR04 ultrasonic`sensor so it doesn't
        // get triggers to fast.  That is, the sensor needs a minimum
        // delay between trigger pulses to reset.
        _delay_ms(200);

        //float delay = linear_mapping(Dmm, Dmax, Delay_min, Delay_max);

        // duty cycle = (time_signal_high / period) * 100 = 50%
        // playtime_us = 100ms == 0.1us
        // frequency = play middle C, ie C4
        playSound(523.25, 0.5, 0.1);

//        _delay_ms(delay);
    }

    

    return 0;
}



void playSound(float frequency, float duty_cycle, unsigned long playtime_us){

    float period_us = (1/frequency)*1e6; // Period in microseconds.
    unsigned int time_signal_high_us = (duty_cycle * period_us); // in us
    unsigned int time_signal_low_us = (period_us - time_signal_high_us); //in us

    // Number of times our for loop needs to excute.
    unsigned long cycles_for_playtime; 
    

    // cycles  is how  many  periods  fit into  the  whole playing  time
    // required.  As  one  invocation  of  our for  loop  below  is  one
    // period. in length.
    cycles_for_playtime = ( (unsigned long)(playtime_us/period_us) );

    for (unsigned long j=0; j<cycles_for_playtime; j++){

        // Play for one Period forech high-low combinations
        PORTC |= (1<<pin_passive_buzzer); // Set output of portB pin0 to high
        for (unsigned int i=0; i<time_signal_high_us; i++){
            _delay_us(1);
        }

        PORTC &= ~(1<<pin_passive_buzzer); // Set output of portB pin0 to low
        for (unsigned int i=0; i<time_signal_low_us; i++){
            _delay_us(1);
        }

    }
    
}




