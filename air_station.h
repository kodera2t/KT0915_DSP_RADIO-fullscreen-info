#define fm_num  7
#include <avr/pgmspace.h>
prog_char a_string_0[] PROGMEM = "South Chu :126.1MHz";   // "String 0" etc are strings to store - change to suit.
prog_char a_string_1[] PROGMEM = "GND       :126.2MHz";
prog_char a_string_2[] PROGMEM = "South Kyu:133.15MHz";
prog_char a_string_3[] PROGMEM = "TWR      :118.05MHz";
prog_char a_string_4[] PROGMEM = "ANA       :129.1MHz";
prog_char a_string_5[] PROGMEM = "JAL      :128.92MHz";
prog_char a_string_6[] PROGMEM = "TWR       :118.1MHz";

PROGMEM const char *air_stations[] =
{   
  a_string_0,
  a_string_1,
  a_string_2,
  a_string_3,
  a_string_4,
  a_string_5,
  a_string_6,
  };
 
