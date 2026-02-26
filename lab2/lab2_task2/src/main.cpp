#include <Arduino.h>
#include <util/delay.h>


int main(void)
{
    init();

    Serial.begin(9600);
    _delay_ms(1000);
        
    // Arithmetic operators
    unsigned int a = 100;
    unsigned int b = 10;
    Serial.print("a=");
    Serial.println(a);
    Serial.print("b=");
    Serial.println(b);
    
    Serial.print("a+b=");
    Serial.println(a+b);
    Serial.print("a-b=");
    Serial.println(a-b);
    Serial.print("a*b=");
    Serial.println(a*b);
    Serial.print("a/b=");
    Serial.println(a/b);
    Serial.print("a%b=");
    Serial.println(a%b);



    // Comparison Operators
    Serial.print("a==b=");
    Serial.println(a==b);
    Serial.print("a<=b=");
    Serial.println(a<=b);
    Serial.print("a>=b=");
    Serial.println(a>=b);
    Serial.print("a<b=");
    Serial.println(a<b);
    Serial.print("a>b=");
    Serial.println(a>b);
    Serial.print("a!=b=");
    Serial.println(a!=b);



    // Logical Operators
    Serial.print("a&&b=");
    Serial.println(a&&b);
    Serial.print("a||b=");
    Serial.println(a||b);
    Serial.print("!a=");
    Serial.println(!a);

        

    // Arethmetic Operators
    Serial.print("a&b=");
    Serial.println(a&b);
    Serial.print("a|b=");
    Serial.println(a|b);
    Serial.print("a^b=");
    Serial.println(a^b);
    Serial.print("a<<3=");
    Serial.println(a<<3);
    Serial.print("a>>3=");
    Serial.println(a>>3);

    


    while (1) {
        // stay alive
        _delay_ms(100);
    }

}
