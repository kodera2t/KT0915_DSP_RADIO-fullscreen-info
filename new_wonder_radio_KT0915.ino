/*
 * DSP Radio (KT0915) control program on Arduino IDE
 *
 *  Created on: Feb. 13 , 2013
 *      Author: kodera2t
 */

#include "fm_station.h"
#include "air_station.h"
#include "timetable.h"
#include <TFT.h>
#include <SPI.h>
#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>
// SPI LCD pin definition for the Uno
#define cs   10
#define dc   9
#define rst  8
/// DSP radio address
#define RADIO 0x35

int read_byte,raw_upper,upper,lower,mode,read_byte2;
int old_mode;
int j,k,ref,p,q,r;
int old_hou,old_minu,count;
int terminal_1  = 2;
int terminal_2  = 4;
int ct,pt,event,event2;
int rssi_count;
int rssi_count2=0;
int hoge,band_disp;
unsigned int i,hou,minu,sec,rssi,pgalevel_rf,pgalevel_if,old_rssi;
unsigned int stereo,old_stereo;
//unsigned long rssi_count, rssi_count2;
unsigned int channel_num,s_upper,s_lower;

unsigned int initial_num;
volatile int encorder_val=0;
volatile int mode_set=0; /// mode_set=0:AM, mode_set=1:FM
volatile int band_mode = LOW;
volatile int mask,recmode;
float listen_freq;
float freq,old_freq;
volatile char old_state = 0;
unsigned char s_upper2, s_lower2, s_upper3;


// char array to print to the screen
char c_hou[3],c_minu[3],c_sec[3],c_freq[10],c_freq2[2],c_rssi[4];
char buffer[20];
// create an instance of the library
TFT TFTscreen = TFT(cs, dc, rst);
// create an instance of RTC
tmElements_t tm;



void setup() {
  unsigned int upper,lower,raw_upper;
  unsigned int mask,mode,mode_set,hoge,intfreq;
  Wire.begin() ;
  TFTscreen.begin();
  attachInterrupt(0, Rotary_encorder, CHANGE);
  attachInterrupt(1,mode_setting, CHANGE);
//    lcd.begin(16,2);
  pinMode(terminal_1, INPUT);
  pinMode(terminal_2, INPUT);
  pinMode(3, INPUT);
  digitalWrite(terminal_1, HIGH);
  digitalWrite(terminal_2, HIGH);
  digitalWrite(3,HIGH);

// clear the screen with 
//B   G   R 149,18,18 background blue
  TFTscreen.background(149, 18, 18);
  TFTscreen.setRotation(90); //Set the vertical orientation  
  // write the static text to the screen
  // set the font color to 
//B G R 17,234,237 character color
  TFTscreen.stroke(17,234,237);
//drawing borderline
  TFTscreen.line(0,25, TFTscreen.width(), 25);
  TFTscreen.line(0,120, TFTscreen.width(), 120);
  TFTscreen.rect(0,0,TFTscreen.width(),TFTscreen.height());
// set the font size
  TFTscreen.setTextSize(1);
  TFTscreen.text("~Radio Stations~",15,30);
  
// initial clock reading
  if (RTC.read(tm)) {
  hou=tm.Hour;
  minu=tm.Minute;
  sec=tm.Second;
  }else{
  hou=0;
  minu=0;
  }
// initialize several parameters  
  j=0;
  k=1;
  p=0;
  q=0;
  rssi_count=0;
  old_minu=61;    
    mode_set=0; 
//    mode=0; 
    event=1;
    event2=0;
    rssi=0;
    rssi_count=0;
    rssi_count2=0;
    
//// Starting AM mode at 1008 kHz
  i2c_write(RADIO,0x16,0b10000000,0b11000011);
  i2c_write(RADIO,0x22,0b01010100,0b00000000);
//    i2c_write(RADIO,0x0F,0b10001000,0b00001111);
  listen_freq=1008;  
  initial_num=listen_freq;
  channel_num=initial_num;
  s_upper2=(channel_num>>8 | 0b10000000);
  s_lower2=channel_num&0b11111111;
  i2c_write(RADIO,0x02,0b00000000,0b00000111);
  i2c_write(RADIO,0x17,s_upper2,s_lower2);
  i2c_write(RADIO,0x0F,0b10001000,0b00010000);
  i2c_write(RADIO,0x04,0b01100000,0b10000000);
  i2c_write(RADIO,0x22,0b10101111,0b11000100);
///setup end  
}

void loop() {
//  mode=0;
  int intfreq,ffreq,old_ffreq;
  float tmp;
  display_clock(); /// displaying current time
//  rssi_display(); /// displaying RSSI by the color of "@" in the display, and stereo mode in FM



if(event==1){  /// when intterupt by rotary encorder or band switch occurs

  switch(mode_set){
  case 0:////AM
  i2c_write(RADIO,0x16,0b10000000,0b11000011);
  i2c_write(RADIO,0x22,0b01010100,0b00000000);
  listen_freq=1008;  
  initial_num=listen_freq;
  channel_num=initial_num+encorder_val*9;
  s_upper2=(channel_num>>8 | 0b10000000);
  s_lower2=channel_num&0b11111111;
  i2c_write(RADIO,0x02,0b00000000,0b00000111);
  i2c_write(RADIO,0x17,s_upper2,s_lower2);
  old_freq=freq;
  freq=channel_num;
  event=0; 
  mode=0; 
  break;
  
  case 1:////FM
  i2c_write(RADIO,0x16,0b00000000,0b00000010);
  listen_freq=83.3;  
  initial_num=listen_freq*20.0;
  channel_num=initial_num+encorder_val*2;
  s_upper2=(channel_num>>8 | 0b10000000);
  s_lower2=channel_num&0b11111111;
  i2c_write(RADIO,0x02,0b00000000,0b00000111);
  i2c_write(RADIO,0x03,s_upper2,s_lower2);
  old_freq=freq;
  freq=channel_num/20.0;
  event=0;  
  mode=1;
  break; 
  
  case 2: 
  SW_rec(3000);
  break;
  
  case 3:
  SW_rec(4000);
  break;
  case 4:
  SW_rec(5000);
  break;
  case 5:
  SW_rec(6000);
  break; 
  case 6:
  SW_rec(7000);
  break; 
  case 7:
  SW_rec(8000);
  break; 
  case 8:
  SW_rec(9000);
  break;
  case 9:
  SW_rec(10000);
  break;
  case 10:
  SW_rec(11000);
  break; 
  case 11:
  SW_rec(12000);
  break; 
  case 12:
  SW_rec(13000);
  break; 
  case 13:
  SW_rec(14000);
  break;  
  case 14:
  i2c_write(RADIO,0x16,0b00000000,0b00000010);
  listen_freq=126.2;  
  initial_num=listen_freq*20.0;
  channel_num=initial_num+encorder_val;
  s_upper2=(channel_num>>8 | 0b10000000);
  s_lower2=channel_num&0b11111111;
  i2c_write(RADIO,0x02,0b00000000,0b00000111);
  i2c_write(RADIO,0x03,s_upper2,s_lower2);
  old_freq=freq;
  freq=channel_num/20.0;
  event=0;  
  mode=1;
  break;
  }
// broadcast_info(mode);

  TFTscreen.stroke(149,18,18);
  TFTscreen.setTextSize(2);
  TFTscreen.text(".", 50, 143);  
  TFTscreen.text("0", 60, 143); 

  intfreq=old_freq;
  tmp=(old_freq-intfreq)*10;
  ffreq=tmp;
  TFTscreen.stroke(149,18,18);
  String n_freq2= String(intfreq);
  n_freq2.toCharArray(c_freq, 6);
  TFTscreen.setTextSize(2);
  TFTscreen.text(c_freq, 20, 143);


 
   if((mode_set==1)||(mode_set==14)){
  intfreq=old_freq;
  tmp=(old_freq-intfreq)*10;
  old_ffreq=ffreq;
  ffreq=tmp;

  TFTscreen.stroke(149,18,18);
  String n_freq2= String(intfreq);
  n_freq2.toCharArray(c_freq, 6);
  TFTscreen.setTextSize(2);
  TFTscreen.text(c_freq, 20, 143);
  TFTscreen.stroke(149,18,18);
  TFTscreen.setTextSize(2);
  TFTscreen.text(".", 50, 143);  
  String n_freq3= String(old_ffreq);
    n_freq3.toCharArray(c_freq2, 2);
  TFTscreen.setTextSize(2);
  TFTscreen.text(c_freq2, 65, 143);  
  
  intfreq=freq;
  tmp=(freq-intfreq)*10;
  ffreq=tmp;
  TFTscreen.stroke(17,234,237);
  TFTscreen.setTextSize(2);
  TFTscreen.text(".", 50, 143);  
  String n_freq4= String(ffreq);
    n_freq4.toCharArray(c_freq2, 2);
  TFTscreen.setTextSize(2);
  TFTscreen.text(c_freq2, 65, 143); 
  }
  intfreq=freq;
  TFTscreen.stroke(17,234,237);
  String n_freq= String(intfreq);
    n_freq.toCharArray(c_freq, 6);
  TFTscreen.setTextSize(2);
  TFTscreen.text(c_freq, 20, 143);


}
 broadcast_info(mode);
}




void broadcast_info(int band)
{
  if(k==1){
  switch(mode_set){
  case 0:

  for (int i = 0; i < fm_num; i++)
  {
    strcpy_P(buffer, (char*)pgm_read_word(&(air_stations[i]))); 
    TFTscreen.setTextSize(1);
    TFTscreen.stroke(149,18,18);//blue
    TFTscreen.text(buffer, 3, (i+2)*10+30);
    TFTscreen.setTextSize(1);
    TFTscreen.text("AIR BAND 118-133 MHz", 5, 125);  
    TFTscreen.setTextSize(2);
    TFTscreen.text("MHz", 90, 143);
  }
  for (int i = 0; i < sw_num; i++)
  {
    hou=tm.Hour;
    q=hou*sw_num+i;
    strcpy_P(buffer, (char*)pgm_read_word(&(sw_stations[q]))); 
    TFTscreen.setTextSize(1);
    TFTscreen.stroke(17,234,237);
    TFTscreen.text(buffer, 3, (i+2)*10+30);
    TFTscreen.setTextSize(1);
    TFTscreen.text("MW: 522-1620 kHz", 5, 125);  
    TFTscreen.stroke(17,234,237);
    TFTscreen.setTextSize(2);
    TFTscreen.text("kHz", 90, 143);
//    j=0;
  }
  k=0;
  break;
  
  case 1:////FM
  for (int i = 0; i < sw_num; i++)
  {
    hou=tm.Hour;
    q=hou*sw_num+i;
    strcpy_P(buffer, (char*)pgm_read_word(&(sw_stations[q]))); 
    TFTscreen.setTextSize(1);
    TFTscreen.stroke(149,18,18);//blue
    TFTscreen.text(buffer, 3, (i+2)*10+30);
    TFTscreen.setTextSize(1);
    TFTscreen.text("MW: 522-1620 kHz", 5, 125);  
    TFTscreen.setTextSize(2);
    TFTscreen.text("kHz", 90, 143);
//    j=0;
  }  
  for (int i = 0; i < fm_num; i++)
  {
    strcpy_P(buffer, (char*)pgm_read_word(&(fm_stations[i]))); 
    TFTscreen.setTextSize(1);
    TFTscreen.stroke(17,234,237);///yellow
    TFTscreen.text(buffer, 3, (i+2)*10+30);
    TFTscreen.setTextSize(1);
    TFTscreen.text("FM: 76-90 MHz", 5, 125);  
    TFTscreen.setTextSize(2);
    TFTscreen.text("MHz", 90, 143);
  }
  k=0;
  break;
  
  case 2://///SW
  TFTscreen.stroke(149,18,18);
  TFTscreen.setTextSize(2);
  TFTscreen.text(c_freq, 20, 143);
  TFTscreen.stroke(149,18,18);
  TFTscreen.setTextSize(2);  
  TFTscreen.setTextSize(2);
  TFTscreen.text(c_freq2, 65, 143);
  for (int i = 0; i < fm_num; i++)
  {
    strcpy_P(buffer, (char*)pgm_read_word(&(fm_stations[i]))); 
    TFTscreen.setTextSize(1);
    TFTscreen.stroke(149,18,18);//blue
    TFTscreen.text(buffer, 3, (i+2)*10+30);
    TFTscreen.setTextSize(1);
    TFTscreen.text("FM: 76-90 MHz", 5, 125);  
    TFTscreen.setTextSize(2);
    TFTscreen.text("MHz", 90, 143);
  }
  for (int i = 0; i < sw_num; i++)
  {
    hou=tm.Hour;
    q=hou*sw_num+i;
    strcpy_P(buffer, (char*)pgm_read_word(&(sw_stations[q]))); 
    TFTscreen.setTextSize(1);
    TFTscreen.stroke(17,234,237);///yellow
    TFTscreen.text(buffer, 3, (i+2)*10+30);
    TFTscreen.setTextSize(1);
    TFTscreen.text("SW: 2000-30000 kHz", 5, 125);  
    TFTscreen.setTextSize(2);
    TFTscreen.text("kHz", 90, 143);
  }  
  k=0;
  break;
  case 3:
  k=0;
  break;
  case 4:
  k=0;
  break;
  case 5:
  k=0;
  break;
  case 6:
  k=0;
  break;
  case 7:
  k=0;
  break;
  case 8:
  k=0;
  break;
  case 9:
  k=0;
  break;
  case 10:
  k=0;
  break; 
  case 11:
  k=0;
  break; 
  case 12:
  k=0;
  break; 
  case 13:
  k=0;
  break; 
  case 14:
    for (int i = 0; i < sw_num; i++)
  {
    hou=tm.Hour;
    q=hou*sw_num+i;
    strcpy_P(buffer, (char*)pgm_read_word(&(sw_stations[q]))); 
    TFTscreen.setTextSize(1);
    TFTscreen.stroke(149,18,18);//blue
    TFTscreen.text(buffer, 3, (i+2)*10+30);
    TFTscreen.setTextSize(1);
    TFTscreen.text("SW: 2000-30000 kHz", 5, 125);  
    TFTscreen.setTextSize(2);
    TFTscreen.text("kHz", 90, 143);
  }
  for (int i = 0; i < fm_num; i++)
  {
    strcpy_P(buffer, (char*)pgm_read_word(&(air_stations[i]))); 
    TFTscreen.setTextSize(1);
    TFTscreen.stroke(17,234,237);//yekkiw
    TFTscreen.text(buffer, 3, (i+2)*10+30);
    TFTscreen.setTextSize(1);
    TFTscreen.text("AIR BAND 118-133 MHz", 5, 125);  
    TFTscreen.setTextSize(2);
    TFTscreen.text("MHz", 90, 143);
  }  
  k=0;
  break;
  
  }
  }
}


void i2c_write(int device_address, int memory_address, int value, int value2)
{
  Wire.beginTransmission(device_address);
  Wire.write(memory_address);
    delay(5);
  Wire.write(value);
    delay(5);
  Wire.write(value2);
    delay(5);
  Wire.endTransmission();
}


void i2c_read(int device_address, int memory_address)
{
Wire.beginTransmission(device_address);
Wire.write(memory_address);
Wire.endTransmission(false);
Wire.requestFrom(device_address, 2);
read_byte = Wire.read();
//Wire.requestFrom(device_address, 1);
read_byte2 = Wire.read();
Wire.endTransmission(true);
//delay(30);
}




void display_clock(){
  
  if (RTC.read(tm)) {
  hou=tm.Hour;
  minu=tm.Minute;
  sec=tm.Second;
  }
  if(old_minu!=minu){
//  if(r==0){
    //erasing old time
  TFTscreen.stroke(149,18,18);
  TFTscreen.setTextSize(2);
  TFTscreen.text(c_hou, 30, 5);
  TFTscreen.text(":",55,5);
  TFTscreen.text(c_minu, 70, 5);
//  }  
  //writing new time
  TFTscreen.stroke(149,18,18);
  String n_hou= String(hou);
  String n_minu= String(minu);
  String n_sec= String(sec);
//  k=1;  

  n_hou.toCharArray(c_hou, 5);
  n_minu.toCharArray(c_minu, 5);
  n_sec.toCharArray(c_sec, 5);  

  TFTscreen.stroke(17,234,237);
  TFTscreen.setTextSize(2);
  TFTscreen.text(c_hou, 30, 5);
  TFTscreen.text(":",55,5);
  TFTscreen.text(c_minu, 70, 5);
  old_minu=minu;
  }
}

void mode_setting(){
int sw;
ct=millis();
delay(10);
sw=digitalRead(3);
if(sw==LOW && (ct-pt)>50){
band_mode=HIGH;
mode_set=mode_set+1;
}
pt=ct;
if(mode_set>14){
  mode_set=0;
}
event=1;
encorder_val=0;
k=1;
band_disp=0;
}

void Rotary_encorder(void)
{
  if(!digitalRead(terminal_1)){
//    delay(20);
    if(digitalRead(terminal_2)){
      old_state = 'R';
    } else {
      old_state = 'L';
    }
  } else {
    if(digitalRead(terminal_2)){
      if(old_state == 'L'){ 
        encorder_val--;
      }
    } else {
      if(old_state == 'R'){
        encorder_val++;
      }
    }
    old_state = 0;
    event=1;
  }

}

void rssi_display(){
//// display RSSI by the color of "@", renewing every seconds. 
  rssi_count=millis()-1000*rssi_count2; 
  if(rssi_count>1000){
    rssi_count2++;
  i2c_read(RADIO,0x1B);
  old_rssi=rssi;
  rssi=read_byte&0b01111111;
  i2c_read(RADIO,0x18);
  pgalevel_rf=(read_byte&0b11100000)>>5;
  pgalevel_if=(read_byte&0b00011100)>>2;
  if(mode==1){
  rssi=103-rssi-6*pgalevel_rf-6*pgalevel_if;
  
  }else{
  rssi=103-rssi-6*pgalevel_rf-6*pgalevel_if;
  }
  TFTscreen.stroke(149,18,18); ///background color
  TFTscreen.setTextSize(2);
  String o_rssi= String(old_rssi);
  o_rssi.toCharArray(c_rssi, 4);
//  TFTscreen.text(c_rssi, 100, 130);  
  TFTscreen.text("@",110,125);
  if(rssi<20){
  TFTscreen.stroke(252,243,0);
  }else if(rssi<25){
      TFTscreen.stroke(0,252,20);
  }else if(rssi<30){
      TFTscreen.stroke(0,255,255);
  }else if(rssi<60){
          TFTscreen.stroke(0,0,255);
  }  
  TFTscreen.setTextSize(2);
  String n_rssi= String(rssi);
    n_rssi.toCharArray(c_rssi, 4);
  TFTscreen.text("@",110,125);
//  TFTscreen.text(c_rssi, 100, 130);  
//  rssi_count=0;
/////////////
   rssi_count=0;
   old_stereo=stereo;
   i2c_read(RADIO,0x14);
   stereo=read_byte>>7;
   if(stereo==1){
   TFTscreen.stroke(0,0,255);
   TFTscreen.setTextSize(1);
   TFTscreen.text("STEREO MODE",10,133);
   }else{
   TFTscreen.stroke(149,18,18); ///background color
   TFTscreen.setTextSize(1);
   TFTscreen.text("STEREO MODE",10,133);
   }
}
}


void SW_rec(float listen_freq)
{
  i2c_write(RADIO,0x16,0b10000000,0b00000011);
  i2c_write(RADIO,0x22,0b01010100,0b00000000);
//  listen_freq=3000;  
  initial_num=listen_freq;
  channel_num=initial_num+encorder_val*5;
  s_upper2=(channel_num>>8 | 0b10000000);
  s_lower2=channel_num&0b11111111;
  i2c_write(RADIO,0x02,0b00000000,0b00000111);
  i2c_write(RADIO,0x17,s_upper2,s_lower2);
  old_freq=freq;
  freq=channel_num;
  event=0; 
  mode=0;
}
