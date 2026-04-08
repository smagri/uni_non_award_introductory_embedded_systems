#ifndef F_CPU
#define F_CPU 16000000UL
#endif /* F_CPU 16000000UL */



#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>



// These  are my  version of  the bit  manipulation functions,  so you
// don't need to know the order of precedence rules.

// bitSet() Sets bit n of register reg

// bitClear() Sets bit n to 0 in register reg

// Note bigCheck()  and bitRead() do  the same thing, they  return 0/1
// depending  on   what  n   bit  value   we  require   from  register
// reg. bigCheck() however has extra parentheses for safety.

// bitInverse() Toggles of flips bit n in register reg.

#define bitSet(reg, n) (reg |= (1 << n))
#define bitRead(reg, n) ((reg >> n) & 1)
#define bitCheck(reg, n) ( ((reg) >> (n)) & 1 )
#define bitClear(reg, n) (reg &= ~(1 << n))
#define bitInverse(reg, n) (reg ^= (1 << n))


// Setup State Types
typedef enum{
    AUTO_MODE,
    MANUAL_MODE,
    EMERGENCY_MODE
} State;


// State Machine modes functions.
State auto_traffic_lights(void);
State traffic_light_sonar_system(volatile uint8_t *ddr_switch,
                                 volatile uint8_t *port_switch,
                                 volatile uint8_t *port_switch_inputs,
                                 volatile uint8_t *ddr_sonar,
                                 volatile uint8_t *port_sonar,
                                 volatile uint8_t *port_sonar_inputs,
                                 volatile uint8_t *ddr_buzzer,
                                 volatile uint8_t *port_buzzer);

// Sonar/Pedestrian functions
void playSound(float frequency, float duty_cycle, unsigned long playtime_us,
               volatile uint8_t *port_buzzer);
float linear_mapping(float Dmm, float Dmin, float Dmax,
                     float buzzer_delay_min, float buzzer_delay_max);
void debounce_switch(volatile uint8_t *ddr_switch, volatile uint8_t *port_switch,
                     volatile uint8_t *port_switch_inputs,
                     volatile uint8_t *ddr_sonar, volatile uint8_t *port_sonar,
                     bool *system_on_off_toggle);

// Debugging USART functions
void usart_debugging(void);
void usart_init(float baud_rate);
void usart_flush(void);
void usart_read_string(char *ptr_to_str);
void usart_send_byte(unsigned char data);
void usart_send_string(const char *ptr_to_str);
void usart_send_num(float num, char num_int, char num_decimal);
void print_usart_debugging_mode_menu(void);
void set_user_required_usart_debugging_mode(int8_t user_choice);


// Global definitions, hence available to all functions.
#define BUFFER_SIZE 50

// PB0 equates to number 0 as seen on vscode when you press control
// and mouse click on the variable, eventually it leads you to this
// definition. ie PB0->PORTB->0.
#define pin_trigger PB0
#define pin_echo PB1
#define pin_passive_buzzer PC0
// This is still a PWM port on the arduino, where PD0 and PD1 look
// like Tx/Rx serial port comms.
#define pin_switch PD2
// I am still going to use the onboard LED to indicate what state the
// circuit is in.  Decided against it as not in spec and it uses port
// B which I'm already using for the sonar
#define led_onboard PB5

// Setup pins  for auto mode leds.   Other portB pins are  assigned to
// the sonar/ultrasonic sensor.
#define pin_led_red PB2
#define pin_led_green PB3
#define pin_led_yellow PB4


// Our program buffer  that stores TX/RX data for the  Arduino that we
// want to transmit from MCU->PC or recive data from the PC->MCU.
//
// all elements of usart_buffer are initially assgined to 0.
char usart_buffer[BUFFER_SIZE] = {0};
char *ptr_to_usart_buffer = usart_buffer;

// Flags for respective  main state machine modes  to print debugging,
// or  not, data  as it  is calculated.   Input is  taken from  the PC
// serial  monitor and  recived by  the MCU  via an  interrupt service
// routine, hence this is a non-blocking uart RX buffer read.
bool usart_debugging_mode_pedestrian_distance = 0;
bool usart_debugging_mode_led_brightness = 0;
bool usart_debugging_mode_1_and_2_values = 0;

// Set to volatile as can be changed in the ISR.  Indicates the RX
// xfer of string from PC to MCU is complete or not.
volatile bool flag_rx_done = 0; // Initialised to not complete.



ISR(USART_RX_vect){

    // We have recived the interrupt indicating the RX buffer contains
    // data.  Remember first we need  to enable the interrupt for, data
    // in RX buffer, with the RXCIE0 flag in UCSR0B.
    
    char tmp = UDR0;

    //usart_send_string("\ndbg: USART_RX_vect(): in ISR\n");

    if (flag_rx_done == 0){
        // If we  have reached the end  of string sent from  the PC to
        // the MCU. Terminate the string as per C standard.  Otherwise
        // put the UDRO RX data into our usart_buffer.
        if ( (tmp == '\r') || (tmp == '\n') ){
            *ptr_to_usart_buffer = '\0';
            flag_rx_done = 1; // xfer from PC to MCU complete
        }
        else{
            *ptr_to_usart_buffer++ = tmp;
        }
    }
    
}



int main(void)
{
    // Assign addresses of atmege328p ports to pointer address so that
    // if the ports change(we choose to use different ports) the only
    // change to our code in these 3 lines, not having to change every
    // DDRB, PORTB... ports references in our code.
    
    // Use atmega328p ports to interface to the ultrasonic sensor.

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

    // Interface of atmega328p ports to toggle on and off switch.
    volatile uint8_t *ddr_switch = &DDRD; 
    volatile uint8_t *port_switch = &PORTD;
    volatile uint8_t *port_switch_inputs = &PIND;


    // Interface the the passive buzzer to arduino output port.
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
    bitSet(*port_sonar, pin_echo); // enable the pull up

    // Configure pin as an input.  Then set it high as that enables
    // its pullup resistor on the arduino pin.
    bitClear(*ddr_switch, pin_switch);
    bitSet(*port_switch, pin_switch);

    // Configure pin 5 on port B, connected to the onboard LED, on the
    // arduino uno r3, as an output port.
    bitSet(*ddr_sonar, led_onboard);


    // Setup traffic light LEDs in portB
    bitSet(*ddr_sonar, pin_led_red);
    bitSet(*ddr_sonar, pin_led_green);
    bitSet(*ddr_sonar, pin_led_yellow);

    

    // Initialise the USART.
    usart_init(9600);
    _delay_ms(100);

    sei(); // Enable global interrupts.
    
    
    State state_current;
    state_current = AUTO_MODE;

    usart_flush();
    print_usart_debugging_mode_menu();


    while (1) {

        usart_debugging();
        
        switch (state_current) {
            case AUTO_MODE: {

                //state_current = auto_traffic_lights();
                
                state_current = traffic_light_sonar_system(ddr_switch,
                                                           port_switch,
                                                           port_switch_inputs,
                                                           ddr_sonar,
                                                           port_sonar,
                                                           port_sonar_inputs,
                                                           ddr_buzzer,
                                                           port_buzzer);


                // if (condition) {
                //     state_current = NEXT_STATE;
                // }
                break;
            }
            case MANUAL_MODE: {
            
                break;
            }
            case EMERGENCY_MODE: {
            
                break;
            }
            default:
                break;
        }



        // Yield our process to be  nice to other processes running on
        // your  system/computer.   However,  remove if  makes  system
        // unresponsive.
        //_delay_ms(100);
    }
    
    return 0;
}




///////////////////////////////////////////////////////////////////////////////
//                           User defined functions                          //
///////////////////////////////////////////////////////////////////////////////


State traffic_light_sonar_system(volatile uint8_t *ddr_switch,
                                 volatile uint8_t *port_switch,
                                 volatile uint8_t *port_switch_inputs,
                                 volatile uint8_t *ddr_sonar,
                                 volatile uint8_t *port_sonar,
                                 volatile uint8_t *port_sonar_inputs,
                                 volatile uint8_t *ddr_buzzer,
                                 volatile uint8_t *port_buzzer){


    // // Assign addresses of atmege328p ports to pointer address so that
    // // if the ports change(we choose to use different ports) the only
    // // change to our code in these 3 lines, not having to change every
    // // DDRB, PORTB... ports references in our code.
    
    // // Use atmega328p ports to interface to the ultrasonic sensor.

    // // Volatile  type makes  sure that  the compiler  doesn't optimise
    // // away the contents of the register.  It must always be read from
    // // memory as  some bit  of hardware  may change  it, not  only our
    // // program can change it.

    // // Interface of atmega328p ports to ultrasonic sensor.
    // volatile uint8_t *ddr_sonar = &DDRB; // Use Data Direction Register B.
    // volatile uint8_t *port_sonar = &PORTB; // Use port B register.

    // // Input register(reads actual pin voltage)
    // volatile uint8_t *port_sonar_inputs = &PINB;
    
    // // Interface of atmega328p ports to the passive buzzer.
    // volatile uint8_t *ddr_buzzer = &DDRC; 
    // volatile uint8_t *port_buzzer = &PORTC;

    // // Interface of atmega328p ports to toggle on and off switch.
    // volatile uint8_t *ddr_switch = &DDRD; 
    // volatile uint8_t *port_switch = &PORTD;
    // volatile uint8_t *port_switch_inputs = &PIND;


    // Ultrasonic  sensor  we  are  using is  the  HR-SR04,  with  the
    // following specification.
    //
    // When we have  linear mapping we know that distance  vs delay is
    // linear over a feasible  distance.  Hence, the FEASIBLE DISTANCE
    // is Dmin to Dmax.  We  may get incorrect distance values outside
    // that region and meaningless buzzer delays.
    float Dmin = 10; // minimum feasible distance for ultrasonic sensor, mm.
    float Dmax = 100;// maximum feasible distance for ultrasonic sensor, mm.

    // WRKS:
    // I think you can't hear the buzzer below a 10ms on high pulse.
    //float buzzer_delay_min = 50; // maximum feasible buzzer delay, ms.
    //float buzzer_delay_max = 500; // maximum feasible buzzer delay, ms.


    // Linear mapping equation: slope = (y2 -y1)/(x2-x1)
    //
    // We have y as delay and x as distance.
    //
    // Note buzzer_delay_max(y2) is  greater than buzzer_delay_min(y1)
    // so the  slope is  positive.  Hence, we  get shorter  delays for
    // shorter distances.
    //
    // If we  swap buzzer_delay_max  and buzzer_delay_min  and keeping
    // the Dmin and Dmax the same.  So  y2 is smaller than y1 we get a
    // negative  slope.   Hence  we  get  longer  delays  for  shorter
    // distances.

    //float buzzer_delay_min = 40; // minimum feasible buzzer delay, ms.
    //float buzzer_delay_max = 3000; // maximum feasible buzzer delay, ms.
    //float buzzer_delay_min  = 20; // minimum feasible buzzer delay, ms.
    //float buzzer_delay_max = 500; // maximum feasible buzzer delay, ms.
    float buzzer_delay_min = 500; // minimum feasible buzzer delay, ms.
    float buzzer_delay_max = 20; // maximum feasible buzzer delay, ms.


    
    // // Interface the the passive buzzer to arduino output port.
    // bitSet(*ddr_buzzer, pin_passive_buzzer);

    // // Set pin  PB0, that is pin  0 of port  B to an output  port.  It
    // // will generate  the trigger  signal required for  the ultrasonic
    // // sensor to start measuring distances to objects.
    // bitSet(*ddr_sonar, pin_trigger);

    // // Set pin PB1, that is pin 1 of port B to an input port.  It will
    // // receive the echo  signal from the ultrasonic  sensor.  The echo
    // // signal is the  signal returned when the trigger  signal hits an
    // // obstacle.
    // bitClear(*ddr_sonar, pin_echo); // Set PB1 as an input port.
    // bitSet(*port_sonar, pin_echo); // enable the pull up

    // // Configure pin as an input.  Then set it high as that enables
    // // its pullup resistor on the arduino pin.
    // bitClear(*ddr_switch, pin_switch);
    // bitSet(*port_switch, pin_switch);

    // // Configure pin 5 on port B, connected to the onboard LED, on the
    // // arduino uno r3, as an output port.
    // bitSet(*ddr_sonar, led_onboard);
    // bitClear(*port_sonar, led_onboard); // Start with the LED off

    
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

    // Total  time to  play the  buzzer  sound.  Much  slower and  the
    // ultrasonic sensor  triggers are too infrequent  or don't happen
    // at all.
    unsigned long playtime_us = 10000; // 10ms

    // When  the switch  is toggled  our program  runs or  stops after
    // debounceing the  switch.  We start off  with the system/circuit
    // off.
    static bool system_on_off_toggle = false;

    
    // Debounce the switch.
    debounce_switch(ddr_switch, port_switch, port_switch_inputs,
                    ddr_sonar, port_sonar, &system_on_off_toggle);

    // If system/circuit is off skip everything.
    if (!system_on_off_toggle){
        //continue;
        return AUTO_MODE;
    }
        
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
        ;

    // When  the  echo pin  has  gone  high  we can  start  timing
    // time_echo_signal_is_high in  us.  If  we don't  receive and
    // echo within the timout period in us we stop timing.
    //
    // We  are  waiting  for  the  echo   pin  going  high  as  it  is
    // proportional to the distance the ultrasonic sensor measures.
    //
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
        
    // Multiply by a 1000 to get mm as velocity of sound is in
    // seconds.
    float Dmm
        = ((float)echo_high_us_count / 1.0e6)* velocity_of_sound / 2 * 1000;
        

    // Prepare  Dmm  for  input to  vscode  Teleplot(plots  serial
    // character values output to the serial port).
    //
    // It must be in this format for TELEPLOT to plot values.
    // Name of teleplot can change by RENAMING Dmm string to
    // something else, that is >SomethingElse:\n
    //
    if (usart_debugging_mode_pedestrian_distance){
        // Print  out   Distance_mm,  the  current  distance   to  the
        // pedestrian via vscode Teleplot.  Values sent to Teleplot in
        // format it expects.

        for (int i=0; i<10; i++){
            Dmm = i;
            usart_send_string(">sonar_distance_mm:");
            // Send output to Teleplot
            usart_send_num(Dmm, 5, 6);
            // Telepot value terminating character.
            usart_send_string("\n");

            _delay_ms(100);

        }
    }

    
    // Linearly map  the distance, from  the ultrasonic sensor  to the
    // object, against the buzzer delay.  
    float delay = linear_mapping(Dmm, Dmin, Dmax,
                                 buzzer_delay_min, buzzer_delay_max);
 
    // Play the buzzer at the  frequency, duty cycle and total playing
    // time required in us.  The buzzer playing is proportional to the
    // distance the object is from the ultrasonic sensor.
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

        
    return AUTO_MODE;
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

    // To  get  slower  beeps  you  need  to  increase  the  slope  by
    // increasing buzzer_delay_max.  So that you get bigger delays for
    // more quickly for the same range of Dmax and Dmin.
    
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

    // If we SWAP  buzzer_delay_max and buzzer_delay_min we  get a -ve
    // slope.  Hence,  we get faster  beeping at longer  distances and
    // slower beeping at shorter distances.   Thus for a small x delay
    // is big and for  a bigger x delay is smaller.   Draw a -ve slope
    // graph to see this.

    
    // Note: You lose precision with integer division because integers
    // cannot store  fractional parts. When two  integers are divided,
    // the result is truncated (the decimal part is discarded).
        
    delay = buzzer_delay_min
        + (Dmm - Dmin)
        * (buzzer_delay_max - buzzer_delay_min)
        / (Dmax - Dmin);

    return delay;
}



// Button/switch pin debouncing.  We execute  a delay of ms(amount has
// to be determined emperically because  every button is different) on
// the transition of  the button form high-low and  low-high to factor
// in the the noise/ringing on the signal at the transitions.

void debounce_switch(volatile uint8_t *ddr_switch, volatile uint8_t *port_switch,
                     volatile uint8_t *port_switch_inputs,
                     volatile uint8_t *ddr_sonar, volatile uint8_t *port_sonar,
                     bool *system_on_off_toggle){

    // Must be persistant between calls in main()'s while loop.
    static bool switch_status_old = 1;
    bool switch_status;
    
    
    // What  value is  where  we  have the  button  connected.  It  is
    // intially high.  Wait for the transition from high-low to toggle
    // switch, changing its state from its previous state.
    switch_status = bitRead(*port_switch_inputs, pin_switch);

    // Keep reading  switch_status with  the previous command  till it
    // transitions from high-low or low-high.  That is when the switch
    // is pressed or released.
    if(switch_status != switch_status_old){

        // Button has transitioned from high-low or low-high.

        // Wait a while due to ringing signal on transition
        // high-low or low-high.
            
        // You need to tune this delay  based on the unique button you
        // have.  Mechanical switches roughly bounce for
        // 5-20ms.
        // not enough for my switch as occasionally i don't get a toggle
        //_delay_ms(10);
        _delay_ms(50);
        switch_status = bitRead(*port_switch_inputs, pin_switch);


        // If now after the wait for switch bouncing to stop on
        // the transition and the pin has really changed state.
        if (switch_status != switch_status_old){
            //Serial.println("dbg: debounce_switch()): Transition detected");
            switch_status_old = switch_status;

            
            // We only  toggle on the press  edge, high-low tansition.
            // Not on both edges high-low and low-high.
            if(switch_status == 0){
                //Serial.println("dbg: debounce_switch(): Button pressed");
                // toggle the state of the led
                bitInverse(*port_sonar, led_onboard);

                // toggle system state/circuit on or off.
                *system_on_off_toggle = !(*system_on_off_toggle);
                // Serial.println(*system_on_off_toggle);
                return;
            }
        }

    } // end: if(switch_status != switch_status_old){
       
} // end: debounce_switch()



State auto_traffic_lights(void){

    // Set PB2,  PB3, PB4 as outputs.   Other portB pins are  used for
    // the sonar/ultrasonic sensor and  are left unchanged.  Note that
    // doing it  this way does not  let you change the  pinout for the
    // leds later.
    // DDRB |= 0x1C; 

    // // LEDs on Left->Right continuously.
    // for (uint8_t i = PB2; i <= PB4; i++)
    // {
    //     PORTB &= 0xE3;
    //     PORTB |= (1 << i);
    //     _delay_ms(1000); // 1 second between led changes
    // }
    DDRB |= (1 << PB2) | (1 << PB3) | (1 << PB4);
    
    for (uint8_t i = PB2; i <= PB4; i++)
    {
        // Clear only PB2, PB3, PB4 (leave others untouched)
        PORTB &= ~((1 << PB2) | (1 << PB3) | (1 << PB4));

        // Turn ON current LED
        PORTB |= (1 << i);

        _delay_ms(1000);
    }

    
    if (usart_debugging_mode_led_brightness){

        // Print out current ambient brightness
        float led_light_level;

        // Debugging
        for (int i=0; i<10; i++){
            led_light_level = i;
            usart_send_string(">led_light_level(%):");
            // Send output to Teleplot
            usart_send_num(led_light_level, 4, 0);
            // Telepot value terminating character.
            usart_send_string("\n");
            
            _delay_ms(100);
        }

         
    }

    return AUTO_MODE;
}




// Debugging USART functions //////////////////////////////////////////////////
void usart_debugging(void){

    // Sets mode of debugging according to request from user PC serial
    // monitor.  Option for no debugging  also exists.  It can also be
    // used with vscode Teleplot.
    //
    // PC byte to MCU input reading  is into the USART RX register and
    // is non-blocking as it is under USART_RX ISR control.
    
    uint8_t user_choice; // User choice for debugging mode.
    
    // Nothing to do if we haven't recived an input from the PC serial
    // monitor yet, via the USART_RX interrupt service routine.
    if (!flag_rx_done)
        return;

    // Xfer from  PC to MCU is  complete and via ISR.   We can
    // now process that data.
        
    // Check if it's  exactly one digit 1–3 by  checking the ASCII
    // numbers.  Also,  reject non single character  entries, such
    // decimals and outside range ascii characters.
    if ( (usart_buffer[0]<'0')
         || (usart_buffer[0]>'3')
         || (usart_buffer[1]!='\0') ){
        usart_send_string("Invalid selection. Try again\n");

        // Ignore this  byte and get  ready for the  next byte
        // from the RX buffer.
    }
    else{
        // Convert input string into an integer.
        //
        // note: atoi() expects a null - terminated string.
        user_choice = atoi(usart_buffer);
        // usart_send_string("dbg: main(): User_choice is: ");
        // usart_send_num(user_choice, 1, 0);
        // usart_send_byte('\n'); // send EOL, CRLF
        // Set debugging mode flags.
        set_user_required_usart_debugging_mode(user_choice);
    }
    
        
    // Get the next byte from the RX buffer.
    ptr_to_usart_buffer = usart_buffer;
    flag_rx_done = 0;
    usart_flush();
        
    print_usart_debugging_mode_menu();

    return;
}



void print_usart_debugging_mode_menu(void){

    // Output the menu for debugging operations.
    usart_send_byte('\n');
    usart_send_string("\nEnter a number for usart debugging required:\n");
    usart_send_string("0. Switch off debugging mode, no data from MCU to PC.\n");
    usart_send_string("1. Send pedestrian distance measurments to PC.\n");
    usart_send_string("2. Send ambient brightness measurments to PC.\n");
    usart_send_string("3. Both data in mode 1 and 2 sent to PC.\n");
    
}


void set_user_required_usart_debugging_mode(int8_t user_choice){

    // Set program state for debugging operations.

    switch (user_choice) {
        case 0: {
            usart_send_string("\nDebugging mode has been turned off.\n");
            usart_debugging_mode_pedestrian_distance = 0;
            usart_debugging_mode_led_brightness = 0;
            break;
        }
        case 1: {
            usart_send_string("\nSending pedestrian distance measurments");
            usart_send_string(" to PC via Teleplot\n");
            usart_debugging_mode_pedestrian_distance = 1;
            usart_debugging_mode_led_brightness = 0;
            break;
        }
        case 2: {
            usart_send_string("\nSending ambient brightness measurements");
            usart_send_string(" to PC via Teleplot\n");
            usart_debugging_mode_led_brightness = 1;
            usart_debugging_mode_pedestrian_distance = 0;
            break;
        }
        case 3: {
            usart_send_string("\nSending both data measurements from mode 1 and 2");
            usart_send_string(" to PC via Teleplot\n");
            usart_debugging_mode_pedestrian_distance = 1;
            usart_debugging_mode_led_brightness = 1;
            break;
        }
        default:
            break;
    }

}



void usart_init(float baud_rate){


    // Setting the baud rate register:
    //
    // Fosc, internal Crystal Arduino r3 = 16MHz
    // ubrr0 = (Fosc/prescale factor * baud rate) - 1
    // U2Xn=0, prescale factor is 2/4/2=16
    // U2Xn=1, prescale factor is 2/4=8
    
    float ubrr0 = ( (F_CPU / (16.0 * baud_rate)) - 1);
    int ubrr0a = (int)ubrr0;

    // Round up if neccessary to the next integer.
    if (ubrr0 - ubrr0a >= 0.5) {
        ubrr0a = ubrr0a + 1;
    }

    // Set baud rate
    UBRR0 = ubrr0a;
    
    // Enable transmitter and receiver, and the Interrupt Service
    // Routine on the RX buffer.
    UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);
    
    // Set frame format: 8 bit data, 1 stop bit, no parity
    UCSR0C = (0 << UMSEL01) | (0 << UMSEL00) |  // Async mode
             (0 << UPM01) | (0 << UPM00) |        // No parity
             (0 << USBS0) |                       // 1 stop bit  
             (1 << UCSZ01) | (1 << UCSZ00);       // 8-bit character

}


void usart_flush(void){
    char dummy;

    // Check if the UCSROA(USART Control  and Status Register) has the
    // RXC0 flag  set.  If flag set  then there is unread  data in the
    // receive buffer.  Read what's in  the RX buffer which clears the
    // buffer.
    while (bitCheck(UCSR0A, RXC0))
        dummy = UDR0;

    ((void)dummy); // To suppress warning
}



void usart_send_byte(unsigned char data){

    // Send a byte  when the UDRE0(USART data register  empty flag) id
    // is set.  In the UCSR0A (UART contorl or status register A).  It
    // means that the USART data register is empty.
    
    while (!bitCheck(UCSR0A, UDRE0))
        ;
    UDR0 = data; 
}




// check if const is really needed, cgpt says c++ compiler is strictor
// than c compiler.
void usart_send_string(const char *pstr){
    
    // note:  const means  a  read-only-string, prevents  accidentally
    // modifying  string   literals(which  causes   crashes).   String
    // literals in c  are a sequence of characters  enclosed in double
    // quotes, that is, an constant  array of characters that are null
    // terminated, ie '\0' terminated.

    // Send each byte, one at a  time, till the string terminator '\0'
    // is  sent.  '\0'  is literally  the numeric  value 0  written in
    // character form(NULL ACSII character).
    while (*pstr != '\0') {
        usart_send_byte(*pstr);
        pstr++;
    }
}



void usart_send_num(float num, char num_int, char num_decimal){

    // Send a number from MCU to PC by converting it to a string.

    // dtostrf(num,  width, precision,  string) converts  float to  an
    // ASCII string.  We add a string  terminator '\0' so the PC knows
    // when we are at the end of a string).

    
    char str[20];
    if (num_decimal == 0)
        dtostrf(num, num_int, num_decimal, str);
    else
        dtostrf(num, (num_int+num_decimal+1), num_decimal, str);
    str[num_int+num_decimal+1] = '\0';
    usart_send_string(str);
        
}
