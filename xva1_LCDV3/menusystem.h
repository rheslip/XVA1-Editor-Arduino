
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

#define LCD_X 20  // 20 char display
#define LCD_Y 4   // 4 lines
#define TOPMENU_Y 0   // line to display top menus
#define MSG_Y   1    // line to display messages
#define SUBMENU_FIELDS 4 // max number of subparameters on a line ie for 20 char display each field has 5 characters
#define SUBMENU_Y 2 // line number to display submenus (parameter names)
#define SUBMENU_VALUE_Y 3 // line number to display parameter values

// XVA1 and XFM2 have 512 parameters in a patch. anything higher than 511 doesn't get saved to the synth memory so is safe for other uses
#define LOAD_SLOT 512  // parameter numbers I use for XVA1 memory load and save slots
#define WRITE_SLOT 513
#define INIT_SLOT 514  // fake parameter for init menu
#define DUMMY 515   // dummy - editing this one does no harm 
#define NUMPARAMS DUMMY+1

enum paramtype{TYPE_NONE,TYPE_NUM, TYPE_TEXT}; // parameter display types

// holds index into the pot arrays
unsigned char controls[4]={0,2,12,15};


// submenus 
struct submenu {
  char *name; // display short name
  char *longname; // longer name displays on message line
  uint8_t range;  // max value of parameter
  enum paramtype ptype; // how its displayed
  char ** ptext;   // points to array of text for text display
  uint16_t parameter; // parameter number
};

// the parameter arrays must be padded to a multiple of SUBMENU_FIELDS ie 4, 8, 12 etc
// this makes the index wrap around checking a lot easier. 
// makes sense to scroll submenus SUBMENU_FIELDS at a time. if we side scroll one at a time the parameter positions change which could be a bit confusing



// oscillator submenus
char * textoffon[] = {" OFF", "  ON"};
char * textwaves[] = {"SAWU", "SAWD"," SQR"," TRI"," SIN","NOIS","SS3S","SS7M","SS7S"};

struct submenu osc1params[] = {
  // name,longname,range,type,textfield,parameter#
  "ENAB","Osc. On/Off",1,TYPE_TEXT,textoffon,1,
  "WAVE","Waveform",8,TYPE_TEXT,textwaves,11,
  "DTUN","Detune",255,TYPE_NUM,0,23, 
  "TPOS","Transpose",255,TYPE_NUM,0,19,   
  "  PW","Pulse Width",255,TYPE_NUM,0,15,
  "LEVL","Level",255,TYPE_NUM,0,27, 
  "LVLL","Level Left",255,TYPE_NUM,0,31, 
  "LVLR","Level Right",255,TYPE_NUM,0,32,
  "SDET","Sawstack Detune",255,TYPE_NUM,0,285, 
};

struct submenu osc2params[] = {
  // name,longname,range,type,textfield,parameter#
  "ENAB","Osc. On/Off",1,TYPE_TEXT,textoffon,2,
  "WAVE","Waveform",8,TYPE_TEXT,textwaves,12,
  "DTUN","Detune",255,TYPE_NUM,0,24,      
  "TPOS","Transpose",255,TYPE_NUM,0,20,
  "  PW","Pulse Width",255,TYPE_NUM,0,16,
  "LEVL","Level",255,TYPE_NUM,0,28, 
  "LVLL","Level Left",255,TYPE_NUM,0,33, 
  "LVLR","Level Right",255,TYPE_NUM,0,34,
  "SDET","Sawstack Detune",255,TYPE_NUM,0,286,
};

struct submenu osc3params[] = {
  // name,longname,range,type,textfield,parameter#
  "ENAB","Osc. On/Off",1,TYPE_TEXT,textoffon,3,
  "WAVE","Waveform",8,TYPE_TEXT,textwaves,13,
  "DTUN","Detune",255,TYPE_NUM,0,25,     
  "TPOS","Transpose",255,TYPE_NUM,0,21,
  "  PW","Pulse Width",255,TYPE_NUM,0,17,
  "LEVL","Level",255,TYPE_NUM,0,29, 
  "LVLL","Level Left",255,TYPE_NUM,0,35, 
  "LVLR","Level Right",255,TYPE_NUM,0,36,
  "SDET","Sawstack Detune",255,TYPE_NUM,0,287,
  "RING","Ringmod 3-4",1,TYPE_TEXT,textoffon,271,
};

struct submenu osc4params[] = {
  // name,longname,range,type,textfield,parameter#
  "ENAB","Osc. On/Off",1,TYPE_TEXT,textoffon,4,
  "WAVE","Waveform",8,TYPE_TEXT,textwaves,14,
  "DTUN","Detune",255,TYPE_NUM,0,26,  
  "TPOS","Transpose",255,TYPE_NUM,0,22,    
  "  PW","Pulse Width",255,TYPE_NUM,0,18,
  "LEVL","Level",255,TYPE_NUM,0,30, 
  "LVLL","Level Left",255,TYPE_NUM,0,37, 
  "LVLR","Level Right",255,TYPE_NUM,0,38,
  "SDET","Sawstack Detune",255,TYPE_NUM,0,288,
};

// filter parameter submenus
char * textfilter[] = {" BYP","LP1P","LP2P","LP3P","LP4P","HP1P","HP2P","HP3P","HP4P","BP2P","BP4P","BP2P","BR4P","LLPS","LBPS",
    "LHPS","LLPP","LBPP","LHPP","BBPP","BHPP","HHPP"};
char * textfiltroute[] = {" STD","  LR"};

struct submenu filtparams[] = {
  // name,longname,range,display type,textfield *,parameter number
  "TYPE","Filter Type",21,TYPE_TEXT,textfilter,71,  // filter type
  "CUT1","Filter 1 Cut",255,TYPE_NUM,0,72,  // cutoff 1 freq
  "RES1","Filter 1 Resonance",255,TYPE_NUM,0,77,  // filter 1 resonance
  "  EG","Env. Gen. Level",255,TYPE_NUM,0,75,   // env gen depth 
  
  "CUT2","Filter 2 Cut",255,TYPE_NUM,0,78,  // cutooff 2 freq
  "RES2","Filter 2 Resonance",255,TYPE_NUM,0,79,  // filter 2resonance
  "VELO","Velocity Level",255,TYPE_NUM,0,73,   // velocity 
  "DRIV","Drive",7,TYPE_NUM,0,275,   // drive
  
  "KBTR","Keyboard Tracking",255,TYPE_NUM,0,74,   // keyboard tracking
  "EGVL","Env. Gen. Velocity",255,TYPE_NUM,0,76,   // eg velocity
  "VELR","Velocity Resonance",255,TYPE_NUM,0,276,   // velocity resonance
  " KBR","Keyboard Resonance",255,TYPE_NUM,0,277,   // keyboard resonance

  "ROUT","Filter Routing",1,TYPE_TEXT,textfiltroute,278,   // routing
};


// save patch menu - uses internal parameter WRITE_SLOT
struct submenu saveparams[] = {
// name,range,display type,textfield *,parameter number
  "Slot","Double Click to Save",127,TYPE_NUM,0,WRITE_SLOT,   // XVA1 memory slots 0-127 
  "    ","Save to Memory",0,TYPE_NONE,0,DUMMY,     
  "    ","",1,TYPE_NONE,0,DUMMY,   // dummy parameter doesn't display
  "    ","",1,TYPE_NONE,0,DUMMY,   // 
};

// load patch menu - uses internal parameter LOAD_SLOT
struct submenu loadparams[] = {
// name,longname,range,display type,textfield *,parameter number
  "Slot","Load from Memory",127,TYPE_NUM,0,LOAD_SLOT,   // XVA1 memory slots 0-127
  "    ","",1,TYPE_NONE,0,DUMMY,     
  "    ","",1,TYPE_NONE,0,DUMMY,   // dummy parameter doesn't display 
  "    ","",1,TYPE_NONE,0,DUMMY,   // 
};

// init patch menu - uses dummy internal parameter INIT_SLOT
char * textinit[] = {"Clik",};
struct submenu initparams[] = {
// name,longname,range,display type,textfield *,parameter number
  "Dubl","Double Click to Init",0,TYPE_TEXT,textinit,INIT_SLOT,   // INIT_SLOT used to interpret the double click  
  "    ","",1,TYPE_NONE,0,DUMMY,   // dummy parameter doesn't display
  "    ","",1,TYPE_NONE,0,DUMMY,   // 
  "    ","",1,TYPE_NONE,0,DUMMY,   
};

// envelope generator submenus
// just doing ADSR to keep it simple
struct submenu egampparams[] = {
  // name,longname,range,display type,textfield *,parameter number
  "ARAT","Attack Rate",255,TYPE_NUM,0,117,  // attack rate
  "DRAT","Decay Rate",255,TYPE_NUM,0,127,  // decay1 rate
  "SUST","Sustain Level",255,TYPE_NUM,0,97,  // sustain level
  "RRAT","Release Rate",255,TYPE_NUM,0,132,   // release rate 
};

struct submenu egfiltparams[] = {
  // name,longname,range,display type,textfield *,parameter number
  "ARAT","Attack Rate",255,TYPE_NUM,0,116,  // attack rate
  "DRAT","Decay Rate",255,TYPE_NUM,0,126,  // decay1 rate
  "SUST","Sustain Level",255,TYPE_NUM,0,96,  // sustain level
  "RRAT","Release Rate",255,TYPE_NUM,0,131,   // release rate 
};

struct submenu egpitchparams[] = {
  // name,longname,range,display type,textfield *,parameter number
  "ARAT","Attack Rate",255,TYPE_NUM,0,115,  // attack rate
  "DRAT","Decay Rate",255,TYPE_NUM,0,125,  // decay1 rate
  "SUST","Sustain Level",255,TYPE_NUM,0,95,  // sustain level
  "RRAT","Release Rate",255,TYPE_NUM,0,130,   // release rate 
};

// LFO submenus
char * textlfowaves[] = {" TRI"," SQR","SAWU","SAWD"," SIN","Sx2x","Sx3x","Sx^3","GUIT"," S&H"};
char * textlforange[] = {" LOW","HIGH"};
char * textlfosync[] = {"FREE"," KEY","MFRE","MKEY"};
struct submenu lfo1params[] = {
  // name,longname,range,display type,textfield *,parameter number
  "WAVE","Waveform",9,TYPE_TEXT,textlfowaves,160,  // waveform
  "RANG","Range",1,TYPE_TEXT,textlforange,166,  // range
  " SPD","Speed",255,TYPE_NUM,0,161,  
  "SYNC","Sync to Key, Multi",3,TYPE_TEXT,textlfosync,162,   // sync
  "FADE","Fade in Rate",255,TYPE_NUM,0,163,  // fade
  "PITC","Pitch Depth",255,TYPE_NUM,0,164,  
  " AMP","Amplitude Depth",255,TYPE_NUM,0,165,  // amp depth 
  "--->","More on the right ->",255,TYPE_NONE,0,DUMMY,  // empty 
  "PWHL","Mod Wheel Pitch",255,TYPE_NUM,0,181,  // mod wheel pitch
  "PAFT","Aftertouch Pitch",255,TYPE_NUM,0,180,  // aftertouch pitch
  "AWHL","Mod Wheel Amp.",255,TYPE_NUM,0,193,  // mod wheel amp
  "AAFT","Aftertouch Amp.",255,TYPE_NUM,0,192,  // aftertouch amp
};

struct submenu lfo2params[] = {
  // name,longname,range,display type,textfield *,parameter number
  "WAVE","Waveform",9,TYPE_TEXT,textlfowaves,170,  // waveform
  "RANG","Range",1,TYPE_TEXT,textlforange,176,  // range
  " SPD","Speed",255,TYPE_NUM,0,171,  // speed/rate
  "SYNC","Sync to Key, Multi",3,TYPE_TEXT,textlfosync,172,   // sync
  "FADE","Fade in Rate",255,TYPE_NUM,0,173,  // fade
  "  PW","Pulse Width Depth",255,TYPE_NUM,0,174,  // pw depth
  " CUT","Filter Cut Depth",255,TYPE_NUM,0,175,  
  "--->","More on the right ->",255,TYPE_NONE,0,DUMMY,  // empty 
  "PWHL","Mod Wheel Pulse Width",255,TYPE_NUM,0,185,  // mod wheel pw
  "PAFT","Aftertouch Pulse W.",255,TYPE_NUM,0,184,  // aftertouch pw
  "CWHL","Mod Wheel Filter Cut",255,TYPE_NUM,0,189,  // mod wheel cut
  "CAFT","Aftertouch Filt. Cut",255,TYPE_NUM,0,188,  // aftertouch cut
};

// Reverb submenus
char * textrvbmode[] = {"PLAT","HALL"};

struct submenu reverbparams[] = {
  // name,longname,range,display type,textfield *,parameter number
  " WET","Reverb Level",255,TYPE_NUM,0,391,  // wet level
  "MODE","Reverb Type",1,TYPE_TEXT,textrvbmode,392,  // reverb type
  "DCAY","Decay Time",255,TYPE_NUM,0,393,  // decay
  "DAMP","H.F. Damping",255,TYPE_NUM,0,394,  // damping
  " HPF","Tail L.F. Cut",255,TYPE_NUM,0,397,  // hipass
  " SPD","Tail Mod Speed",255,TYPE_NUM,0,395,  // mod speed
  "DPTH","Tail Mod Level",255,TYPE_NUM,0,396,  // mod depth  
};

// Chorus submenus
char * textchorusmode[] = {"MONO","STER","CROS"};

struct submenu chorusparams[] = {
  // name,longname,range,display type,textfield *,parameter number
  " WET","Chorus Level",255,TYPE_NUM,0,361,  // wet level
  "MODE","Chorus Type",2,TYPE_TEXT,textchorusmode,362,  //  type
  " SPD","Speed",255,TYPE_NUM,0,363,  // mod speed
  "DPTH","Depth",255,TYPE_NUM,0,364,  // mod depth 
  "FDBK","Feedback",255,TYPE_NUM,0,365,  // 
  "LRPH","L-R Phase",255,TYPE_NUM,0,366,  // 
};

// phaser submenus
char * textphasermode[] = {"MONO","STER","CROS"};
struct submenu phaserparams[] = {
  // name,longname,range,display type,textfield *,parameter number
  " WET","Phaser Level",255,TYPE_NUM,0,311,  // wet level
  "MODE","Phaser Type",2,TYPE_TEXT,textchorusmode,312,  //  type
  " SPD","Speed",255,TYPE_NUM,0,314,  // mod speed
  "DPTH","Depth",255,TYPE_NUM,0,313,  // mod depth 
  "FDBK","Feedback",255,TYPE_NUM,0,315,  // 
  "OFFS","Offset",255,TYPE_NUM,0,316,  // 
  "STAG","Stages",12,TYPE_NUM,0,317,  // 
  "LRPH","L-R Phase",255,TYPE_NUM,0,318,  // 
};


// amp mod submenus

struct submenu ampmodparams[] = {
  // name,longname,range,display type,textfield *,parameter number
  " WET","Amp. Mod. Level",255,TYPE_NUM,0,330,  // wet level
  " SPD","Speed",255,TYPE_NUM,0,331,  // mod speed
  "RANG","Speed Range",255,TYPE_NUM,0,332,  // 
  "LRPH","L-R Phase",255,TYPE_NUM,0,333,  // 
};

// Delay submenus
char * textdelaymode[] = {"STER","CROS"," LRC"," RLC","MONO",};

struct submenu delayparams[] = {
  // name,longname,range,display type,textfield *,parameter number
  " WET","Delay Level",255,TYPE_NUM,0,301,  // wet level
  "MODE","Delay Type",4,TYPE_TEXT,textdelaymode,302,  
  "TIME","Delay Time",255,TYPE_NUM,0,303,  // 
  "FDBK","Feedback",255,TYPE_NUM,0,304,  // 
  "  LP","Lopass Response",255,TYPE_NUM,0,305,  // 
  "  HP","Hipass Response",255,TYPE_NUM,0,306,  // 
  "TMPO","Tempo",255,TYPE_NUM,0,307,  // hipass
  " MUL","Tempo Multiplier",255,TYPE_NUM,0,308,  // 
  " DIV","Tempo Divider",255,TYPE_NUM,0,309,  // 
  " SPD","Mod Speed",255,TYPE_NUM,0,298,  // mod speed
  "DPTH","Mod Depth",255,TYPE_NUM,0,299,  // mod depth 
  "SMER","Smear",7,TYPE_NUM,0,291,  
  "  2x","2X mode",1,TYPE_TEXT,textoffon,292,     
};

// global submenus
char * textlegatomode[] = {"POLY","MONO",};
char * textportamode[] =  {" OFF","  ON","FING",};
struct submenu globalparams[] = {
  // name,longname,range,display type,textfield *,parameter number
  "TPOS","Transpose",255,TYPE_NUM,0,241,  
  "BNDU","Bend Up Range",10,TYPE_NUM,0,242,  
  "BNDD","Bend Down Range",10,TYPE_NUM,0,243,  // 
  "LEGA","Legato Mode",1,TYPE_TEXT,textlegatomode,244,  // 
  "PMOD","Portamento Mode",2,TYPE_TEXT,textportamode,245,  // 
  "PTIM","Portamento Time",255,TYPE_NUM,0,246,  // 
  " VOL","Volume",255,TYPE_NUM,0,248,  // 
  " PAN","Pan",255,TYPE_NUM,0,247,  
  "VOFF","Velocity Offset",127,TYPE_NUM,0,249,  // 
  "TUNE","Tuning",255,TYPE_NUM,0,251,  
};

// gate submenus

struct submenu gateparams[] = {
  // name,longname,range,display type,textfield *,parameter number
  "ENAB","Gate Enable",1,TYPE_TEXT,textoffon,385,   
  "CURV","Curve Shape",1,TYPE_NUM,0,386,  // mod speed
  "ATTK","Attack",255,TYPE_NUM,0,387,  // 
  "RELS","Release",255,TYPE_NUM,0,388,  // 
};

// arp submenus
char * textarpmode[] = {" OFF","  UP","DOWN","UPDN","PLAY","RAND"};
struct submenu arpparams[] = {
  // name,longname,range,display type,textfield *,parameter number
  "MODE","Arp Mode",5,TYPE_TEXT,textarpmode,450,   
  "TMPO","Tempo (min 44)",255,TYPE_NUM,0,451,  
  " MUL","Tempo Multiplier",10,TYPE_NUM,0,453,  // 
  " OCT","Octaves",10,TYPE_NUM,0,454,  // 
};

// sequencer submenus
//char * textseqsteps[] = {"   1","   2","   3","   4","   5","   6","   7","   8","   9",
//    "  10","  11","  12","  13","  14","  15","  16",};  // maps 0-16 to 1-16
struct submenu seqparams[] = {
  // name,longname,range,display type,textfield *,parameter number
  "ENAB","On/Off",1,TYPE_TEXT,textoffon,428,  
  "VELO","Velocity",127,TYPE_NUM,0,429,  
  "STPS","Seq. Length",16,TYPE_NUM,0,430, 
  "TMPO","Tempo",255,TYPE_NUM,0,431,   
  " MUL","Tempo Multiplier",10,TYPE_NUM,0,432,  // 
  "TPOS","Transpose",255,TYPE_NUM,0,433,  
  "STPS","-->Steps",255,TYPE_NONE,0,DUMMY,  // filler - so steps start on a new screen
  "--->","-->Steps",255,TYPE_NONE,0,DUMMY,  
  "   1","Step 1",255,TYPE_NUM,0,434,     
  "   2","Step 2",255,TYPE_NUM,0,435,  
  "   3","Step 3",255,TYPE_NUM,0,436,  
  "   4","Step 4",255,TYPE_NUM,0,437,  
  "   5","Step 5",255,TYPE_NUM,0,438,     
  "   6","Step 6",255,TYPE_NUM,0,439,  
  "   7","Step 7",255,TYPE_NUM,0,440,  
  "   8","Step 8",255,TYPE_NUM,0,441, 
  "   9","Step 9",255,TYPE_NUM,0,442,     
  "  10","Step 10",255,TYPE_NUM,0,443,  
  "  11","Step 11",255,TYPE_NUM,0,444, 
  "  12","Step 12",255,TYPE_NUM,0,445, 
  "  13","Step 13",255,TYPE_NUM,0,446, 
  "  14","Step 10",255,TYPE_NUM,0,447, 
  "  15","Step 15",255,TYPE_NUM,0,448, 
  "  16","Step 16",255,TYPE_NUM,0,449, 
};

// MIDI submenus
struct submenu midiparams[] = {
  // name,longname,range,display type,textfield *,parameter number
  "PAFT","Pitch Aftertouch",255,TYPE_NUM,0,200,   
  "PRND","Pitch Random",255,TYPE_NUM,0,203, 
  "PWWH","Pulse Width Wheel",255,TYPE_NUM,0,205,  
  "PWAF","Pulse Width Aftertouch",255,TYPE_NUM,0,204, 
  "CTWH","Cutoff Wheel",255,TYPE_NUM,0,209,  
  "CTAF","Cutoff Aftertouch",255,TYPE_NUM,0,208, 
  "VOWH","Volume Wheel",255,TYPE_NUM,0,213,  
  "VOAF","Volume Aftertouch",255,TYPE_NUM,0,212,  
  "CL1L","Control 1 HI",255,TYPE_NUM,0,400,  
  "CL1H","Control 1 LO",255,TYPE_NUM,0,401, 
  "CL2L","Control 2 HI",255,TYPE_NUM,0,402,  
  "CL2H","Control 2 LO",255,TYPE_NUM,0,403, 
  "CL3L","Control 3 HI",255,TYPE_NUM,0,404,  
  "CL3H","Control 3 LO",255,TYPE_NUM,0,405, 
  "CL4L","Control 4 HI",255,TYPE_NUM,0,406,  
  "CL4H","Control 4 LO",255,TYPE_NUM,0,407, 
};

// Effects submenus - groups together stuff that I don't use much on 2nd menu
char * textgain[] = {" 0dB","+6dB","12dB","18dB",};
char * textdtype[] = {"CLIP","SOFT","TUB1","TUB2",};
char * textdfilt[] = {"48kc","20kc","18kc","16kc","14kc","12kc","10kc"," 8kHz",};

struct submenu fxparams[] = {
  // name,longname,range,display type,textfield *,parameter number
  " PRE","Pre FX Gain",3,TYPE_TEXT,textgain,510,   
  "POST","Post FX Gain",3,TYPE_TEXT,textgain,511, 
  " LPF","Lowpass Filter",255,TYPE_NUM,0,320, 
  " HPF","Highpass Filter",255,TYPE_NUM,0,321,  
  "DIST","Distortion On/Off",1,TYPE_TEXT,textoffon,350,  
  "DTYP","Distortion Type",3,TYPE_TEXT,textdtype,354, 
  " PRE","Pre Dist. Gain",255,TYPE_NUM,0,351,  
  "POST","Post Dist. Gain",255,TYPE_NUM,0,352, 
  "FILT","Post Dist. Filter",7,TYPE_TEXT,textdfilt,353,  
  "CRSH","Bitcrusher Depth",24,TYPE_NUM,0,380,  
  "DECI","Decimator Depth",23,TYPE_NUM,0,370,  
};

// performance parameters - quick access to useful parameters
struct submenu perfparams[] = {
  // name,longname,range,type,textfield,parameter#
  " CUT","Filter Cutoff",255,TYPE_NUM,0,72,
  "DPTH","Filter E.G. Depth",255,TYPE_NUM,0,75,
  "RES0","Filter Resonance",255,TYPE_NUM,0,77,
  "TYPE","Filter Type",21,TYPE_TEXT,textfilter,71,  
  "SPD1","LFO1 Speed",255,TYPE_NUM,0,161,  
  "PITC","LFO1 Pitch Depth",255,TYPE_NUM,0,164,  
  "SPD2","LFO2 Speed",255,TYPE_NUM,0,171, 
  " CUT","LFO2 Filt. Cut",255,TYPE_NUM,0,175,  
  "CHOR","Chorus Level",255,TYPE_NUM,0,361,
  "RVRB","Reverb Level",255,TYPE_NUM,0,391,
  "DLAY","Delay Level",255,TYPE_NUM,0,301,
  "PHAS","Phaser Level",255,TYPE_NUM,0,311,
  "  AM","Amp Mod Level",255,TYPE_NUM,0,330,
  " SEQ","Sequencer On/Off",1,TYPE_TEXT,textoffon,428,  
};

// top menus
struct menu {
   char *name; // menu text
   struct submenu * submenus; // points to submenus for this menu
   int8_t submenuindex;   // stores the index of the submenu we are currently using
   int8_t numsubmenus; // number of submenus - not sure why this has to be int but it crashes otherwise. compiler bug?
};

// the top level menus
// these much match the order in the array - used for quick menu navigation
#define OSCS    0
#define FILTERS 6
#define ENVELOPES 7
#define LFOS 4
#define EFFECTS 9

struct menu mainmenu[] = {
  // name,submenu *,initial submenu index,number of submenus

  "Oscillator 1",osc1params,0,sizeof(osc1params)/sizeof(submenu),
  "Oscillator 2",osc2params,0,sizeof(osc2params)/sizeof(submenu),
  "Oscillator 3",osc3params,0,sizeof(osc3params)/sizeof(submenu),
  "Oscillator 4",osc4params,0,sizeof(osc4params)/sizeof(submenu),
  "LFO 1",lfo1params,0,sizeof(lfo1params)/sizeof(submenu),
  "LFO 2",lfo2params,0,sizeof(lfo2params)/sizeof(submenu),
  "Filters",filtparams,0,sizeof(filtparams)/sizeof(submenu),
  "Amplitude Envelope",egampparams,0,sizeof(egampparams)/sizeof(submenu),
  "Filter Envelope",egfiltparams,0,sizeof(egfiltparams)/sizeof(submenu),
  "Reverb",reverbparams,0,sizeof(reverbparams)/sizeof(submenu),
  "Delay",delayparams,0,sizeof(delayparams)/sizeof(submenu),
  "Chorus/Flanger",chorusparams,0,sizeof(chorusparams)/sizeof(submenu),
  "Phaser",phaserparams,0,sizeof(phaserparams)/sizeof(submenu),
  "Init Patch(Dub Clik)",initparams,0,sizeof(initparams)/sizeof(submenu),
  "Save Patch(Dub Clik)",saveparams,0,sizeof(saveparams)/sizeof(submenu),
  "Load Patch",loadparams,0,sizeof(loadparams)/sizeof(submenu),
};

#define NUM_MAIN_MENUS sizeof(mainmenu)/ sizeof(menu)

// second menu for less used parameters

struct menu secondarymenu[] = {
  // name,submenu *,initial submenu index,number of submenus
  "* Performance ",perfparams,0,sizeof(perfparams)/sizeof(submenu),
  "* Global Params",globalparams,0,sizeof(globalparams)/sizeof(submenu),
  "* Amplitude Modulator",ampmodparams,0,sizeof(ampmodparams)/sizeof(submenu),
  "* Arpeggiator",arpparams,0,sizeof(arpparams)/sizeof(submenu),
  "* Sequencer",seqparams,0,sizeof(seqparams)/sizeof(submenu),
  "* Gate",gateparams,0,sizeof(gateparams)/sizeof(submenu),
  "* Pitch Envelope",egpitchparams,0,sizeof(egpitchparams)/sizeof(submenu),
  "* Additional Effects",fxparams,0,sizeof(fxparams)/sizeof(submenu),
  "* MIDI",midiparams,0,sizeof(midiparams)/sizeof(submenu),
};

#define NUM_SECONDARY_MENUS sizeof(secondarymenu)/ sizeof(menu)
