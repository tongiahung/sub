#include <Wire.h>
#include <SHT1x.h>
#include <avr/interrupt.h>
#include <EEPROM.h>
#define DTPin_SHT 10  // data pin cam bien nhiet do
#define CKPin_SHT 11 // CK pin cam bien nhiet do
#define buzzer 2
#define relay1 3
#define relay2 4
SHT1x sht1x(DTPin_SHT, CKPin_SHT);
#define sub_id 6
float temp;
int temp_set,temp_set_t,temp_t,bdg,canhbao;
void SET_UP_TIMER(){
    TCCR1A = 0;
    TCCR1B = 0;
    TIMSK1 = 0;
    TCCR1B |= (1 << CS12) | (0 << CS11) | (0 << CS10);    // prescale = 256
    TCNT1 = 62500;
    TIMSK1 = (1 << TOIE1); 
    sei();  
}
void setup() {
  Wire.begin(sub_id); 
  Wire.onReceive(receiveEvent); 
  SET_UP_TIMER();
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, LOW);
  temp_set=EEPROM.read(0); temp_set_t=temp_set;
}

void loop() {
  temp = sht1x.readTemperatureC();
  temp_compare();
}

void temp_compare(){
  temp_t = int(temp);
  if(temp_t <= temp_set){
    canhbao=0;
    if(bdg>=1800){      // 1800 giay = 30 phut
      digitalWrite(relay1, !digitalRead(relay1));
      digitalWrite(relay2, !digitalRead(relay2));
      bdg=0;
      }
    }
  else {
    if(canhbao==0){
      if(bdg>=1800) {
        digitalWrite(relay1, HIGH);
        digitalWrite(relay2, HIGH);
        canhbao=1;    // bao hieu da mo 2 relay
        bdg=0;
      }
    }
    if(canhbao==1){   // da mo 2 relay
        if(bdg>=1800) {
          Wire.beginTransmission(sub_id); 
          Wire.write("QUA NHIET");
          Wire.endTransmission();
          canhbao=0;
          bdg=0;
        }
      }
  }
}


void receiveEvent()
{
  if(Wire.available())
  {
    char buf[6]="",buf2[3]="",buf3[2]=""; 
    String str_buf="";
    String receive = Wire.readString();     // cu phap : "SET45"  -> 45 do
    receive.toCharArray(buf, 6);
    strncpy(buf2, buf + 0, 3);
    str_buf = String(buf2);
    if (str_buf=="SET"){
      strncpy(buf3, buf + 3, 2);
      temp_set = 10*(buf3[0]-48)+ (buf3[1]-48) ;
      if(temp_set != temp_set_t){  // tranh viec thay doi o nho EEPROM lien tuc
        EEPROM.write(0, temp_set);
        temp_set_t=temp_set;
        }
      }
  }
}

ISR (TIMER1_OVF_vect) 
{
    bdg++;
}
