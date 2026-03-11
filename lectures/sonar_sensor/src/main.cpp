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
//#define bitSet(reg, n) (reg |= (1 << n))
#define bitSet(reg, n) (reg |= 1 << n)
#define bitRead(reg, n) ((reg >> n) & 1)
//#define bitCheck(reg, n) ( ((reg) >> (n)) & 1 )
#define bitCheck(reg, n) (reg >> n & 1)
#define bitClear(reg, n) (reg &= ~(1 << n))
//#define bitInverse(reg, n) (reg ^= (1 << n))

// PB0 equates to number 0 as seen on vscode when you press control
// and mouse click on the variable, eventually it leads you to this
// definition. ie PB0->PORTB->0.
#define pin_trigger PB0
#define pin_echo PB1


// Measure  distance  to  an  obstacle using  the  HC-SR04  ultrasonic
// sensor.  Then plot data to vscode's teleplot.

int main(void)
{
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
    volatile uint8_t *pin_sonar = &PINB; // Input register(reads
                                         // actual pin voltage)

    // Set pin  PB0, that is pin  0 of port  B to an output  port.  It
    // will generate  the trigger  signal required for  the ultrasonic
    // sensor to start measuring distances to objects.
    bitSet(*ddr_sonar, pin_trigger);

    // Set pin PB1, that is pin 1 of port B to an input port.  It will
    // receive the echo  signal from the ultrasonic  sensor.  The echo
    // signal is the  signal returned when the trigger  signal hits an
    // obstacle.
    bitClear(*ddr_sonar, pin_echo); // Set PB1 as an input port.

    // Maximum  sensor measurment  distance is  5m. v=d/t,  thus t/cnt
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
    uint16_t cnt = 0;

    float vel_sound = 343; // Velocity of sound about 343m/s at 25degreesC.

    // Accounts for a little bit more than 5m range of sensor.
    uint16_t timeout_counter = 30000;



    while (1) {
        cnt = 0;
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
            cnt++;
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
        float Dmm = (float)cnt / 1.0e6 * vel_sound / 2 * 1000;

        // Prepare  Dmm  for  input to  vscode  teleplot(plots  serial
        // character values output to the serial port).
        Serial.print(">Dmm:");
        Serial.println(Dmm, 6); // print Dmm to precision of 6 decimal points

    }

    

    return 0;
}


