// Debugging USART functions
void usart_debugging(void);
void usart_init(float baud_rate);
void usart_flush(void);
void usart_read_string(char *ptr_to_str);
void usart_send_byte(unsigned char data);
void usart_send_string(const char *ptr_to_str);
void usart_send_num(float num, char num_int, char num_decimal);

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
