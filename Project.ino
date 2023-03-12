#include <Keypad_MC17.h>
#include <Wire.h>
#include <SPI.h>
#include <TFT.h>
#include "Adafruit_MCP23017.h"
#include <Servo.h>

Servo serwo;

#define led1 0
#define led2 1
#define led3 2
#define led4 3
#define led5 4

#define PIR 5

#define I2CADDR 0x20
#define CS   10
#define DC   9
#define RESET  8

uint8_t addMCP = 1;

//Etapy
//0 - szukanie kodu
//1 - potencjometr
//2 - fotodioda
//3 - Znak
//4 - OPEN
//5 - End
int level = 0;

const int BORDER_VER = 4;
const int BORDER_HOR = 3;
const int LINESIZE = 22;

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {0, 1, 2, 3}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 5, 6, 7}; //connect to the column pinouts of the keypad

const int LEN = 4;
int passSize = 0;
char pass[LEN+1] = "****"; 
char goodPass[LEN+1] = "5712";

int passLvl = 0;
char goodPass1[LEN+1] = "5626";

int Rb = 0;
int Gb = 0;
int Bb = 0; 

int poten1 = 0;
int poten2 = 0;
int fotoLed = 0;
int thr;

unsigned long currentTime;
unsigned long memTime = 0;
unsigned long pirTime = 0;
unsigned long diffTime;

const int values[6] = {4, 2, 3, 0, 5};
const char dir[6] = "28468";
int lvl2Num = 0;

bool isDrawn = false;

int phase = 0;
int times[3] = {7, 1, 2};
int val = 0; 
int number=0;
bool active;

char morseCode[8][24]  = {
  {'1', '0', '1', '1', '1', '0', '1', '1', '1', '0', '1', '1', '1', '0', '1', '1', '1', 'S', 'S', 'S', 'S', 'S', 'S', 'E'}, //1
  {'1', '0', '1', '0', '1', '1', '1', '0', '1', '1', '1', '0', '1', '1', '1', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'E'}, //2
  {'1', '0', '1', '0', '1', '0', '1', '1', '1', '0', '1', '1', '1', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'E'}, //3
  {'1', '0', '1', '0', '1', '0', '1', '0', '1', '1', '1', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'E'}, //4
  {'1', '0', '1', '0', '1', '0', '1', '0', '1', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'E'}, //5
  {'1', '1', '1', '0', '1', '0', '1', '0', '1', '0', '1', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'E'}, //6
  {'1', '1', '1', '0', '1', '1', '1', '0', '1', '0', '1', '0', '1', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'E'}, //7
  {'1', '1', '1', '0', '1', '1', '1', '0', '1', '1', '1', '0', '1', '0', '1', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'E'} //8
};


Keypad_MC17 keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS, I2CADDR );
Adafruit_MCP23017 mcp;
TFT tft = TFT(CS, DC, RESET);

void setup(){
  Serial.begin(9600);
  while( !Serial ){/*wait*/}       // for USB serial switching boards (MKR)
  Wire.begin( );                   // now needed
  keypad.begin( );

  pinMode(A2, INPUT);
  
  mcp.begin(addMCP);

  mcp.pinMode(led1, OUTPUT);
  mcp.pinMode(led2, OUTPUT);
  mcp.pinMode(led3, OUTPUT);
  mcp.pinMode(led4, OUTPUT);
  mcp.pinMode(led5, OUTPUT);

  serwo.attach(3);
  serwo.write(90);

  pinMode(PIR, INPUT);
          
  tft.begin();
  tft.setRotation(2);
  tft.background(Rb, Gb, Bb);
  if(level==0)
  {
    setText(pass, 255, 255, 255, 0.5, 3, 4);
    setText("Find a way out", 255, 255, 255, 3, 2, 1);
  }

  for(int i=8; i<=15; i++)
  {
    mcp.pinMode(i, INPUT);
    mcp.pullUp(i, HIGH);
  }

  memTime = millis();
  thr = analogRead(A2);
}

void setText(char text[], int r, int g, int b, float pos, int line, int sizeTxt)
{
  tft.setTextSize(sizeTxt);
  tft.stroke(r,g,b);
  tft.text(text, BORDER_HOR + pos*sizeTxt*(5+1), BORDER_VER + LINESIZE * line);
}

void delText(char text[], float pos, int line, int sizeTxt)
{
  tft.setTextSize(sizeTxt);
  tft.stroke(Rb,Gb,Bb);
  tft.text(text, BORDER_HOR + pos*sizeTxt*(5+1), BORDER_VER + LINESIZE * line);  
}

void lightLEDS(int poten)
{
  switch(poten)
  {
    case 5:
    ledControl(1,1,1,1,1);
      break;
    case 4:
    ledControl(1,1,1,1,0);
      break;
    case 3: 
    ledControl(1,1,1,0,0);
      break;
    case 2:
    ledControl(1,1,0,0,0);
      break;
    case 1:
    ledControl(1,0,0,0,0);
      break;
    case 0:
    ledControl(0,0,0,0,0);
      break;    
  }
}

bool isCovered(int fotoLed)
{
  Serial.println(fotoLed);
  if(fotoLed>thr+50)
    return true;
  else
    return false;
}

bool allLight()
{
  if(mcp.digitalRead(led1) == 1 and mcp.digitalRead(led2) == 1 and mcp.digitalRead(led3) == 1 and mcp.digitalRead(led4) == 1 and mcp.digitalRead(led5) == 1)
    return 1;
  else
    return 0;
}

void ledControl(bool l1, bool l2, bool l3, bool l4, bool l5)
{
  mcp.digitalWrite(led1, l1);
  mcp.digitalWrite(led2, l2);
  mcp.digitalWrite(led3, l3);
  mcp.digitalWrite(led4, l4);
  mcp.digitalWrite(led5, l5);
}
  
void loop(){
 
  poten1 = analogRead(A3);
  poten1 = map(poten1, 0, 800, 0, 5);

  fotoLed = analogRead(A2);
  
  char key = keypad.getKey();
  //Serial.println(key);
  
  if(level==0)
  {
    lightLEDS(poten1);
    if (key)
    {
      delText(pass, 0.5, 3, 4);
      if(key=='#')
      {
        passSize = 0;
        strncpy(pass, "****", 4);
        setText(pass, 255, 255, 255, 0.5, 3, 4);
      }
      else if(key=='*')
      {
        if(strncmp(pass, goodPass, 4)==0)
        {
            setText(pass, 0, 255, 0, 0.5, 3, 4);
            delay(1000);
            tft.background(Rb,Gb,Bb);
            level+=1;
        }
        else
            setText(pass, 0, 0, 255, 0.5, 3, 4);
      }
      else if(passSize!=LEN)
      {
        pass[passSize] = key;
        passSize++;
        setText(pass, 255, 255, 255, 0.5, 3, 4);
      }
    }
  }
  else if(level==1)
  { 
    lightLEDS(poten1);
    Serial.println(poten1);     
    if(lvl2Num>0)
      setText("4", 0, 255, 0, 1, 1, 3);
    else
      setText("4", 255, 255, 255, 1, 1, 3);
           
    if(lvl2Num>1)
      setText("2", 0, 255, 0, 1, 2, 3);
    else
      setText("2", 255, 255, 255, 1, 2, 3);

    if(lvl2Num>2)
      setText("3", 0, 255, 0, 1, 3, 3);
    else
      setText("3", 255, 255, 255, 1, 3, 3);

    if(lvl2Num>3)
      setText("0", 0, 255, 0, 1, 4, 3);
    else
      setText("0", 255, 255, 255, 1, 4, 3);

    if(lvl2Num>4)
      setText("5", 0, 255, 0, 1, 5, 3);
    else
      setText("5", 255, 255, 255, 1, 5, 3);

    setText("2^", 0, 0, 255, 4, 1, 3);
    setText("8V", 0, 0, 255, 4, 2, 3);
    setText("4<", 0, 0, 255, 4, 3, 3);
    setText("6>", 0, 0, 255, 4, 4, 3);
    setText("8V", 0, 0, 255, 4, 5, 3);

    if(lvl2Num==5)
    {
      delay(1000);
      level++;
      tft.background(Rb,Gb,Bb);     
    }
      
    if (key)
    {
      if(key == dir[lvl2Num] and poten1 == values[lvl2Num])
        lvl2Num++;
    }


  }
  else if(level==2)
  {
 
    currentTime = millis();
    setText("EP EO PKK XNECDP", 255, 255, 255, 0, 1, 1);
    setText("YKRAN WJ AUA", 255, 255, 255, 0, 2, 1);
    setText("FQOP BKN W BAS OAYO", 255, 255, 255, 0, 5, 1);

    Serial.print(fotoLed);
    Serial.print("/");
    Serial.println(thr);
    

    tft.stroke(255,255,255);
    if(!isCovered(fotoLed))
    {
      tft.fill(127,127,127);
      memTime = currentTime;
    }
    else
    {
      tft.fill(0,0,0);
    }
      
    tft.circle(tft.width()/2, tft.height()/2, 10);

    tft.line(tft.width()/2-30, tft.height()/2, tft.width()/2, tft.height()/2-11);
    tft.line(tft.width()/2+30, tft.height()/2, tft.width()/2, tft.height()/2-11);

    tft.line(tft.width()/2-30, tft.height()/2, tft.width()/2, tft.height()/2+11);
    tft.line(tft.width()/2+30, tft.height()/2, tft.width()/2, tft.height()/2+11);

    if(currentTime-memTime>5000UL)
    {
      tft.fill(0, 255, 0);
      tft.circle(tft.width()/2, tft.height()/2, 10);      
      delay(1000);
      level+=1;
      tft.background(0,0,0);
    }
  }
  else if(level==3)
  {  
    if(!isDrawn)
    {
      memTime = 0;
      mcp.digitalWrite(led1, LOW);
      mcp.digitalWrite(led2, LOW);
      mcp.digitalWrite(led3, LOW); 
      mcp.digitalWrite(led4, LOW); 
      mcp.digitalWrite(led5, LOW);
  
      setText("S", 0, 0, 255, 0.1, 0.5, 20);
      
      tft.stroke(0, 255, 255);
      for(int i=0; i<20; i++)
        tft.line(0, tft.height()/2+i-16, tft.width(), tft.height()/2+i-16);
      tft.stroke(0, 255, 0);
      for(int i=0; i<20; i++)
      {
          tft.line(0, i+20, tft.width()/2, tft.height()/2+i+20);
          tft.line(tft.width(), i+20, tft.width()/2, tft.height()/2+i+20);
      }
      isDrawn=true;
    }
    if(phase!=3)
    {
      currentTime = millis();
      diffTime = currentTime - memTime;
      if(diffTime >= 200UL)
      {
        char sign = morseCode[number][val];
       Serial.println(String(number));
  
        if(sign == '1')
          {
            mcp.digitalWrite(phase,HIGH);
            mcp.digitalWrite(led4,HIGH);
            active=true;
            val++;
          }
        else if(sign == '0')
        {
          mcp.digitalWrite(phase,LOW);
          mcp.digitalWrite(led4,HIGH);
          active=true;
          val++;
        }
        else if(sign == 'S')
        {
          mcp.digitalWrite(phase,LOW);
          mcp.digitalWrite(led4,LOW);
          active=false;
          val++;
        }
        else if(sign == 'E')
        {
          mcp.digitalWrite(phase,LOW);
          mcp.digitalWrite(led4,LOW);
          active=false;
          val = 0;
          if(number==7)
            number=0;
          else
            number++;
        }
        memTime=currentTime;
      }
      if(digitalRead(PIR))
      {
        Serial.print(String(active) + ", " + String(times[phase]));
        if(active==true and number==times[phase])
        {
          mcp.digitalWrite(phase, HIGH);
          phase++;
        }
        else
        {
          phase=0;
          mcp.digitalWrite(led1, LOW);
          mcp.digitalWrite(led2, LOW);
          mcp.digitalWrite(led3, LOW);
        }
        delay(3000);
      }

    }
    else
    {
      mcp.digitalWrite(led1, HIGH);
      mcp.digitalWrite(led2, HIGH);
      mcp.digitalWrite(led3, HIGH);
      mcp.digitalWrite(led4, HIGH);
    }
      

    if(mcp.digitalRead(9)==LOW and mcp.digitalRead(12)==LOW and mcp.digitalRead(15)==LOW and
    mcp.digitalRead(8)==HIGH and mcp.digitalRead(10)==HIGH and mcp.digitalRead(11)==HIGH and mcp.digitalRead(13)==HIGH and mcp.digitalRead(14)==HIGH)
    {
      mcp.digitalWrite(led5,HIGH);
    }
    else
    {
      mcp.digitalWrite(led5, LOW);
    }

   if(allLight())
   {
    delay(1000);
    for(int i=0; i<6; i++)
    {
      ledControl(1,1,1,1,1);
      delay(100);
      ledControl(0,0,0,0,0);
      delay(100);
    }
    delay(500);
    level++;
    tft.background(0,0,0);
   }
  }
  else if(level==4)
  {   
    setText("OPEN", 255, 255, 255, 3, 3, 2);

    if(key)
    {
      if(key=='5' and (passLvl==0 or passLvl==3))
        passLvl++;
      else if(key=='6' and passLvl==1)
        passLvl++;
      else if(key=='2' and passLvl==2)
        passLvl++;
      else
        passLvl=0;
    }

    if(passLvl==4)
    {
      tft.background(0,0,0);
      level++;
    }
  }
  else if(level==5)
  {
    serwo.write(5);
    while(1)
    {
    
    int mov = 10;
    tft.background(255, 255, 255);
    tft.stroke(0,0,0);
    for(int i=0; i<60; i++)
      tft.line(tft.width()/2-60+i, tft.height()/2-i-mov, tft.width()/2+60-i, tft.height()/2-i-mov);
    
    tft.line(tft.width()/2, tft.height()/2-mov, tft.width()/2, tft.height()/2+75-mov);
    for (int i=0; i<20; i++)
    {
      tft.line(tft.width()/2+i, tft.height()/2-mov, tft.width()/2+i, tft.height()/2+75-mov);
      tft.line(tft.width()/2-i, tft.height()/2-mov, tft.width()/2-i, tft.height()/2+75-mov);
    }
    ledControl(1,0,1,0,1);
    delay(1000);

    tft.background(0, 0, 0);
    tft.stroke(255, 255, 255);
    for(int i=0; i<60; i++)
      tft.line(tft.width()/2-60+i, tft.height()/2-i-mov, tft.width()/2+60-i, tft.height()/2-i-mov);
    
    tft.line(tft.width()/2, tft.height()/2-mov, tft.width()/2, tft.height()/2+75-mov);
    for (int i=0; i<20; i++)
    {
      tft.line(tft.width()/2+i, tft.height()/2-mov, tft.width()/2+i, tft.height()/2+75-mov);
      tft.line(tft.width()/2-i, tft.height()/2-mov, tft.width()/2-i, tft.height()/2+75-mov);
    }    
    ledControl(0,1,0,1,0);
    delay(1000);    
    }
  }
}
