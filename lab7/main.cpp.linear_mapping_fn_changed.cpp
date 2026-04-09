#ifndef F_CPU
#define F_CPU 16000000UL
#endif // Set system clock frequency



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


// Global definitions, hence available to all functions.
#define BUFFER_SIZE 50

// Define feasible ranges for linear mapping:
#define ADC_MIN 86 // minimum feasible ADC count for photocell
#define ADC_AMBIENT_MAX 200
#define ADC_MAX 935 // maximum feasible ADC count for photocell
#define TIME_ON_MIN 0 // maximum feasible LED time on, ms.
#define PWM_PERIOD_MS 10 // maximum feasible LED time on, ms.

// LED pin connection on atmega328p.
#define pin_led PB1



// Our program buffer  that stores TX/RX data for the  Arduino that we
// want to transmit from MCU->PC or recive data from the PC->MCU.
//
// all elements of usart_buffer is assgined to 0
char usart_buffer[BUFFER_SIZE] = {0};
char *ptr_to_usart_buffer = usart_buffer;

//int adc_cur = 0;
volatile uint16_t adc_cur = 0;


void usart_init(float baud_rate);
void usart_flush(void);
void usart_read_string(char *ptr_to_str);
void usart_send_byte(unsigned char data);
void usart_send_string(const char *ptr_to_str);
void usart_send_num(float num, char num_int, char num_decimal);
//uint32_t linear_mapping(uint16_t adc_cur);
uint8_t linear_mapping(uint16_t adc_cur);
void delay_ms_runtime(uint16_t ms);
void pwm1a_init(void);
void adc_init(void);

ISR(ADC_vect){
        
    //usart_send_string("dbg: in ISR");
    
    // Triggers when interrupt conversion is complete.
    adc_cur = ADC; // Read ADC data register.
}



int main(void)
{

    // DDR  sets direction  → OCR  controls  brightness →  PORT is  no
    // longer used for PWM pins
    
    // Interface the LED to the atmega328p ports.
    volatile uint8_t *ddr_led = &DDRB; // used Data Direction Register B
    //volatile uint8_t *port_led = &PORTB; // use port B register

    // Set pin_led to an output on port_led.
    bitSet(*ddr_led, pin_led);
    //bitClear(*port_led, pin_led); // Start with the LED off

    
    usart_init(9600); // Initialise Regiters
    pwm1a_init();  // PWM1a pin PB1 initialised.
    adc_init(); // ADC init on A1

    sei(); // Enable global interrupts.

    // ADSC sets  the ADC to  start conversions.  Setting this  bit is
    // the  "software trigger"  that  tells the  hardware to  actually
    // begin the  electrical process  of sampling  the voltage  on the
    // pin.   As soon  as  the conversion  is  finished, the  hardware
    // automatically  clears  it  to  0.   This  is  known  as  single
    // conversion mode  as the  conversion occurs  only once  for each
    // time ADSC is set.
    bitSet(ADCSRA, ADSC);
    _delay_ms(10);

    while (1) {

        // Single conversion mode is active, so conversion only occurs
        // once  everytime ADSC  is  set.  Get  the  current value  of
        // signal mark time.
        bitSet(ADCSRA, ADSC);

        // Get ADC register value ready for plotting in vscode telepot:
        // usart_flush();
        // usart_send_string(">adc_cur:");
        // // ADC register value 4 places including decimal point as an integer.
        // usart_send_num(adc_cur, 4, 0);
        // // Telepot value terminating character.
        // usart_send_string("\n");
        // _delay_ms(500);
        
        // _delay_ms(100);
        //uint32_t time_on_cur = linear_mapping(adc_cur);
        
        // Clamp for safety
        // if (time_on_cur > PWM_PERIOD_MS)
        //     time_on_cur = PWM_PERIOD_MS;

        // // Convert from "time on in one PWM period" to 8-bit duty value
        // //uint8_t duty = (uint8_t)((time_on_cur * 255UL) / PWM_PERIOD_MS);
        // uint8_t duty = (uint8_t)((time_on_cur * 255UL) / PWM_PERIOD_MS);
        uint8_t duty = linear_mapping(adc_cur);
        OCR1A = duty;
        // Hardware PWM output on PB1
        //OCR1A = duty;
        // OCR1A = 10;
        // _delay_ms(1000);
        // OCR1A = 80;
        // _delay_ms(1000);
        // OCR1A = 255;
        // _delay_ms(1000);

        // usart_flush();
        // usart_send_string("while loop: time_on_cur=");
        // usart_send_num(time_on_cur, 6, 0);
        // usart_send_byte('\n'); // CRLF or EOL
        // // _delay_ms(500);
        // if (time_on_cur > 0 && time_on_cur < 2)
        //     time_on_cur = 2;
        // if (time_on_cur > PWM_PERIOD_MS)
        //     time_on_cur = PWM_PERIOD_MS;


        // // Deliver  a PWM(Pulse  Width  Modulated) signal  to the  LED
        // // proportional to the current adc value.

        // // // Output signal mark.
        // bitSet(*port_led, pin_led);
        // delay_ms_runtime(time_on_cur);

        // // // Output signal space.
        // uint32_t time_off = (PWM_PERIOD_MS - time_on_cur);
        
        // // usart_flush();
        // // usart_send_string("while loop: time_off=");
        // // usart_send_num(time_off, 6, 0);
        // // usart_send_byte('\n'); // CRLF or EOL
        // // _delay_ms(500);
        // bitClear(*port_led, pin_led);
        // delay_ms_runtime(time_off);
        usart_send_string(">adc=:");
        usart_send_num(adc_cur, 4, 0);
        usart_send_string("\n");
        usart_send_string(">duty=:");
        usart_send_num(duty, 3, 0);
        usart_send_string("\n");
        _delay_ms(100);
        
         
    }
    
    return 0;
}




// User defined functions: /////////////////////////////////////////////////////

void delay_ms_runtime(uint16_t ms) {
    for (uint16_t i = 0; i < ms; i++) {
        _delay_ms(1); // This is a constant, so the compiler is happy
    }
}



//uint32_t linear_mapping(uint16_t adc_cur){
uint8_t linear_mapping(uint16_t adc_cur){

    // Linear mapping equation: slope = (y2 -y1)/(x2-x1)
    //
    // We have y as delay and x as distance.
    //
    // The slope is essentialy the gradient of the straight line, also
    // known as  the rate of change.   The rate of change  in a linear
    // equation is constant.  So when  we use linear mapping, we apply
    // a  constant rate  of change  between  the input  range and  the
    // output range.

    // To  get  slower  delays  you  need to  increase  the  slope  by
    // increasion PWM_PERIOD_MS.  So that you get bigger delays for more
    // quickly for the same range of ADC_MAX and ADC_MIN.
    uint32_t duty;
    uint32_t time_on_cur;
    
    // Clamp input so  it stays in sensor range.  If  you SWAP ADC_MIN
    // and ADC_MAX both  clamping statements are true  and adc_cur, is
    // always equal to  ADC_MAX.  Hence, you get a  constant value for
    // delay and thus beeping.
    //
    // eg ADC_MIN=150 ADC_MAX=100, swapped.
    if (adc_cur < ADC_MIN)
        adc_cur = ADC_MIN;

    if (adc_cur > ADC_MAX)
        adc_cur = ADC_MAX;

    // If we  SWAP PWM_PERIOD_MS and  TIME_ON_MIN we get a  -ve slope.
    // Hence, we  get faster  beeping at  longer distances  and slower
    // beeping at shorter distances.  Thus for  a small x delay is big
    // and for a bigger x delay is smaller.  Draw a -ve sloap graph to
    // see this.

    
    // Note: You lose precision with integer division because integers
    // cannot store  fractional parts. When two  integers are divided,
    // the result is truncated (the decimal part is discarded).

    // Orig code:
    // time_on_cur = TIME_ON_MIN
    //     + (uint32_t)(adc_cur - ADC_MIN)
    //     * ((PWM_PERIOD_MS - TIME_ON_MIN)) / ((ADC_MAX - ADC_MIN));
    
    // time_on_cur = PWM_PERIOD_MS 
    //     - (adc_cur - ADC_MIN) 
    //     * (PWM_PERIOD_MS - TIME_ON_MIN) / (ADC_MAX - ADC_MIN);

    // time_on_cur = PWM_PERIOD_MS 
    //     - ((uint32_t)(adc_cur - ADC_MIN) * (PWM_PERIOD_MS - TIME_ON_MIN) / (ADC_MAX - ADC_MIN));
    duty = (uint32_t)(adc_cur - ADC_MIN) * 255UL / (ADC_MAX - ADC_MIN);

    
    // usart_flush();
    // usart_send_string("linear_mapping: time_on_cur=");
    // usart_send_num(time_on_cur, 10, 0);
    // usart_send_byte('\n'); // CRLF or EOL
    // _delay_ms(500);
    
//    return time_on_cur;
    return (uint8_t)duty;
}




void usart_init(float baud_rate){

    // USE Claude  Haiku 4.5 version of  this fn as it  makes settings
    // more clear:
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
    
    // Enable transmitter and receiver
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    
    // Set frame format: 8 bit data, 1 stop bit, no parity
    UCSR0C = (0 << UMSEL01) | (0 << UMSEL00) |  // Async mode
             (0 << UPM01) | (0 << UPM00) |        // No parity
             (0 << USBS0) |                       // 1 stop bit  
             (1 << UCSZ01) | (1 << UCSZ00);       // 8-bit character


    // // MUX3-0, leave as default 0 => ADC0 pin will be used for voltage
    // // divider circuit.
    // ADMUX = 0; // Should be 0 by default though.

    // // Setting up the ADC:
    // bitSet(ADMUX, REFS0); // Vcc is Vref == 5V

    // // Prescaler of 128 bit, ADPS2=1 ADPS1=1 ADPS0=1
    // //ADCSRA |= 0b111 << 0;
    // ADCSRA = 0;
    // // Set prescaler to 128 (ADPS2=1, ADPS1=1, ADPS0=1)
    // ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    // // bitSet(ADCSRA, ADPS2);
    // // bitSet(ADCSRA, ADPS1);
    // // bitSet(ADCSRA, ADPS0);

    // bitSet(ADCSRA, ADIE); // ADC conversion complete interrupt enabled.
    // bitSet(ADCSRA, ADEN); // ADC ENable, ready for writing ADCcount to
    //                       // ADC register.
    
    // // note:  for ADC  and  future  modules, it  is  generally a  good
    // // routine to set enabling bit ADEN after all other configurations
    // // are completed, eg clock, reference, input, working mode etc.  }
}




void pwm1a_init(){
    // PWM init for PB1

        // PB1 = OC1A output
    bitSet(DDRB, pin_led);

    // Timer1 setup:
    // Fast PWM 8-bit
    // Non-inverting output on OC1A
    // Prescaler = 64
    //
    // PWM frequency = 16MHz / (64 * 256) ≈ 976 Hz

    TCCR1A = 0;
    TCCR1B = 0;

    bitSet(TCCR1A, COM1A1);   // Non-inverting PWM on OC1A
    bitSet(TCCR1A, WGM10);    // Fast PWM 8-bit
    bitSet(TCCR1B, WGM12);

    bitSet(TCCR1B, CS11);     // Prescaler 64
    bitSet(TCCR1B, CS10);

    OCR1A = 0;                // Start LED off
}





void adc_init(void)
{
    // ----------- ADC MUX setup -----------
    // REFS0 = 1 → Vref = AVcc (5V)
    // MUX[3:0] = 0001 → ADC1 (A1 / PC1)

    ADMUX = 0;
    bitSet(ADMUX, REFS0);   // AVcc reference
    bitSet(ADMUX, MUX0);    // Select ADC1

    // Disable digital input buffer on ADC1 (reduces noise/power)
    bitSet(DIDR0, ADC1D);

    // ----------- ADC Control setup -----------
    ADCSRA = 0;

    // Prescaler = 128 → ADC clock = 16MHz / 128 = 125kHz
    bitSet(ADCSRA, ADPS2);
    bitSet(ADCSRA, ADPS1);
    bitSet(ADCSRA, ADPS0);

    bitSet(ADCSRA, ADIE);   // Enable ADC interrupt
    bitSet(ADCSRA, ADEN);   // Enable ADC

    // Optional: start first conversion
    //bitSet(ADCSRA, ADSC);
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



void usart_read_string(char *ptr){

    // UDR0 is the  TX/RX data I/O register of the  arduino.  When the
    // flag RXC0  is set in  the UCR0A(Control and Status  Register A)
    // data is ready  to be read from the RX  buffer(UDR0).  When RXC0
    // is cleared in the UCR0A register the RX buffer(UDR0), is empty.

    char tmp;

    // Continue  reading bytes/characters  from the  RX buffer  of the
    // arduino MCU till EOL is found.   The data comes from the serial
    // monitor on my PC as ASCII characters.
    while (1) {

        // Block/wait till the user sends something.  Kai's code
        while (!bitCheck(UCSR0A, RXC0))
            ;            

        tmp = UDR0; // TX/RX I/O register, or data buffer.

        // Echo immediately to see if RXC0 os beomg set/
        // usart_send_byte(tmp);

        // Make sure your Serial Monitor has: Line Ending: CRLF (or at
        // least LF due to this code.).
        //
        // Some terminals send \r, some \n and some both.
        //
        // If this byte is found we are at the end of our string.
        if ( (tmp == '\r') || (tmp == '\n') ){
            // Terminate string with string terminator as per C
            // programming language spec.
            *ptr = '\0';
            //flag_read_done = 1;
            return;
        }
        else{
            // Otherwise save the character in the usart_buffer.
            *ptr++ = tmp;
        }

        
    } // end: while(1)
        
} // end: usart_read_string()



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

