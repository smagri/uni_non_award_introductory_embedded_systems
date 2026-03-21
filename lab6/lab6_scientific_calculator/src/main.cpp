#define F_CPU 16000000UL



//#include <Arduino.h> // for debugging only
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>



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



// Function prototypes
void usart_init(float baud_rate);
void usart_init_v2(float baud_rate);
void usart_flush(void);
void usart_read_string(char *ptr_to_str);
void usart_send_byte(unsigned char data);
void usart_send_string(const char *ptr_to_str);
void usart_send_num(float num, char num_int, char num_decimal);
void print_math_fn_menu(void);
void process_user_input(int8_t user_choice);
    


// Global definitions, hence available to all functions.
#define BUFFER_SIZE 50

// Our program buffer  that stores TX/RX data for the  Arduino that we
// want to transmit from MCU->PC or recive data from the PC->MCU.
//
// all elements of usart_buffer is assgined to 0
char usart_buffer[BUFFER_SIZE] = {0};
char *ptr_to_usart_buffer = usart_buffer;
bool flag_read_done = 0;


int main(void){
    
    usart_init_v2(19200);
    //usart_init_v2(9600);
    //usart_init(9600);
    _delay_ms(100);


    uint8_t user_choice;

    
    while (1) {
        usart_flush(); // Clear out the RX register to prevent false reading.
        
        print_math_fn_menu();
        
        usart_flush(); // Clear out the RX register to prevent false reading.
        
        // Blocks waiting for user input. Value stored in USART RX
        // buffer.
        usart_read_string(ptr_to_usart_buffer);

        
        // Check if it's  exactly one digit 1–9 by  checking the ASCII
        // numbers.  Also,  reject non single character  entries, such
        // decimals and outside range ascii characters.
        if ( (usart_buffer[0]<'1')
             || (usart_buffer[0]>'9')
             || (usart_buffer[1]!='\0') ){
            usart_send_string("Invalid selection. Try again\n");
            continue;
        }
        else{
            // Convert input string into an integer.
            //
            // note: atoi() expects a null - terminated string.
            user_choice = atoi(usart_buffer);
            // usart_send_string("dbg: main(): User_choice is: ");
            // usart_send_num(user_choice, 1, 0);
            // usart_send_byte('\n'); // send EOL, CRLF
        }
        
        process_user_input(user_choice);
        
    } // end: while(1)
    
    return 0;
}

    
// User defined functions: /////////////////////////////////////////////////////

// checked
void usart_init(float baud_rate){
    // Fosc is 16MHz, so this would work too, I prefer to be more explicit.
    //float ubrr0 = (1.0e6 / baud_rate) - 1;
    
    float ubrr0 = ( (F_CPU / (16.0 * baud_rate)) - 1);
    int ubrr0a = (int)ubrr0;

    if (ubrr0 - ubrr0a >= 0.5) {
        ubrr0a = ubrr0a + 1;
    }

    UBRR0 = ubrr0a;
    bitSet(UCSR0B, TXEN0);
    UCSR0C |= 3<< UCSZ00;
    
} // end: usart_init()

// Perhaps safer according to claude haiku 4.5
// void usart_init(float baud_rate){
//     float ubrr0 = ( (F_CPU / (16.0 * baud_rate)) - 1);
//     int ubrr0a = (int)ubrr0;

//     if (ubrr0 - ubrr0a >= 0.5) {
//         ubrr0a = ubrr0a + 1;
//     }

//     // Set baud rate
//     UBRR0 = ubrr0a;
    
//     // Enable transmitter and receiver
//     UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    
//     // Set frame format: 8 bit data, 1 stop bit, no parity
//     UCSR0C = (0 << UMSEL01) | (0 << UMSEL00) |  // Async mode
//              (0 << UPM01) | (0 << UPM00) |        // No parity
//              (0 << USBS0) |                       // 1 stop bit  
//              (1 << UCSZ01) | (1 << UCSZ00);       // 8-bit character
// }


// checked // compiled
void usart_init_v2(float baud_rate){
    usart_init(baud_rate);

    // Enable bit RXEN0 in USCR0B register.
    bitSet(UCSR0B, RXEN0);
 }


// checked
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


// checked
void usart_read_string(char *ptr){

    // UDR0 is the RX data I/O register of the arduino.  When the flag
    // RXC0 is set in the UCR0A(Control and Status Register A) data is
    // ready  to be  read  from  the RX  buffer(UDR0).   When RXC0  is
    // cleared in the UCR0A register the RX buffer(UDR0), is empty.

    char tmp;

    // Continue  reading bytes/characters  from the  RX buffer  of the
    // arduino MCU till EOL is found.   The data comes from the serial
    // monitor on my PC.
    while (1) {

        // Block/wait till the user sends something.  Kai's code
        while (!bitCheck(UCSR0A, RXC0))
            ;            

        // Debugging code.
        // while (1) {
        //     if (bitCheck(UCSR0A, RXC0)) {
        //         char c  = UDR0;
        //         usart_send_byte(c); // echo immediately
        //     }
        // }
        
        tmp = UDR0; // RX I/O register, or data buffer.

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



// checked
void usart_send_byte(unsigned char data){
    while (!bitCheck(UCSR0A, UDRE0))
        ;
    UDR0 = data;
}


// checked

// check if const is really needed, cgpt says c++ compiler is strictor
// than c compiler.
void usart_send_string(const char *pstr){
    // note: const means a read-only-string, prevents accidentally
    // modifying string literals(which causes crashes).

    // Send each byte, one at a time, till the string terminator \0 is
    // sent.
    while (*pstr != '\0') {
        usart_send_byte(*pstr);
        pstr++;
    }
}


// checked
void usart_send_num(float num, char num_int, char num_decimal){
    char str[20];
    if (num_decimal == 0)
        dtostrf(num, num_int, num_decimal, str);
    else
        dtostrf(num, (num_int+num_decimal+1), num_decimal, str);
    str[num_int+num_decimal+1] = '\0';
    usart_send_string(str);
        
}



void print_math_fn_menu(void){

    // Output the menu, of mathamatical functions that can be
    // performed, to the serial monitor.
    
    usart_send_string("\nEnter a number corresponding to the math fn you want");
    usart_send_string(" to perform on your number/s, x and/or y.\n");
    usart_send_string("1. abs(x), where x is an integer.\n");
    usart_send_string("2. fabs(x), where x can be a float.\n");
    usart_send_string("3. pow(x, y), x^y.\n");
    usart_send_string("4. sqrt(x), square root of x.\n");
    usart_send_string("5. exp(x), e^x, e approx =2.71828, Euler's number.\n");
    usart_send_string("6. log(x), natural logarithm, what to raise e to, to get x\n");
    usart_send_string("7. log10(x), what do raise 10 to, to get x.\n");
    usart_send_string("8. ceil(x), rounds UP to the nearest integer.\n");
    usart_send_string("9. floor(x), rounds DOWN to the nearest integer\n");
    
}



void process_user_input(int8_t user_choice){

    // Numbers entered by user after user choosing a mathematical
    // operation to perform on them.
    float first_number , second_number;

    // Converted number after mathematical operation performed.  Kept
    // as a float as usart_send_num(float num....) expects this.
    //
    // usart_send_num(float num, char num_total_chars, char num_decimal_places)
    float number;

    uint8_t max_field_width_num = 10;
    uint8_t num_decimal_places = 6;
        

    usart_send_string("\nEnter first number: ");
    usart_flush(); // Get junk out of MCU RX buffer
    usart_read_string(ptr_to_usart_buffer);
    first_number = atoi(ptr_to_usart_buffer);
    //usart_send_string("dbg: process_user_input(): First number: ");
    //usart_send_num(first_number, 1, 0);
    //usart_send_byte('\n');
    

    if(user_choice == 3){
        usart_send_string("Enter second number: ");
        usart_flush();
        usart_read_string(ptr_to_usart_buffer);
        second_number = atoi(ptr_to_usart_buffer);
    }



    switch (user_choice) {
        case 1: {
            usart_send_string("Absolute value, abs(x), is = ");
            number = fabs(first_number);
            //usart_send_string("Absolute value, abs(");
            //usart_send_num(first_number, max_field_width_num, num_decimal_places);
            //number = abs(first_number);
            //usart_send_string(") is = ");
            //usart_send_num(number, max_field_width_num, num_decimal_places);
            //usart_send_byte('\n'); // CRLF or EOL
            break;
        }
        case 2: {
            usart_send_string("Absolute value, fabs(x), is = ");
            number = fabs(first_number);
            break;
        }
        case 3: {
            usart_send_string("x^y, pow(x,y), is = ");
            number = pow(first_number, second_number);
            break;
        }
        case 4: {
            usart_send_string("Square Root, sqrt(x), is = ");
            number = sqrt(first_number);
            break;
        }
        case 5: {
            usart_send_string("Exponential, exp(x), is = ");
            number = exp(first_number);
            break;
        }
        case 6: {
            usart_send_string("Natural Logarithm, log(x), is = ");
            number = log(first_number);
            break;
        }
        case 7: {
            usart_send_string("Common Logarithm, log10(x), is = ");
            number = log10(first_number);
            break;
        }
        case 8: {
            usart_send_string("Ceiling, ceil(x), is = ");
            number = ceil(first_number);
            break;
        }
        case 9: {
            usart_send_string("Floor value, floor(x), is = ");
            number = floor(first_number);
            break;
        }
        default:
            break;
            
    } // end: switch()

    usart_send_num(number, max_field_width_num, num_decimal_places);
    usart_send_byte('\n'); // CRLF or EOL
    
        

} // end: process_user_input()

