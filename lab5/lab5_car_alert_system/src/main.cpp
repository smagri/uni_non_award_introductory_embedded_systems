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



void playSound(float frequency, float duty_cycle, unsigned long playtime_us,
               volatile uint8_t *port_buzzer);
float linear_mapping(float Dmm, float Dmin, float Dmax,
                     float buzzer_delay_min, float buzzer_delay_max);


// Measure  distance  to  an  obstacle using  the  HC-SR04  ultrasonic
// sensor.  Then plot data to  vscode's teleplot.  Also, emit a buzzer
// pulse to indicate you are measuring distances.

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
    volatile uint8_t *port_sonar_inputs = &PINB;
    
    // Interface of atmega328p ports to the passive buzzer.
    volatile uint8_t *ddr_buzzer = &DDRC; 
    volatile uint8_t *port_buzzer = &PORTC;

    // WORKS:
    
    // float Dmin = 50; // minimum feasible distance for ultrasonic sensor, mm.
    // float Dmax = 2000;// maximum feasible distance for ultrasonic sensor, mm.
    // float buzzer_delay_min = 50; // maximum feasible buzzer delay, ms.
    // float buzzer_delay_max = 500; // maximum feasible buzzer delay, ms.

    // EXPERIMENT, works as well:

    // Ultrasonic  sensor  we  are  using is  the  HR-SR04,  with  the
    // following specification.
    //
    //float Dmin = 20; // minimum feasible distance for ultrasonic sensor, mm.
    // float Dmax = 4000;// maximum feasible distance for ultrasonic sensor, mm.

    // // I think you can't hear the buzzer below a 10ms on high pulse.
    // float buzzer_delay_min = 50; // maximum feasible buzzer delay, ms.
    // float buzzer_delay_max = 500; // maximum feasible buzzer delay, ms.

    // Didn't sound good, same buzzzing frequency that is not easily
    // distinguishable.  These are the feasible distances in the
    // lecture notes.
    //
    // float Dmin = 20; // minimum feasible distance for ultrasonic sensor, mm.
    // float Dmax = 200;// maximum feasible distance for ultrasonic sensor, mm.
    // float buzzer_delay_min = 1; // maximum feasible buzzer delay, ms.
    // float buzzer_delay_max = 20; // maximum feasible buzzer delay, ms.

    // When we have  linear mapping we know that distance  vs delay is
    // linear over a feasible  distance.  Hence, the FEASIBLE DISTANCE
    // is Dmin to Dmax.

    // Modulo 1
    //float Dmin = 100; // minimum feasible distance for ultrasonic sensor, mm.
    //float Dmax = 150;// maximum feasible distance for ultrasonic sensor, mm.
    // Modulo 3
    float Dmin = 200; // minimum feasible distance for ultrasonic sensor, mm.
    float Dmax = 250;// maximum feasible distance for ultrasonic sensor, mm.
    float buzzer_delay_min = 50; // maximum feasible buzzer delay, ms.
    float buzzer_delay_max = 500; // maximum feasible buzzer delay, ms.

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

    // Maximum  sensor measurment  distance is  5m. v=d/t,thus t/echo_high_us_count
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
    // D(distance to  obstacle)=(time_echo_signal_is_high*velocity_of_sound/2).
    // Which   implies   that:
    //
    // time_echo_signal_is_high=2D/V=2*distance_to_obstacle/velocity_of_sound.
    //
    // The velocity of a sound wave is 346m/s@25degreesC.
    uint16_t echo_high_us_count = 0;

    float velocity_of_sound = 343; // Velocity of sound about 343m/s
                                   // at 25degreesC.

    // Accounts for a little bit more than 5m range of sensor.
    uint16_t timeout_counter = 30000;

    // frequency = play middle C, ie C4
    //float frequency = 261.63;

    float frequency = 523.25; // C5, one octave above middle C
    

    // duty cycle = (time_signal_high / period) * 100 = 50%
    //
    // Changine the  duty cycle with  a fixed  T can vary  the average
    // power of  the PWM signal.   Hence, if  we have interfaced  to a
    // passive buzzer it will get louder with the increace of the duty
    // cycle.
    //
    float duty_cycle = 0.5; // 50% duty cycle

    // Much slower and the ultrasonic sensor triggers are too
    // infrequent or don't happen at all.
    unsigned long playtime_us = 10000; // 10ms
    

    while (1) {
        echo_high_us_count = 0;
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
        while (!bitCheck(*port_sonar_inputs, pin_echo))
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
        while (bitCheck(*port_sonar_inputs, pin_echo) && timeout_counter--){
            echo_high_us_count++;
            _delay_us(1);
        }
        // Ensure echo pulse has completely finished.
        while(bitCheck(*port_sonar_inputs, pin_echo));
        
        
        // Note: Velocity  = Distance/Time,  so,
        // D = (time_echo_signal_is_high * velocity_of_sound/2).
        // Where D is the distance_to_the_obstacle in m.

        // Remember that the Distance value returned by the ultrasonic
        // sensor is 2D,  the round trip distance  of the trigger/echo
        // signals.
        
        // Which   implies   that time_echo_signal_is_high = 2D/V.
        //
        // The velocity of a sound wave is 346m/s@25degreesC.
        //
        
        // Also  remember that  echo_high_us_count  is in  us and  the
        // formula requires seconds.   So divide echo_high_us_count by
        // number of us in a second.
        
        // Dmm in mm.
        // float Dmm
        //     = ((float)echo_high_us_count / 1.0e6)* velocity_of_sound / 2 * 1000;
        float Dmm
            = 2 * ((float)echo_high_us_count / 1.0e6)* velocity_of_sound / 2 * 1000;
        

        // Prepare  Dmm  for  input to  vscode  teleplot(plots  serial
        // character values output to the serial port).
        //
        // It  must be  in this  format for  TELEPLOT to  plot values.
        // Name  of teleplot  can change  by RENAMING  >Dmm string  to
        // something else, that is >SomethingElse.
        //
        Serial.print(">Dmm:");
        Serial.println(Dmm, 6); // print Dmm to precision of 6 decimal points
        
        float delay = linear_mapping(Dmm, Dmin, Dmax,
                                     buzzer_delay_min, buzzer_delay_max);

        playSound(frequency, duty_cycle, playtime_us, port_buzzer);

        // Internal function requires a constant known at complie time
        // not at runtime as here.
        //
        // _delay_ms(delay);

        // Delay  after  the  beep/  beep  interval,  proportional  to
        // distance from  ultrasonic sensor to obstacle.   Done, using
        // linear mapping.
        //
        // We are trying to make a fast beep for a close object and a slow
        // beep for a far object.
        
        // Why we  use linear mapping.  eg  a volume knob, as  we increace
        // the volume  x steps we expect  a volume _increase_ that  is the
        // same at 2x steps...


        for (uint16_t i = 0; i<delay; i++) {
            _delay_ms(1);
        }


        // Delay required for HC-SR04  ultrasonic`sensor so it doesn't
        // get triggers to fast.  That  is, the sensor needs a minimum
        // delay between trigger pulses to reset.
        //
        // Started  with a  200ms delay  so the  distance measurements
        // take longer  to aquire.   Hence, the sensor  distances give
        // the appearance of not measuring correctly.
        //
        // _delay_ms();
        //
        // When using playsound(...) we get a >=50ms pulse here anyhow
        // though so we don't need an extra delay here.
        
    } // end: while(1)

    

    return 0;
}



void playSound(float frequency, float duty_cycle, unsigned long playtime_us,
               volatile uint8_t *port_buzzer){

    float period_us = (1/frequency)*1e6; // Period in microseconds.
    unsigned int time_signal_high_us = (duty_cycle * period_us); // in us
    unsigned int time_signal_low_us = (period_us - time_signal_high_us); //in us

    // Number of times our for loop needs to excute.
    unsigned long cycles_for_playtime; 
    

    // cycles  is how  many  periods  fit into  the  whole playing  time
    // required.  As  one  invocation  of  our for  loop  below  is  one
    // period in length.
    cycles_for_playtime = ( (unsigned long)(playtime_us/period_us) );

    for (unsigned long j=0; j<cycles_for_playtime; j++){

        // Play for one Period forech high-low combinations
        bitSet(*port_buzzer, pin_passive_buzzer);
        for (unsigned int i=0; i<time_signal_high_us; i++){
            _delay_us(1);
        }

        bitClear(*port_buzzer, pin_passive_buzzer);
        for (unsigned int i=0; i<time_signal_low_us; i++){
            _delay_us(1);
        }

    }
    
}



float linear_mapping(float Dmm, float Dmin, float Dmax,
                     float buzzer_delay_min, float buzzer_delay_max){

    // Linear mapping equation: slope = (y2 -y1)/(x2-x1)
    //
    // We have y as delay and x as distance.
    //
    // The slope is essentialy the gradient of the straight line, also
    // known as  the rate of change.   The rate of change  in a linear
    // equation is constant.  So when  we use linear mapping, we apply
    // a  constant rate  of change  between  the input  range and  the
    // output range.
    
    float delay;
    
    // Clamp input so it stays in  sensor range.  If you SWAP Dmin and
    // Dmax both clamping statements are true and Dmm, is always equal
    // to Dmax.   Hence, you get a  constant value for delay  and thus
    // beeping.
    //
    // eg Dmin=150 Dmax=100, swapped.
    if (Dmm < Dmin)
        Dmm = Dmin;

    if (Dmm > Dmax)
        Dmm = Dmax;

    // Of we SWAP  buzzer_delay_max and buzzer_delay_min we  get a -ve
    // slope.  Hence,  we get faster  beeping at longer  distances and
    // slower beeping at shorter distances.   Thus for a small x delay
    // is big and for  a bigger x delay is smaller.   Draw a -ve sloap
    // graph to see this.

    
    // Note: You lose precision with integer division because integers
    // cannot store  fractional parts. When two  integers are divided,
    // the result is truncated (the decimal part is discarded).
        
    delay = buzzer_delay_min
        + (Dmm - Dmin)
        * ((buzzer_delay_max - buzzer_delay_min)) / ((Dmax - Dmin));

    return delay;
}
