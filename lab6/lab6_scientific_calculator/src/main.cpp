#define F_CPU 16000000UL



//#include <Arduino.h> // for debugging only
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
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
    
    //usart_init_v2(19200);
    //usart_init_v2(9600);
    usart_init(9600);
    _delay_ms(100);
    usart_flush(); // Clears anything in the RX buffer

    uint8_t user_choice;

    // Debugging code.
    // while (1) {
    //     if (bitCheck(UCSR0A, RXC0)) {
    //         char c  UDR0;
    //         usart_send_byte(c); // echo immediately
    //     }
    // }
        
    while (1) {
        print_math_fn_menu();
        //usart_flush(); // Clears any recived data, ie throws away user input.

        //usart_send_string(usart_buffer);
        //usart_send_byte('\n');

        // Blocks waiting for user input. Value stored in USART buffer.
        usart_read_string(usart_buffer);

        if (flag_read_done){
            // Echo input to the screen.
            usart_send_string(usart_buffer);
            usart_send_byte('\n');
            flag_read_done = 0;
        }
        usart_flush();
        _delay_ms(10);

        // Convert input string into an integer.
        //
        // note: atoi() expects a null - terminated string.
        user_choice = atoi(usart_buffer);
        usart_send_string("User_choice is: ");
        usart_send_num(user_choice, 3, 2);
        usart_send_byte('\n');
        
        //process_user_input(user_choice);
        
    } // end: while(1)
    
    return 0;
}

    
// User defined functions: /////////////////////////////////////////////////////

// checked
// void usart_init(float baud_rate){
//     // Fosc is 16MHz
//     //float ubrr0 = (1.0e6 / baud_rate) - 1;
//     float ubrr0 = ( (F_CPU / (16.0 * baud_rate)) - 1);
//     int ubrr0a = (int)ubrr0;

//     if (ubrr0 - ubrr0a >= 0.5) {
//         ubrr0a = ubrr0a + 1;
//     }

//     UBRR0 = ubrr0a;
//     bitSet(UCSR0B, TXEN0);
//     UCSR0C |= 3<< UCSZ00;
    
// } // end: usart_init()
void usart_init(float baud_rate){
    float ubrr0 = ( (F_CPU / (16.0 * baud_rate)) - 1);
    int ubrr0a = (int)ubrr0;

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


// checked // compiled
void usart_init_v2(float baud_rate){
    usart_init(baud_rate);

    // Enable bit RXEN0 in USCR0B register.
    bitSet(UCSR0B, RXEN0);
 }


// checked
void usart_flush(void){
    char dummy;
    
    while (bitCheck(UCSR0A, RXC0))
        // This clears the RX buffer // TODO: 
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
        //         = char c  UDR0;
        //         usart_send_byte(c); // echo immediately
        //     }
        // }
        
        tmp = UDR0; // RX I/O register, or data buffer.

        // Echo immediately to see if TX is broken(clock/baud issue).
        usart_send_byte(tmp);
        
        // Some terminals send \r, some \n and some both.
        //
        // If this byte is found we are at the end of our string.
        if ( (tmp == '\r') || (tmp == '\n') ){
            *ptr = '\0';
            flag_read_done = 1;
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


// checked // TODO: check if const is really needed, cgpt says c++
// compiler is strictor than c compiler
void usart_send_string(const char *pstr){
    // note: const means a read-only-string, prevents accidentally
    // modifying string literals(which cases crashes)

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

    usart_send_string("Enter a number corresponding to the math fn you want to");
    usart_send_string(" perform on your number/s, x and/or y.\n");
    usart_send_string("1. abs(x)\n");
    usart_send_string("2. fabs(x)\n");
    usart_send_string("3. pow(x, y)\n");
    usart_send_string("4. sqrd(x)\n");
    usart_send_string("5. exp(x)\n");
    usart_send_string("6. log(x)\n");
    usart_send_string("7. log10(x)\n");
    usart_send_string("8. ceil(x)\n");
    usart_send_string("9. floor(x)\n");
    
}



void process_user_input(int8_t user_choice){
//    int user_choice, first_number , second_number;
    int first_number , second_number;
    //char result[20];
    

    // // Blocks waiting for user input.
    // usart_read_string(ptr_to_usart_buffer);
    // //result[0] = *ptr_to_usart_buffer;
    // //result[1] = '\0';

    // // atoi() expects a null - terminated string.
    // user_choice = atoi(ptr_to_usart_buffer);
    // usart_send_string("User_choice is: ");
    // usart_send_num(user_choice, 3, 2);
        


    if ( !((user_choice>=0) && (user_choice<=9)) ){
        usart_send_string("Invalid selection. Try again");
        return;
    }

    usart_send_string("\nEnter first number: ");
    usart_flush();
    usart_read_string(ptr_to_usart_buffer);
    // dbg: display back to PC serial port monitor
    usart_send_string(usart_buffer);
    usart_send_byte('\n'); // NOTE: must send a character constant not a string
        
    first_number = atoi(ptr_to_usart_buffer);
    usart_send_string("dbg: process_user_input(): First number: ");
    usart_send_num(first_number, 3, 0);
    usart_send_byte('\n');

    if(user_choice == 3){
        usart_send_string("Enter second number: ");
        usart_flush();
        usart_read_string(ptr_to_usart_buffer);
        usart_send_string(usart_buffer);
        usart_send_byte('\n');

        second_number = atoi(ptr_to_usart_buffer);
        usart_send_string("dbg: process_user_input(): Second number: ");
        usart_send_num(second_number, 3, 0);
        usart_send_byte('\n');
    }

    //bool flag_print = 1;

    switch (user_choice) {
        case 1: {
            usart_send_string("dbg: process_user_input(): Choise made 1: \n");
            usart_send_string("dbg: process_user_input() abs()=");
            usart_send_num(abs(first_number), 5, 2);
            usart_send_byte('\n');
            break;
        }
        default:
            break;
    }
    
        

} // end: process_user_input()

