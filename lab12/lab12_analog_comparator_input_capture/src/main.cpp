#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

#define bitSet(reg, n)      ((reg) |=  (1 << (n)))
#define bitClear(reg, n)    ((reg) &= ~(1 << (n)))
#define bitToggle(reg, n)   ((reg) ^=  (1 << (n)))
#define bitCheck(reg, n)    ((reg) &   (1 << (n)))

// TC2 PWM output pin:
// OC2B = PD3 = Arduino D3
#define pin_pwm PD3

volatile float tLow = 0;
volatile float tHigh = 0;

float tFall = 0;
float tRise = 0;

volatile float Tov;
volatile float Tclk_tc1;

volatile uint16_t numOV = 0;
uint16_t icr1;

void set_tc1(void);
void set_tc2(void);



// Debugging USART functions
void usart_debugging(void);
void usart_init(float baud_rate);
void usart_flush(void);
void usart_read_string(char *ptr_to_str);
void usart_send_byte(unsigned char data);
void usart_send_string(const char *ptr_to_str);
void usart_send_num(float num, char num_int, char num_decimal);



ISR(TIMER1_OVF_vect)
{
    numOV++;
}



ISR(TIMER1_CAPT_vect)
{
    icr1 = ICR1 + 1;

    float tmp = numOV * Tov + icr1 * Tclk_tc1;

    // Note  this  is  for  TC2  Fast  PWM  connected  to  the  Analog
    // Comaparator(AC)  Negative  pin.   tFall   and  tLow  belong  to
    // ACO(Analog Comaparator Output) of the Input capture.
    
    if (!bitCheck(TCCR1B, ICES1))
    {
        // Falling edge of comparator output
        tFall = tmp;

        // AC(Analog  Comaparator) output  high  duration  for PWM  on
        // Positive AC Input.
        //
        //tHigh = tFall - tRise;
        
        // AC(Analog  Comaparator)  output  low duration  for  PWM  on
        // Negative AC Input.
        //
        tLow = tFall - tRise;
    }
    else
    {
        // Rising edge of comparator output
        tRise = tmp;

        // AC(Analog  Comaparator) output  low  duration  for PWM  on
        // Positive AC Input.
        //
        // tLow = tRise - tFall;
        
        // AC(Analog  Comaparator) output  high  duration  for PWM  on
        // Negative AC Input.
        //
        tHigh = tRise - tFall;
    }

    // Toggle between rising-edge and falling-edge capture next.
    bitToggle(TCCR1B, ICES1);
}



int main(void)
{
    // Analog Comparator input pins
    // PD6 = AIN0 = positive input
    // PD7 = AIN1 = negative input
    bitClear(DDRD, PD6);
    bitClear(DDRD, PD7);

    // TC2 PWM output pin
    // PD3 = OC2B = Arduino D3
    bitSet(DDRD, pin_pwm);

    // Connect Analog Comparator output to Timer1 Input Capture
    bitSet(ACSR, ACIC);

    usart_init(115200);

    set_tc2();
    set_tc1();

    sei();

    
    float dc_est = 0; // Estimated Duty Cycle
    float dc_true = (OCR2B + 1.0) / (OCR2A + 1.0) * 100.0; // True Duty Cycle

    float f_est = 0; // Estimated PWM Frequency
    float f_true = 16000000.0 / (1024.0 * (OCR2A + 1.0)); // True PWM frequency

    unsigned int cnt = 0;

    float tHigh_copy;
    float tLow_copy;

    while (1)
    {

        // We  disable interrupts  here as  we don't  want the  Timer1
        // Input Capture ISR to change them while we are using them.
        //
        cli();
        tHigh_copy = tHigh;
        tLow_copy = tLow;
        sei();
        
        
        if (tHigh_copy != 0 && tLow_copy != 0)
        {
            // Duty Cycle Estimated as a Percentage value.
            dc_est = tHigh_copy / (tHigh_copy + tLow_copy) * 100.0;

            /*
                One full period is still:
                    T = tHigh + tLow
                So estimated frequency is:
                    f = 1 / T
            */

            f_est = 1.0 / (tHigh_copy + tLow_copy);
        }

        // When dc_true  resets, that is OCR2B=OCR2A,  The dc_est lags
        // dc_true  initially.  This  is  because  dc_true changes  in
        // software immediately  but dc_est can only  change after the
        // hardware has  captured new  PWM edges  after this  point in
        // time.   Timer1 must  capture a  new rising  edge and  a new
        // falling edge in the ISR before dc_est changes.

        // Occasionally  a spike  appears  at the  duty cycle  restart
        // time.  This is  normal as sometimes tLow is  updated on one
        // captured edge and tHigh is updated on the opposite captured
        // edge inside.  Thus one value  can briefly belong to the new
        // PWM cycle  while the  other still  belongs to  the previous
        // cycle.

        // Plot true and estimated duty cycle together in Teleplot
        usart_send_string(">dc_true(%):");
        usart_send_num(dc_true, 3, 3);
        usart_send_byte('\n');

        usart_send_string(">dc_est(%):");
        usart_send_num(dc_est, 3, 3);
        usart_send_byte('\n');

        // Plot tHigh and tLow together in Teleplot
        usart_send_string(">tHigh:");
        usart_send_num((tHigh_copy*1000), 6, 3);
        //usart_send_num((tHigh*1000), 8, 1);
        usart_send_byte('\n');

        usart_send_string(">tLow:");
        usart_send_num((tLow_copy*1000), 6, 3);
        //usart_send_num((tLow*1000), 8, 1);
        usart_send_byte('\n');

        // Plot true and estimated frequency together in Teleplot
        usart_send_string(">f_true(Hz):");
        usart_send_num(f_true, 6, 3);
        usart_send_byte('\n');

        usart_send_string(">f_est(Hz):");
        usart_send_num(f_est, 6, 3);
        usart_send_byte('\n');

        _delay_ms(10);

        /*
            Duty cycle decrement every 1.5 seconds.

            The loop delays by about 10 ms.
                1.5 seconds = 1500 ms
                1500 ms / 10 ms = 150

            So after 150 loop iterations, decrement OCR2B.
        */

        cnt++;
        if (cnt >= 150)
        {
            cnt = 0;

            if (OCR2B >= 25)
            {
                OCR2B -=25;
            }
            else
            {
                OCR2B = OCR2A;
                //OCR2B = OCR2A * (90/100); // 90% duty cycle
            }

            dc_true = (OCR2B + 1.0) / (OCR2A + 1.0) * 100.0;
        }
    }

    return 0;
}



void set_tc2(void)
{

    // Converting set_tc0()  to set_tc2():
    //
    // Had to  change the registers names where there were 0's  to 2's.
    //
    // Changed  the bits for the  correct prescaler for this register.
    //
    // Calculation of OCR2A and OCR2B done.

    
    /*
        TC2 Fast PWM mode with TOP = OCR2A.

        Mode 7:

            WGM22 = 1
            WGM21 = 1
            WGM20 = 1

        PWM output is on OC2B = PD3 = Arduino D3.
    */

    TCCR2A = 0;
    TCCR2B = 0;

    TCCR2A |= 0b11;          // WGM21 = 1, WGM20 = 1
    bitSet(TCCR2B, WGM22);   // WGM22 = 1

    bitSet(TCCR2A, COM2B1);  // Non-inverting PWM on OC2B

    /*
        Choose PWM frequency = 62.5 Hz.

        f_PWM = F_CPU / (prescaler * (OCR2A + 1))
        OCR2A = F_CPU / (prescaler * f_PWM) - 1
        OCR2A = 16,000,000 / (1024 * 62.5) - 1
        OCR2A = 249

        TC2 is 8 bit register so OCR2A < 255 will work.
    */

    OCR2A = 249;

    /*
        OCR2B controls duty cycle.

        True duty cycle is approximately:
            duty = (OCR2B + 1) / (OCR2A + 1) * 100%
        Start near 100%.
    */

    OCR2B = OCR2A;

    /*
        TC2 prescaler = 1024.

        Important:
        For TC2, CS22:CS21:CS20 = 111 means prescaler 1024.

        This is different from TC0, where 101 meant prescaler 1024.
    */

    TCCR2B |= 0b111;
}



void set_tc1(void)
{
    /*
        TC1 is used for Input Capture.

        The  Analog Comparator  output(AC0)  is connected  to the  TC1
        Input Capture unit by setting ACIC in ACSR in main().  See pg8
        and  pg17 of  leacture  notes for  diagrams  and dataheet  for
        setting ACSR.
        
    */

    TCCR1A = 0;
    TCCR1B = 0;

    TCNT1 = 0;

    bitSet(TCCR1B, ICES1);   // Start by capturing rising edge
    bitSet(TCCR1B, ICNC1);   // Enable input capture noise cancellation

    bitSet(TIMSK1, TOIE1);   // Enable Timer1 overflow interrupt
    bitSet(TIMSK1, ICIE1);   // Enable Timer1 input capture interrupt

    /*
        Timer1 prescaler = 1024.

        For TC1:
        CS12:CS11:CS10 = 101 means prescaler 1024.
    */

    TCCR1B |= 0b101;

    float f = 16.0e6 / 1024.0; // Frequency hardware will run at

    // Tov = (number of ticks) * (period of one tick)
    //
    // Remember in this mode, Normal Mode,  tc1 is a 16bit register
    //
    Tov = 65536.0 / f;       // Timer1 overflow period in seconds
    Tclk_tc1 = 1.0 / f;      // Timer1 tick period in seconds
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
