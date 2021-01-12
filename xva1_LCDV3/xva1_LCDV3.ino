// Copyright 2020 Rich Heslip
//
// Author: Rich Heslip 
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// 
// simple editor for the XVA1 synth
// implemented with an ESP32,4 line x 20 char LCD display, encoders + pots
// July 2020 R Heslip
//
// Version 3 Jan 2021 - wrote my own menu system to reduce menu diving. uses a menu encoder with switch for menu navigation and 4 encoders to edit parameter values (pots could be used too)
// this editor is for tweeking the most used parameters and saving patches in XVA1 memory. its not intended for "deep" editing - for that use my CTRLR editor which can edit virtually everything
// two levels of menus - top level is by functional block/group/activity e.g. oscillator 1, oscillator 2, save patch etc
// 2nd level is the parameters for that block e.g. waveform, detune, transpose, pulsewidth. we can edit 4 of these at a time with the 4 pots/encoders
// main encoder scrolls through the top level selections when the encoder button is pressed or the parameter list for the selection when not pressed. ie you can reach any parameter quickly with a press+rotation and then a rotation
// double click main encoder button is used as an enter command for the init, load and store patch routines
// I'm pretty happy with this. MUCH faster and more intuitive than the multi level menus in version 1 & 2 and very little code - mostly menu data structures and they are pretty concise too compared to Arduino Menu lib
// Jan 10/21 - added a secondary top menu to keep the main menu from getting too long. this is for less used stuff. double click toggles between main and secondary
// Jan 12 - added a bit of code and some menus, now the encoders don't work unless serial is enabled. Should be OK as long as you don't send anything on the serial port. ESP32 arduino is quite flakey...

#include <Arduino.h> 
//#include <BLEDevice.h>
//#include <BLEUtils.h>
//#include <BLEServer.h>
//#include <BLE2902.h>
//#include <pgmspace.h>
#include <LiquidCrystal.h>
#include "menusystem.h"  
#include "MIDI.h"
#include "io.h"
#include "Clickencoder.h"
#include <strings.h>

#define DEBUG // enables serial out - should be disabled unless needed because I used the serial pins for encoder buttons 1&2- does strange things to the menus


// sample interrupt timer defs
#define ENC_TIMER_MICROS 1000 // 1khz for encoder
hw_timer_t * timer1 = NULL;


// MIDI stuff
uint8_t MIDI_Channel=1;  // default MIDI channel 
uint8_t incoming_MIDI_channel;  //  MIDI channel of incoming data
long midimessagetime;
#define MIDI_CHANNEL_DISPLAY_TIME 1000  // length of time to show incoming midi channel on LCD

// create LCD display device
// RS,E,D4,D5,D6,D7
LiquidCrystal lcd(23, 33, 25, 15, 18, 19);

// encoders 
ClickEncoder menuEncoder(ENC_A,ENC_B,ENC_SW,4); // divide by 4 works best with this encoder
ClickEncoder P1Encoder(P1ENC_A,P1ENC_B,P1_SW,4); // switch is hooked to RXD0 - have to set it up after calling serial.begin()
ClickEncoder P2Encoder(P2ENC_A,P2ENC_B,P2_SW,4); 
ClickEncoder P3Encoder(P3ENC_A,P3ENC_B,P3_SW,4); 
ClickEncoder P4Encoder(P4ENC_A,P4ENC_B,P4_SW,4); 

// ADC readings
#define ADC_RANGE 4096  // 12 bit ADC
#define CV_AVG 8     // number of readings to average readings over - 8 is max to avoid overflowing 16 bit int
#define CV_SCALE 16  // A/D scale factor for parameter values

// volume pot stuff
#define VOLUME_LOCK 1000 // we lock the volume pot after this time (ms) to avoid audio crackles
#define VOLUME_THRESHOLD 5 // volume change required to unlock
uint16_t volume=0;
bool volume_locked;
long volumetimer;

// array that holds 512 synth parameters plus some internal parameters above that -FPGA memory read slot,write slot, dummy
// param 0 is not used in the XFM2 or XVA1
uint8_t parameters[NUMPARAMS]; // xva1/xfm2 parameters plus a few extras for editor use

// change a parameter on the FPGA synth
// write data from the parameter array which is what the menus modify
// note that Rene's documentation says the 2 byte address threshold is >=255 but his UI code uses >=256

void setparameter(uint16_t paramnumber) {
  Serial2.write('s');
  if (paramnumber <256) {  
    Serial2.write((unsigned char)paramnumber);  // address
    Serial2.write(parameters[paramnumber]);  // data
  }
  else {
    Serial2.write(255);  // address low
    Serial2.write((unsigned char)(paramnumber-256));  // address high
    Serial2.write(parameters[paramnumber]); // data
  }
}

// write a parameter to the FPGA synth
// same as above but the value is passed as an argument

void writeparameter(uint16_t paramnumber,unsigned char val) {
  Serial2.write('s');
  if (paramnumber <256) {  
    Serial2.write((unsigned char)paramnumber);  // address
    Serial2.write(val);  // data
  }
  else {
    Serial2.write(255);  // address low
    Serial2.write((unsigned char)(paramnumber-256));  // address high
    Serial2.write(val); // data
  }
}

// read all 512 parameters from the FPGA
void read_params(void) {
  while (Serial2.available() == 1) Serial2.read(); // dump any unread shit
  Serial2.write('d'); 
  for (int i=0; i<512;++i) {
    while (Serial2.available() == 0); // wait for the data
    parameters[i]=Serial2.read();
  }
}

// load patch from FPGA memory
// slot - FPGA memory slot number
void loadpatch(uint8_t slot) {
  Serial2.write('r'); 
  Serial2.write(slot);
  while (Serial2.available() == 0); // wait for the acknowledgement
  Serial2.read(); // should be 0, I don't check
  read_params(); 
}

// write patch to FPGA memory
// slot - FPGA memory slot number
void writepatch(uint8_t slot) {
  Serial2.write('w'); 
  Serial2.write(slot);
  while (Serial2.available() == 0); // wait for the acknowledgement
  Serial2.read(); // should be 0, I don't check
}

// init patch
void initpatch(void) {
  Serial2.write('i'); 
  while (Serial2.available() == 0); // wait for the acknowledgement
  Serial2.read(); // should be 0, I don't check
  read_params(); 
}

// encoder timer 0 interrupt handler at 1khz
// uses the millis timer
void ICACHE_RAM_ATTR encTimer(){
  menuEncoder.service();    // check the encoder inputs 
  P1Encoder.service();
  P2Encoder.service();
  P3Encoder.service();
  P4Encoder.service();
}


// simple MIDI handler - 47Effects library crashes on me
// called when there is MIDI data available
// all we do here is detect incoming channel number and when the data arrived
// this is used in maindisplay() to show the incoming MIDI channel

void doMIDI(void) {
  unsigned char mididata,midichannel;
  mididata=Serial1.read();
  if ((mididata & 0x90) == 0x90) {  // this is a note on message
    //Serial.println(mididata);
    incoming_MIDI_channel=(mididata &0x0f)+1;
    midimessagetime=millis();
  }
}

// menu stuff

bool channeldisplay=false; // true while we are showing MIDI channel
menu * topmenu=mainmenu;  // points at current menu
int8_t topmenuindex;  // keeps track of which top menu item we are displaying
int8_t mainmenuindex;  // saves index for main menu when we are in secondary menu
int8_t secondarymenuindex;  // saves index for secondary menu when we are in main menu

// timer and flag for managing temporary messages
#define MESSAGE_TIMEOUT 1500
long messagetimer;
bool message_displayed;

// display the top menu
void drawtopmenu( int8_t index) {
    lcd.setCursor ( 0, TOPMENU_Y ); 
    lcd.print("                    "); // kludgy line erase
    lcd.setCursor ( 0, TOPMENU_Y ); 
    lcd.print(topmenu[index].name);
}

// display a sub menu item and its value
// index is the index into the current top menu's submenu array
// pos is the relative x location on the screen ie field 0,1,2 or 3 
void drawsubmenu( int8_t index, int8_t pos) {
    submenu * sub;
    // print the name text
    lcd.setCursor ((LCD_X/SUBMENU_FIELDS)*pos, SUBMENU_Y ); // set cursor to parameter name field
    sub=topmenu[topmenuindex].submenus; //get pointer to the submenu array
    if (index < topmenu[topmenuindex].numsubmenus) lcd.print(sub[index].name); // make sure we aren't beyond the last parameter in this submenu
    else lcd.print("     ");

    // print the value
    lcd.setCursor ((LCD_X/SUBMENU_FIELDS)*pos, SUBMENU_VALUE_Y ); // set cursor to parameter value field
    if ((sub[index].parameter < DUMMY) && (index < topmenu[topmenuindex].numsubmenus)) { // don't print dummy parameter or beyond the last submenu item
      uint8_t val=parameters[sub[index].parameter];  // fetch the parameter value
      if (val> sub[index].range) parameters[sub[index].parameter]=val=sub[index].range; // check the parameter range and limit if its out of range ie we loaded a bad patch
      switch (sub[index].ptype) {
        case TYPE_NUM:   // print the value as an unsigned integer    
          char temp[5];
          sprintf(temp,"%4u",val); // lcd.print doesn't seem to print uint8 properly
          lcd.print(temp);  
          lcd.print(" ");  // blank out any garbage
          break;
        case TYPE_TEXT:  // use the value to look up a string
          lcd.print(sub[index].ptext[val]); // parameter value indexes into the string array
          lcd.print(" ");  // blank out any garbage
          break;
        default:
        case TYPE_NONE:  // blank out the field
          lcd.print("     ");
          break;
      } 
    }
    else lcd.print("     ");  // it was a dummy parameter or an indexing error so blank the field 
}

// display the sub menus of the current top menu

void drawsubmenus() {
  int8_t index = topmenu[topmenuindex].submenuindex;
  for (int8_t i=0; i< SUBMENU_FIELDS; ++i) drawsubmenu(index++,i);
}

//adjust the topmenu index and update the menus and submenus
// dir - int value to add to the current top menu index ie L-R scroll
void scrollmenus(int8_t dir) {
  topmenuindex+= dir;
  if (topmenu== mainmenu) {
    if (topmenuindex < 0) topmenuindex = NUM_MAIN_MENUS -1; // handle wrap around
    if (topmenuindex >= NUM_MAIN_MENUS ) topmenuindex = 0; // handle wrap around
  }
  if (topmenu== secondarymenu) {
    if (topmenuindex < 0) topmenuindex = NUM_SECONDARY_MENUS -1; // handle wrap around
    if (topmenuindex >= NUM_SECONDARY_MENUS ) topmenuindex = 0; // handle wrap around
  }
  drawtopmenu(topmenuindex);
  drawsubmenus();    
}

// same as above but scrolls submenus
void scrollsubmenus(int8_t dir) {
  dir= dir*SUBMENU_FIELDS; // sidescroll SUBMENU_FIELDS at a time
  topmenu[topmenuindex].submenuindex+= dir;
  if (topmenu[topmenuindex].submenuindex < 0) topmenu[topmenuindex].submenuindex = 0; // stop at first submenu
  if (topmenu[topmenuindex].submenuindex >= topmenu[topmenuindex].numsubmenus ) topmenu[topmenuindex].submenuindex -=dir; // stop at last submenu     
  drawsubmenus();      
}

// show a message on 2nd line of display - it gets auto erased after a timeout
void showmessage(char * message) {
  lcd.setCursor(0, MSG_Y); 
  lcd.print(message);
  messagetimer=millis();
  message_displayed=true;
}

// clear the message on the message line
void erasemessage(void) {
    lcd.setCursor(0, MSG_Y); 
    lcd.print("                    "); 
    message_displayed=false;  
}

// unlock the volume pot
void volumeunlock(void) {
  volume_locked=false;  // unlock the volume so we use the pot volume, not the patch volume
  volumetimer=millis();
}


void setup() {
  
  // hack - use serial pins are used for encoder switches. serial out still works but it messes up the switch inputs sometimes
  // there is a 470 ohm R between the ESP32 pins and the serial chip on the board I'm using so this shouldn't hurt anything
#ifdef DEBUG   
 Serial.begin(115200);
 Serial.println("Starting setup");   
#endif


  
// Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  Serial2.begin(500000, SERIAL_8N1, RXD2, TXD2);
  
//  Set up serial MIDI port
//  MIDISerial1.begin(31250, SERIAL_8N1, MIDIRX,MIDITX ); // midi port
  channeldisplay=false; // true while we are showing MIDI channel
  Serial1.begin(31250, SERIAL_8N1, MIDIRX, MIDITX);

  read_params(); // sync the parameter array to the synth
  
     // start up the display - 20 chars by 4 lines
  lcd.begin(LCD_X,LCD_Y);               // initialize the lcd 
  lcd.home ();                   // go home
  lcd.print("       XVA1");
  delay (1000);
  topmenuindex=0;
  mainmenuindex=0;
  secondarymenuindex=0;
  drawtopmenu(topmenuindex);    // initial menu display
  drawsubmenus();


  // 2nd timer for encoder sampling
  timer1 = timerBegin(1, 80, true);
  timerAttachInterrupt(timer1, &encTimer, true);
  // Set alarm to trigger ISR (value in microseconds).
  // Repeat the alarm (third parameter)
  timerAlarmWrite(timer1, ENC_TIMER_MICROS, true);
  timerAlarmEnable(timer1);
}


void loop() {
  int16_t enc;
  int8_t index; 
  int16_t encodervalue[4]; 

  ClickEncoder::Button button; 
  
//  MIDI.read();  // do serial MIDI
  if (Serial1.available()) doMIDI();

// process the menu encoder - scroll submenus, scroll main menu when button down
  enc=menuEncoder.getValue(); // compiler bug - can't do this inside the if statement
  button= menuEncoder.getButton(); // 
// Serial.println(button);    
//  if ((button == ClickEncoder::Closed)||(button == ClickEncoder::Held))  {// there is a bit of a delay detecting button state 
  if (digitalRead(ENC_SW) == 0)  {// we are scrolling thru top menus  
//    if ((button == ClickEncoder::Closed)) showmessage("Dub Click-->2nd Menu"); // help message
    if ( enc != 0) {
      scrollmenus(enc);
    }
  }
  else {  // we are scrolling submenus
    if ( enc != 0) {
        scrollsubmenus(enc);           
    }
  }

// process menu encoder double clicks
// switches to main/secondary menu

//  button= menuEncoder.getButton(); // 
  if (button == ClickEncoder::DoubleClicked) {
   // toggle main and secondary
     if (topmenu==mainmenu) {
       topmenu=secondarymenu;
       mainmenuindex=topmenuindex; // save where we are
       topmenuindex=secondarymenuindex; // restore secondary menu position
       showmessage("* 2nd Menu Active");
     }
     else {
       topmenu=mainmenu;
       secondarymenuindex=topmenuindex; // save where we are
       topmenuindex=mainmenuindex;  // restore main menu position
       showmessage("Main Menu Active");
     }
     scrollmenus(0);  // ensure menuindex is still in range for the new menu
  }
 
// process parameter encoder buttons - button gestures are used as shortcuts/alternatives to using main encoder
// hold button to jump to specific top menus, then can rotate to scroll top menus 
// middle left encoder click goes to previous menu
// middle right encoder click goes to next menu
// left encoder click goes to previous submenu
// right encoder click goes to next submenu
  index= topmenu[topmenuindex].submenuindex; // submenu field index
  submenu * sub=topmenu[topmenuindex].submenus; //get pointer to the current submenu array
  button= P1Encoder.getButton();
  
  switch (button) {
    case ClickEncoder::Held:
      topmenuindex=OSCS;
      scrollmenus(0); // update the menus 
      erasemessage(); // screen cleanup
      while (P1Encoder.getButton() == ClickEncoder::Held) {  // the parameter encoder to scrolls top menus as long as the button is pressed
        enc=P1Encoder.getValue();
        if (enc!=0) scrollmenus(enc); 
      }  
      break;  
    case ClickEncoder::Clicked:
      scrollsubmenus(-1);    // click on left encoder goes to previous submenu
      break;
    case ClickEncoder::DoubleClicked:
      switch (sub[index].parameter) {  // these menus use internal parameters
        case INIT_SLOT:
          initpatch();
          volumeunlock();   // use the volume from the volume pot
          showmessage("Patch Initialized");
          break;
        case WRITE_SLOT:
          writepatch(parameters[WRITE_SLOT]);
          showmessage("Patch Saved");
          break;
        default:
        break;
      }
  }

  button= P2Encoder.getButton();
  switch (button) {
    case ClickEncoder::Held:
      topmenuindex=FILTERS;
      scrollmenus(0); // update the menus 
      erasemessage(); // screen cleanup
      while (P2Encoder.getButton() == ClickEncoder::Held) {  // the parameter encoder to scrolls top menus as long as the button is pressed
        enc=P2Encoder.getValue();
        if (enc!=0) scrollmenus(enc); 
      }  
      break;  
    case ClickEncoder::Clicked:
      scrollmenus(-1);    // click on middle left encoder goes to previous menu
      break;
  }
  
  button= P3Encoder.getButton();
  switch (button) {
    case ClickEncoder::Held:
      topmenuindex=ENVELOPES;
      scrollmenus(0); // update the menus 
      erasemessage(); // screen cleanup
      while (P3Encoder.getButton() == ClickEncoder::Held) {  // the parameter encoder to scrolls top menus as long as the button is pressed
        enc=P3Encoder.getValue();
        if (enc!=0) scrollmenus(enc); 
      }  
      break;  
    case ClickEncoder::Clicked:
      scrollmenus(1);    // click ond middle right encoder goes to next menu
      break;
  }

  button= P4Encoder.getButton();
  switch (button) {
    case ClickEncoder::Held:
      topmenuindex=EFFECTS;
      scrollmenus(0); // update the menus 
      erasemessage(); // screen cleanup
      while (P4Encoder.getButton() == ClickEncoder::Held) {  // the parameter encoder to scrolls top menus as long as the button is pressed
        enc=P4Encoder.getValue();
        if (enc!=0) scrollmenus(enc); 
      }  
      break;  
    case ClickEncoder::Clicked:
      scrollsubmenus(1);    // click on right encoder goes to next submenu
      break;
  }

 // process parameter encoders
  encodervalue[0]=P1Encoder.getValue();  // read encoders
  encodervalue[1]=P2Encoder.getValue();
  encodervalue[2]=P3Encoder.getValue();
  encodervalue[3]=P4Encoder.getValue();

  for (int field=0; field<4;++field) { // loop thru the on screen submenus
    if (encodervalue[field]!=0) {  // if there is some input, process it
      uint16_t p=sub[index].parameter; // array index of the parameter we are editing
      int16_t temp=(int16_t)parameters[p] + encodervalue[field]; // use ints here - way easier to handle overflows
      if (temp < 0) temp=0;
      if (temp > (int16_t)sub[index].range) temp=sub[index].range;
      parameters[p]=(uint8_t)temp;
      if (p < LOAD_SLOT) setparameter(p); // don't send internal parameters to the FPGA
      if (p == LOAD_SLOT) {
        loadpatch(parameters[p]); // load patch happens when we change the patch number in that submenu
        volumeunlock();   // use the volume from the volume pot
      }
      showmessage(sub[index].longname);  // show the long name of what we are editing
      drawsubmenu(index,field);
    }
    ++index;
    if (index >= topmenu[topmenuindex].numsubmenus) break; // check that we have not run out of submenus
  }

// handle volume which has a dedicated pot
// we lock the volume after a while because the A/D jumps around and causes audio crackles
  int16_t cv_in=0;
  for (int i=0; i< CV_AVG; ++i) cv_in+=analogRead(VOLUMEPOT); // average the noisy A/D 
  cv_in=(cv_in/CV_AVG)/CV_SCALE;
  int delta = abs(cv_in-volume);
  if (delta > VOLUME_THRESHOLD) {
    volume_locked=false; // we hit the threshold, unlock the pot and reset the timer
    volumetimer=millis();  
  }

  if (volume_locked == false) {
    volume=cv_in;
    writeparameter(509,(unsigned char)(volume)); // adjust volume 0-255
  }

// timer housekeeping
  if ((millis() - volumetimer) > VOLUME_LOCK) volume_locked=true; 
  
  if (((millis() - messagetimer) > MESSAGE_TIMEOUT) && (message_displayed==true)) erasemessage();


}




