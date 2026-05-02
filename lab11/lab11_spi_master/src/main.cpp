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


// Start and stop tc2 immediately
#define start_tc2   (TCCR2B |= 0b101)   // prescaler = 128
#define stop_tc2    (TCCR2B &= ~0b111)


// Timer/Counter overflow interrupt enable and disable
#define enable_tc2_interrupt    (bitSet(TIMSK2, TOIE2))
#define disable_tc2_interrupt   (bitClear(TIMSK2, TOIE2))

#define prescalerTC2 128

// Define feasible ranges for linear mapping:
//
// All values determined mid-morning to mid-day, may be readjusted at
// night time.
//
// A bit of buffer, say 50 levels, was added to ADC_MIN/MAX as we were
// hitting limits to often.
#define ADC_MIN 500 // room temperature at 26.4 degrees
#define ADC_MAX 650 // when warmed by my fingers holding the thermistor tightly

#define TEMPERATURE_MIN 26.4 // degrees C, room temperature

// degrees C, when holding thermistor tightly with my fingers, an estimate
#define TEMPERATURE_MAX 33.0

// LED pin connection on atmega328p.
#define pin_adc PC3

#define BUFFER_SIZE 50


// Total _time_ taken for the counter to count from 0->TOP-1 again.
// For the fast PWM TOP signal.
float timer2_overflow_time_us;

// T=1/F, the perioud of each clock tick.  Where is F/prescaler.
//float clock_tc0;
float period_of_tick;


// Our program buffer  that stores TX/RX data for the  Arduino that we
// want to transmit from MCU->PC or recive data from the PC->MCU.
//
// all elements of usart_buffer is assgined to 0
char usart_buffer[BUFFER_SIZE] = {0};
char *ptr_to_usart_buffer = usart_buffer;



// Kai's original code, causes my_delay_us() to hang as num_timer2_overflow overflows:
//volatile unsigned int num_timer2_overflow = 0;
//volatile unsigned int num_timer2_overflow_max_sonar = 0;
//
// Avoids counter overflow in my_delay_us() for 1s delay.
volatile uint32_t  num_timer2_overflows = 0;

//int adc = 0;
volatile uint16_t adc = 0;




void usart_init(float baud_rate);
void usart_flush(void);
void usart_read_string(char *ptr_to_str);
void usart_send_byte(unsigned char data);
void usart_send_string(const char *ptr_to_str);
void usart_send_num(float num, char num_int, char num_decimal);

void adc_init(void);

float linear_mapping(float x, float x1, float x2, float y1, float y2);


// Timer overflow,  number of overflows  for the current fast  PWM TOP
// signal.  A flag gets set in hardware then the ISR gets called.
//
ISR(TIMER2_OVF_vect)
{
    num_timer2_overflows++;
}



ISR(ADC_vect){
        
    //usart_send_string("dbg: in ISR");
    
    // Triggers when interrupt conversion is complete.
    adc = ADC; // Read ADC data register.
}


void config_tc2(void){
}



int main(void){
    
    // Interface the DTH11 to the atmega328p ports.
    volatile uint8_t *ddr_reg = &DDRC; // used Data Direction Register B
    volatile uint8_t *port_reg = &PORTC; // use port B register
    volatile uint8_t *port_inputs_reg = &PINC;

    // Remember  by  default  all  pins  are  inputs  after  a  reset.
    // However, setting them explicitly makes code more readable.
    
    // Configure A2D  port to  input, internal  pullup disabled  as it
    // would  effect  the  thermistor  voltage divider  and  make  ADC
    // readings wrong.
    bitClear(*ddr_reg, pin_adc);

    // Note, not the normal input configuration:
    // bitSet(*port_reg, pin_adc); // enable the pull up
    //
    // For later, this is normally  how you access its value, however,
    // we are using ISR here.
    //
    // adc_value = bitRead(*port_inputs_reg, pin_adc);
    bitClear(*port_reg, pin_adc); // internal pullup disabled


    // For linear mapping ADC  values to temperatures.  Thermistor ADC
    // value  linearly   mapped  to  temperature.   Thermistor   is  a
    // temperature sensitive resistor.
    //
    float temperature = 0;
 
    float adc_min = ADC_MIN;
    float adc_max = ADC_MAX;
    
    float temperature_min = TEMPERATURE_MIN; 
    float temperature_max = TEMPERATURE_MAX;

    uint8_t spi_byte_xfer_temperature;


    
    usart_init(9600); // Initialise Regiters
    adc_init(); // Configure ADC 

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
        // once everytime ADSC is set.
        bitSet(ADCSRA, ADSC);

        // Get ADC register value ready for plotting in vscode Telepot.
        usart_send_string(">adc:");
        // ADC register value 4 places no decimal place, as is integer.
        usart_send_num(adc, 4, 0);
        // Telepot value terminating character.
        usart_send_string("\n");
        if (adc >= adc_max) {
            usart_send_string(">debug:CLAMP_MAX\n");
        }
        else if (adc <= adc_min){
            usart_send_string(">debug:CLAMP_MIN\n");
        }
        _delay_ms(100);


        // Linearly map the ADC values to a floating point temperature
        // values for local Telepot display.
        temperature = linear_mapping(adc, adc_min, adc_max,
                                     temperature_min, temperature_max);
        // Get temperature value ready for plotting in vscode Telepot.
        usart_send_string(">temperature:");
        // Temperature value as a decimal value
        usart_send_num(temperature, 4, 2);
        // Telepot value terminating character.
        usart_send_string("\n");
        _delay_ms(100);


        // Convert floating  point temperature  values to  byte values
        // for  SPI transmission  to slave.   This is  given that  SPI
        // requires reciving bytes as it does this most cleanly.  Note
        // for mapping, one byte has a value from 0-255.
        //
        spi_byte_xfer_temperature = linear_mapping(temperature,
                                                   temperature_min, temperature_max,
                                                   0.0f, 255.0f);

        // Add 0.5 before casting so we round to nearest byte.
        spi_byte_xfer_temperature = ((uint8_t)(spi_byte_xfer_temperature + 0.5f));



    return 0;
}



///////////////////////////////////////////////////////////////////////////////
//                           User defined functions                          //
///////////////////////////////////////////////////////////////////////////////



void my_delay_us(unsigned long x){

    // Delays, determined with a hardware timer/counter for x us.

    
    // NOTE: that a float assigned  to an integer returns the interger
    // value with no decimal pont values.
        
    // Number of complete Timer2 overflow periods needed.
    unsigned long num_complete_overflows = ( (float)x / timer2_overflow_time_us );

    // Remaining time after complete overflows, in microseconds.
    float remainder_time_us =
        (  (float)x - ((float)num_complete_overflows*timer2_overflow_time_us) );

    // Convert remaining time into Timer2 ticks.
    unsigned char remainder_time_ticks =
        ( remainder_time_us / (period_of_tick * 1.0e6) );

    //  Delay for complete number of overflows of Timer2
    if (num_complete_overflows > 0){
        num_timer2_overflows = 0;
        TCNT2 = 0;

        // // TODO: Test  Clears  and pending  overflow flag  so the  ISR doen't  run
        // immediately and have the delay overflow too short.
        //TIFR2 = (1 << TOV2);

        enable_tc2_interrupt;
        while (num_timer2_overflows < num_complete_overflows);
        disable_tc2_interrupt;
    }

    // Delay for remainder number of ticks
    TCNT2 = 0;
    while (TCNT2 < remainder_time_ticks);
}



float linear_mapping(float x, float x1, float x2, float y1, float y2){

    // Linear mapping equation: slope = (y2 -y1)/(x2-x1)
    //
    // We have y vs x 
    //
    // The slope is essentialy the gradient of the straight line, also
    // known as  the rate of change.   The rate of change  in a linear
    // equation is constant.  So when  we use linear mapping, we apply
    // a  constant rate  of change  between  the input  range and  the
    // output range.

    // To get greater y more quickly you need to increase the slope by
    // increasion y2.  

    // By swapping y1 and y2 the slope becomes -ve(as y2>y1) so we get
    // a swapped around effect, or -ve linear mapping.


    // Clamp input so it stays in  sensor range.
    //
    // 1. Clamp the input to the lower bound
    if (x <= x1) return y1;
    
    // 2. Clamp the input to the upper bound
    if (x >= x2) return y2;

    
    // Note: You lose precision with integer division because integers
    // cannot store  fractional parts. When two  integers are divided,
    // the result is truncated (the decimal part is discarded).
    
    // 3. Perform Linear Interpolation (Lerp)
    // Formula: y = y1 + (x - x1) * (slope)
    // where slope = (y2 - y1) / (x2 - x1)
    return y1 + (x - x1) * (y2 - y1) / (x2 - x1);
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

}



void  adc_init(void){

    // adc_channel = last_digit_of_student_id % number_of_available_channels
    //               3 % 6
    // so to be used as per lab task specification is adc_channel = 3
    
    // MUX3-0, pin will be used for voltage divider circuit.
    ADMUX = 0; // Should be 0 by default though.
    ADMUX |= (1 << MUX1) | (1 << MUX0);

    // Setting up the ADC:
    bitSet(ADMUX, REFS0); // Vcc is Vref == 5V

    // Prescaler of 128 bit, ADPS2=1 ADPS1=1 ADPS0=1
    //ADCSRA |= 0b111 << 0;
    ADCSRA = 0;
    // Set prescaler to 128 (ADPS2=1, ADPS1=1, ADPS0=1)
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    // bitSet(ADCSRA, ADPS2);
    // bitSet(ADCSRA, ADPS1);
    // bitSet(ADCSRA, ADPS0);

    bitSet(ADCSRA, ADIE); // ADC conversion complete interrupt enabled.
    bitSet(ADCSRA, ADEN); // ADC ENable, ready for writing ADCcount to
                          // ADC register.
    
    // note:  for ADC  and  future  modules, it  is  generally a  good
    // routine to set enabling bit ADEN after all other configurations
    // are completed, eg clock, reference, input, working mode etc.  }
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

