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



State auto_traffic_lights(void);
State traffic_light_sensor_system(void);


// Debugging USART functions
void usart_debugging(void);
void usart_init(float baud_rate);
void usart_flush(void);
void usart_read_string(char *ptr_to_str);
void usart_send_byte(unsigned char data);
void usart_send_string(const char *ptr_to_str);
void usart_send_num(float num, char num_int, char num_decimal);
void print_usart_debugging_menu(void);


// Global definitions, hence available to all functions.
#define BUFFER_SIZE 50

// Our program buffer  that stores TX/RX data for the  Arduino that we
// want to transmit from MCU->PC or recive data from the PC->MCU.
//
// all elements of usart_buffer are initially assgined to 0.
char usart_buffer[BUFFER_SIZE] = {0};
char *ptr_to_usart_buffer = usart_buffer;

// Flag for  respective main  state machine  modes to  print debugging
// data as it is calculated.
char usart_debugging_mode = 0; // System starts with no debugging modes active.

// ISR for RX data flag
char flag_rx_done = 0;


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

    
    State state_current;
    state_current = AUTO_MODE;

    while (1) {

        usart_debugging();
        
        switch (state_current) {
            case AUTO_MODE: {

                state_current = auto_traffic_lights();
                
                state_current = traffic_light_sensor_system();


                if (usart_debugging_mode){

                    // Print out Distance_mm,  the current distance to
                    // the pedestrian.
                    
                }


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


State traffic_light_sensor_system(void){

    return AUTO_MODE;
}




State auto_traffic_lights(void){
    
    return AUTO_MODE;
}




// Debugging USART functions
void usart_debugging(void){

    print_usart_debugging_mode_menu();

    if (!flag_rx_done)
        return;

    if (flag_rx_done){
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
            ptr_to_usart_buffer = &usart_buffer[0];
            flag_rx_done = 0;
            usart_flush();
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


    } // end: if(flag_rx_done)
        
}



void print_usart_debugging_mode_menu(void){

    // Output the menu for debugging operations.
    
    usart_send_string("\nEnter a number corresponding to the usart debugging");
    usart_send_string(" mode required:\n");
    usart_send_string("0. Switch off debugging mode, no data from MCU to PC.\n");
    usart_send_string("1. Send pedestrian distance measurments to PC.\n");
    usart_send_string("2. Send ambient brightness measurments to PC.\n");
    usart_send_string("3. Both data in mode 1 and 2 sent to PC.\n");
    
}


void process_user_usart_debugging_mode_input(int8_t user_choice){


    switch (user_choice) {
        case 0: {
            usart_send_string("\nDebugging mode has been turned off.\n");
            usart_debugging_mode = 0;
            break;
        }
        case 1: {
            usart_send_string("\nSending pedestrian distance measurments");
            usart_send_string(" to PC via Teleplot\n");
            usart_debugging_mode_pedestrian_distance_values = 1;
            break;
        }
        case 2: {
            usart_send_string("\nSending ambient brightness measurements");
            usart_send_string(" to PC via Teleplot\n");
            usart_debugging_mode_ambient_brightness_values = 1;
            break;
        }
        case 3: {
            usart_send_string("\nSending both data measurements in mode 1 and 2");
            usart_send_string(" to PC via Teleplot\n");
            usart_debugging_mode_1_and_2_values = 1;
            break;
        }
        default:
            break;
            
    } // end: switch()

} // end: process_user_input()



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




void print_led_status_fn_menu(void){

    // Output the  menu form the  MCU, of  LED status required,  to PC
    // serial monitor.
    
    usart_send_string("\nEnter a number corresponding to the LED state you "
                      "want:\n");
    usart_send_string("0. Turn LED off\n");
    usart_send_string("1. Turn LED on\n");
}
