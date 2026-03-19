#include <Arduino.h> // for debugging only
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>


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
void usart_send_string(char *ptr_to_str);
void print_math_fn_menu(void);



// Global definitions, hence available to all functions.
#define BUFFER_SIZE 50
 // all elements of usart_buffer is assgined to 0
char usart_buffer[BUFFER_SIZE] = {0};
char *ptr_to_usart_buffer = usart_buffer;
bool flag_read_done = 0;


int main( )
{
    usart_init_v2(19200);
    _delay_ms(100);
    usart_flush();

    while (1) {
        print_math_fn_menu();
        // usart_flush();
        // usart_read_string(ptr_to_usart_buffer);

        // usart_send_string(usart_buffer);
        // usart_send_byte('\n');
        //process_user_input();
        
        
    }
    
    return 0;
}

    
// User defined functions: /////////////////////////////////////////////////////

// checked
void usart_init(float baud_rate){
    float ubrr0 = (1.0e6 / baud_rate) - 1;
    int ubrr0a = (int)ubrr0;

    if (ubrr0 - ubrr0a >= 0.5) {
        ubrr0a = ubrr0a + 1;
    }

    UBRR0 = ubrr0a;
    bitSet(UCSR0B, TXEN0);
    UCSR0C |= 3<< UCSZ00;
    
} // end: usart_init()


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
        dummy = UDR0;
}


// checked
void usart_read_string(char *ptr){
    char tmp;
    while (1) {

        while (!bitCheck(UCSR0A, RXC0))
            ;

        tmp = UDR0;
        if ( (tmp == '\r') || (tmp == '\n') ){
            *ptr = '\0';
            flag_read_done = 1;
            return;
        }
        else{
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
void usart_send_string(char *pstr){

    // Send each  byte, one at a  time till the string  terminator \0,
    // that was read in via pstr.
    
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

    usart_send_string("Enter a number corresponding to the math fn you want to"
                      " perform on your number/s, x and/or y");
    usart_send_string("1. abs(x)");
    usart_send_string("2. fabs(x)");
    usart_send_string("3. pow(x, y)");
    usart_send_string("4. sqrd(x)");
    usart_send_string("5. exp(x)");
    usart_send_string("6. log(x)");
    usart_send_string("7. log10(x)");
    usart_send_string("8. ceil(x)");
    usart_send_string("9. floor(x)");
    
}



void process_user_input(void){
    int user_choice, first_number , second_number;
    char result[20];

    usart_read_string(ptr_to_usart_buffer);
    result[0] = *ptr_to_usart_buffer;
    result[1] = '\0';
    
    user_choice = atoi(result);
    usart_send_string("user_choice is: ");
    usart_send_num(num, 1, 0, result);
        


    if ( !((user_choice>=0) && (user_choice<=9)) )
        usart_send_string("Invalid selection. Try again")) {
        return;
    }

    usart_send_string("\nEnter first number: ");
    usart_flush();
    usart_read_string(ptr_to_usart_buffer);
    usart_send_byte('\n');
    first_number = ;

    if(user_choice == 3){
        usart_send_string("Enter second number: ");
        usart_flush();
        usart_read_string(ptr_to_usart_buffer);
        usart_send_string(usart_buffer);
        usart_send_byte('\n');
        second_number = ;
    }

    bool flag_print = 1;

    switch (user_choise) {
        case 1: {
            usart_send_string(*ptr_to_usart_buffer);            
            break;
        }
        default:
            break;
    }
    
        

} // end: process_user_input()

