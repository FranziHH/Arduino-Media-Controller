#include "HID-Project.h"
#include <EEPROM.h>

#define RE_CLK    6
#define RE_DATA   7

#define DEBOUNCE 100  // button debouncer, how many ms to debounce
// the first Button is RotaryButton
byte buttons[] = {8, 2, 3, 4, 5};
#define SWITCH_TIME 3000 // Time for Switch Button Function
int functions[] = {MEDIA_VOL_MUTE, MEDIA_PLAY_PAUSE, MEDIA_STOP, MEDIA_NEXT, MEDIA_PREV};
// This handy macro lets us determine how big the array up above is, by checking the size
#define NUMBUTTONS sizeof(buttons)

static uint8_t prevNextCode = 0; // statics for rotary encoder
static uint16_t store = 0;

void setup()
{ 
  functions[0] = readIntFromEEPROM(1);
  if (functions[0] != MEDIA_VOL_MUTE && functions[0] != MEDIA_PLAY_PAUSE) {
    // set Default Value
    functions[0] = MEDIA_VOL_MUTE;
    writeIntIntoEEPROM(1, functions[0]);
  }
  
  pinMode (RE_CLK,INPUT_PULLUP);
  pinMode (RE_DATA,INPUT_PULLUP);
  
  for (byte i=0; i< NUMBUTTONS; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
  }
  
  Consumer.begin(); 
} 

void loop()
{
  static int8_t re_val;

  if (re_val = read_rotary()) {
    if ( re_val == -1 ) {
      Consumer.write(MEDIA_VOLUME_DOWN);
      Keyboard.releaseAll();
    }
    if ( re_val == 1 ) {
      Consumer.write(MEDIA_VOLUME_UP);
      Keyboard.releaseAll();
    }
  }

  check_switches();
}

void check_switches()
{
  static byte previousstate[NUMBUTTONS];
  static byte currentstate[NUMBUTTONS];
  static unsigned long lasttime[NUMBUTTONS];
  static unsigned long lastswitchtime;
  static bool flag;
  byte index;
  
  for (index = 0; index < NUMBUTTONS; index++) {
    if (millis() < lasttime[index]) {
       // we wrapped around, lets just try again
       lasttime[index] = millis();
    }
    if (millis() - lasttime[index] > DEBOUNCE) {
      currentstate[index] = digitalRead(buttons[index]);
      if (currentstate[index] == LOW) {
        currentstate[index] = digitalRead(buttons[index]); // check a 2nd time to be sure
        if (previousstate[index] != currentstate[index]) {
          if (currentstate[index] == LOW) {// check a 2nd time to be sure
            Consumer.write(functions[index]);
            if (index == 0) lastswitchtime = millis();
          }
        }
      }
      previousstate[index] = currentstate[index];
      lasttime[index] = millis();
    }

    // Switch Button Function
    if (index == 0) {
      if (millis() < lastswitchtime) {
         // we wrapped around, lets just try again
         lastswitchtime = millis();
      }
      if (millis() - lastswitchtime > SWITCH_TIME) {
        if (flag == false) {
          currentstate[index] = digitalRead(buttons[index]);
          if (currentstate[index] == LOW) {
            if (functions[0] == MEDIA_VOL_MUTE) {
              functions[0] = MEDIA_PLAY_PAUSE;
            } else {
              functions[0] = MEDIA_VOL_MUTE;
            }
            writeIntIntoEEPROM(1, functions[0]);
            flag = true;
          }
          lastswitchtime = millis();
        }
      }
    } else {
      flag = false;
    }
    
  } //for

}

// Robust Rotary encoder reading
// Copyright John Main - best-microcontroller-projects.com
// A vald CW or  CCW move returns 1, invalid returns 0.
int8_t read_rotary() {
  static int8_t rot_enc_table[] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
  
  prevNextCode <<= 2;
  if (digitalRead(RE_DATA)) prevNextCode |= 0x02;
  if (digitalRead(RE_CLK)) prevNextCode |= 0x01;
  prevNextCode &= 0x0f;

  // If valid then store as 16 bit data.
  if  (rot_enc_table[prevNextCode] ) {
    store <<= 4;
    store |= prevNextCode;
    if ((store & 0xff) == 0x2b) return -1;
    if ((store & 0xff) == 0x17) return 1;
  }
  return 0;
}

void writeIntIntoEEPROM(int address, int number)
{ 
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
}

int readIntFromEEPROM(int address)
{
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}
