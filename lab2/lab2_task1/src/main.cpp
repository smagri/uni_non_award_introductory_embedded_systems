#include <Arduino.h>
#include <util/delay.h>

int main(void)
{
  // Page 1
  init();

  Serial.begin(9600);
  _delay_ms(1000);

  int a = 100;
  int b = 3;
  int c = 0;

  Serial.print("c=");
  Serial.println(c);
  Serial.print("a+b=");
  Serial.println(a+b);

  Serial.print("a-b=");
  Serial.println(a-b);

  Serial.print("a*b=");
  Serial.println(a*b);

  Serial.print("a/b=");
  Serial.println(a/b);

  // Page 2
  float d = a / b;
  Serial.print("a/b=");
  Serial.println("d");

  d = (float)a / b;
  Serial.print("a/b=");
  Serial.println(d);

  d = a % b;
  Serial.print("a%b=");
  Serial.println(d);


  
  // Extension
  Serial.println("EXTENSION:----------------------------------------------------");
  Serial.println("unsigned char:");
  unsigned char auc=100;
  unsigned char buc=3;
  unsigned char cuc=0;
  Serial.print("cuc=");
  Serial.print(cuc);
  Serial.print("auc+buc=");
  Serial.println(auc+buc);
  Serial.print("auc-buc=");
  Serial.println(auc-buc);
  Serial.print("auc*buc=");
  Serial.println(auc*buc);
  Serial.print("auc/buc=");
  Serial.println(auc/buc);
  Serial.print("auc%buc=");
  Serial.println(auc%buc);



  Serial.println("char:");
  char ac=100;
  char bc=3;
  Serial.print("ac+bc=");
  Serial.println(ac+bc);
  Serial.print("ac-bc=");
  Serial.println(ac-bc);
  Serial.print("ac*bc=");
  Serial.println(ac*bc);
  Serial.print("ac/bc=");
  Serial.println(ac/bc);
  Serial.print("ac%bc=");
  Serial.println(ac%bc);


  
  Serial.println("unsigned int:");
  unsigned int aui=100;
  unsigned int bui=3;
  unsigned int cui=0;
  Serial.print("cui=");
  Serial.println(cui);
  Serial.print("aui+bui=");
  Serial.println(aui+bui);
  Serial.print("aui-bui=");
  Serial.println(aui+bui);
  Serial.print("aui*bui=");
  Serial.println(aui*bui);
  Serial.print("aui/bui=");
  Serial.println(aui/bui);
  Serial.print("aui%bui=");
  Serial.println(aui%bui);



  Serial.println("int:");
  int ai=100;
  int bi=3;
  int ci=0;
  Serial.print("ci=");
  Serial.println(ci);
  Serial.print("ai+bi=");
  Serial.println(ai+bi);
  Serial.print("ai-bi=");
  Serial.println(ai-bi);
  Serial.print("ai*bi=");
  Serial.println(ai*bi);
  Serial.print("ai/bi=");
  Serial.println(ai/bi);
  Serial.print("ai%bi=");
  Serial.println(ai%bi);



  Serial.println("unsigned long:");
  unsigned long aul=100;
  unsigned long bul=3;
  unsigned long cul=0;
  Serial.print("cul=");
  Serial.println(cul);
  Serial.print("aul+bul=");
  Serial.println(aul+bul);
  Serial.print("aul-bul=");
  Serial.println(aul-bul);
  Serial.print("aul*bul=");
  Serial.println(aul*bul);
  Serial.print("aul/bul=");
  Serial.println(aul/bul);
  Serial.print("aul%bul=");
  Serial.println(aul%bul);



  Serial.println("long:");
  long al=100;
  long bl=3;
  long cl=0;
  Serial.print("cl=");
  Serial.println(cl);
  Serial.print("al+bl=");
  Serial.println(al+bl);
  Serial.print("al-bl=");
  Serial.println(al-bl);
  Serial.print("al*bl=");
  Serial.println(al*bl);
  Serial.print("al/bl=");
  Serial.println(al/bl);
  Serial.print("al%bl=");
  Serial.println(al%bl);



  Serial.println("float:");
  float af=100;
  float bf=3;
  float cf=0;
  Serial.print("cf=");
  Serial.println(cf);
  Serial.print("af+bf=");
  Serial.println(af+bf);
  Serial.print("af-bf=");
  Serial.println(af-bf);
  Serial.print("af*bf=");
  Serial.println(af*bf);
  Serial.print("af/bf=");
  Serial.println(af/bf);
  Serial.println("af%bf=");
//  Serial.println(af%bf);



  Serial.println("double:");
  double ad=100;
  double bd=3;
  double cd=0;
  Serial.print("cd=");
  Serial.println(cd);
  Serial.print("ad+bd=");
  Serial.println(ad+bd);
  Serial.print("ad-bd=");
  Serial.println(ad-bd);
  Serial.print("ad*bd=");
  Serial.println(ad*bd);
  Serial.print("ad/bd=");
  Serial.println(ad/bd);
//  Serial.print("ad%bd=");
//  Serial.println(ad%bd);
  
    while (1) {
        // Stay Alive
        _delay_ms(10);
    }
    
    
  return 0;
}

