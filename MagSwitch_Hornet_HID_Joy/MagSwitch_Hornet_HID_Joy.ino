/*
  Tell DCS-BIOS to use a serial connection and use the default Arduino Serial
  library. This will work on the vast majority of Arduino-compatible boards,
  but you can get corrupted data if you have too many or too slow outputs
  (e.g. when you have multiple character displays), because the receive
  buffer can fill up if the sketch spends too much time updating them.
  
  If you can, use the IRQ Serial connection instead.
*/

/*
  HORNET MAG SWITCH HID WORKAROUND EXAMPLE BY DEADMEAT (Joystick v1.1)
  - Joystick version: throw physical switch to trigger HID joystick button press. Assign to appropriate command in DCS

  ALL HORNET MAG SWITCHES SUPPORTED
  - APU
  - LTD/R
  - HOOK BYPASS
  - LAUNCH BAR
  - PITOT HEAT
  - AUG PULL
  - FUEL DUMP (special, has manual override mode for dumping fuel below BINGO etc.)
  - ENGINE CRANK
  - MC (Mission Computer)
  - CANOPY (special, only held in up position)
  - GOUND POWER 1-4 (special, any first switch is held for 3,1 seconds to allow hold-in of rest)
  
  KNOWN ISSUES
  - No switch debounce
  - Blocking button press code
  - Ground power 4 is bugged in DCS. Allow hold-in with powering systems.
  - Will need to add board ID if using multiple boards. See Joystick library documentation.
  - If no other joystick is plugged in, DCS assigns this board to x/y/z axes. Clear axes bindings to resolve
*/

#define DCSBIOS_DEFAULT_SERIAL

#include "DcsBios.h"

#include <Joystick.h>
Joystick_ Joystick; // Create a joystick

//***GENERAL SETTINGS
const int pressDuration = 100; //ms to hold joystick button pressed

//****************2-POSITION MAG SWITCH DEFINITIONS
//****************APU SWITCH DEFINITIONS
int PressApu = 0;                  //count if APU has been pushed
int StateApu = 0;                  //APU switch position
byte SimApu = 0;                   // APU sim switch position (may not be in this position at start)

int JoyButtonApu = 0;              //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)

#define APU_ON_PIN          4
#define APU_MOSFET_PIN      5

//****************LTDR SWITCH DEFINITIONS
int PressLTDR = 0;                //Has switch been pushed?
int StateLTDR = 0;                //Switch position
byte SimLTDR = 0;                 //Switch poisition in DCS

int JoyButtonLTDR = 1;            //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)

#define LTDR_ON_PIN         4
#define LTDR_MOSFET_PIN     5

//****************HOOK BYPASS SWITCH DEFINITIONS
int PressHookBp = 0;                //Has switch been pushed?
int StateHookBp = 0;                //Switch position
byte SimHookBp = 0;                 //Switch poisition in DCS

int JoyButtonHookBp = 2;            //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)

#define HOOKBP_ON_PIN         4
#define HOOKBP_MOSFET_PIN     5

//****************LAUNCH BAR SWITCH DEFINITIONS
int PressLbar = 0;                //Has switch been pushed?
int StateLbar = 0;                //Switch position
byte SimLbar = 0;                 //Switch poisition in DCS

int JoyButtonLbar = 3;            //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)

#define LBAR_ON_PIN         4
#define LBAR_MOSFET_PIN     5

//****************PITOT HEAT SWITCH DEFINITIONS
int PressPitot = 0;                //Has switch been pushed?
int StatePitot = 0;                //Switch position
byte SimPitot = 0;                 //Switch poisition in DCS

int JoyButtonPitot = 4;            //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)

#define PITOT_ON_PIN         4
#define PITOT_MOSFET_PIN     5

//****************AUG PULL SWITCH DEFINITIONS
int PressAugPull = 0;                //Has switch been pushed?
int StateAugPull = 0;                //Switch position
byte SimAugPull = 0;                 //Switch poisition in DCS

int JoyButtonAugPull = 5;            //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)

#define AUGPULL_ON_PIN         4
#define AUGPULL_MOSFET_PIN     5

//****************FUEL DUMP SWITCH DEFINITIONS
int PressDump = 0;                //Has switch been pushed?
int StateDump = 0;                //Switch position
byte SimDump = 0;                 //Switch poisition in DCS

int JoyButtonDump = 6;            //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)

#define DUMP_ON_PIN         2
#define DUMP_MOSFET_PIN     3

//****************3-POSITION MAG SWITCH DEFINITIONS
//****************CRANK SWITCH DEFINITIONS
int PressCrank = 0;               //count if crank has been pushed
int StateCrank = 1;               //Crank switch position

int JoyButtonCrankLeft = 7;       //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)
int JoyButtonCrankOff = 8;
int JoyButtonCrankRight = 9;

#define CRANK_ON_LEFT_PIN   10
#define CRANK_ON_RIGHT_PIN  11
#define CRANK_MOSFET_PIN    12

//****************MC SWITCH DEFINITIONS
int PressMc = 0;               //count if switch has been pushed
int StateMc = 1;               //Switch position

int JoyButtonMc1Off = 10;       //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)
int JoyButtonMcNorm = 11;
int JoyButtonMc2Off = 12;

#define MC_1_OFF_PIN       10   //Up position
#define MC_2_OFF_PIN       11   //Down position
#define MC_MOSFET_PIN      12

//****************CANOPY SWITCH DEFINITIONS
int PressCanopy = 0;               //count if switch has been pushed
int StateCanopy = 1;               //Switch position

int JoyButtonCanopyDown = 13;       //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)
int JoyButtonCanopyOff = 14;
int JoyButtonCanopyUp = 15;

#define CANOPY_DOWN_PIN       10   //Up position
#define CANOPY_UP_PIN         11   //Down position
#define CANOPY_MOSFET_PIN     12

//****************GROUND POWER 1 SWITCH DEFINITIONS
int PressGP1 = 0;               //count if switch has been pushed
int StateGP1 = 1;               //Switch position
int SimGP1 = 1;                 //DCS switch position

int JoyButtonGP1A = 16;       //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)
int JoyButtonGP1Auto = 17;
int JoyButtonGP1B = 18;

#define GP1_A_PIN           10   //Up position
#define GP1_B_PIN           11   //Down position
#define GP1_MOSFET_PIN      12

//****************GROUND POWER 2 SWITCH DEFINITIONS
int PressGP2 = 0;               //count if switch has been pushed
int StateGP2 = 1;               //Switch position
int SimGP2 = 1;                 //DCS switch position

int JoyButtonGP2A = 19;       //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)
int JoyButtonGP2Auto = 20;
int JoyButtonGP2B = 21;

#define GP2_A_PIN           10   //Up position
#define GP2_B_PIN           11   //Down position
#define GP2_MOSFET_PIN      12

//****************GROUND POWER 3 SWITCH DEFINITIONS
int PressGP3 = 0;               //count if switch has been pushed
int StateGP3 = 1;               //Switch position
int SimGP3 = 1;                 //DCS switch position

int JoyButtonGP3A = 22;       //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)
int JoyButtonGP3Auto = 23;
int JoyButtonGP3B = 24;

#define GP3_A_PIN           8   //Up position
#define GP3_B_PIN           7   //Down position
#define GP3_MOSFET_PIN      9

//****************GROUND POWER 4 SWITCH DEFINITIONS
int PressGP4 = 0;               //count if switch has been pushed
int StateGP4 = 1;               //Switch position
int SimGP4 = 1;                 //DCS switch position

int JoyButtonGP4A = 25;       //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)
int JoyButtonGP4Auto = 26;
int JoyButtonGP4B = 27;

#define GP4_A_PIN           10   //Up position
#define GP4_B_PIN           11   //Down position
#define GP4_MOSFET_PIN      12

/* paste code snippets from the reference documentation here */

//****************2-POSITION MAG SWITCH OUTPUT
//****************APU switch position output
DcsBios::LED apuControl_Sw(0x74c2, 0x0100, APU_MOSFET_PIN); //Added underscore to name. APU switch in pos 1, send 5v to MOSFET and 12v to APU solenoid. 

void onApuControlSwChange(unsigned int newValue) {
    SimApu = newValue;
}
DcsBios::IntegerBuffer apuControlSwBuffer(0x74c2, 0x0100, 8, onApuControlSwChange);

//****************LTDR switch position output
DcsBios::LED ltdRSw_(0x74c8, 0x4000, LTDR_MOSFET_PIN); //Added underscore to name. activates MOSFET

void onLtdRSwChange(unsigned int newValue) {
    SimLTDR = newValue;
}
DcsBios::IntegerBuffer ltdRSwBuffer(0x74c8, 0x4000, 14, onLtdRSwChange);

//****************HOOK BYPASS switch position output
DcsBios::LED hookBypassSw_(0x7480, 0x4000, HOOKBP_MOSFET_PIN); //Added underscore to name. activates MOSFET

void onHookBypassSwChange(unsigned int newValue) {
    SimHookBp = newValue;
}
DcsBios::IntegerBuffer hookBypassSwBuffer(0x7480, 0x4000, 14, onHookBypassSwChange);

//****************LAUNCH BAR switch position output
DcsBios::LED launchBarSw_(0x7480, 0x2000, LBAR_MOSFET_PIN); //Added underscore to name. activates MOSFET

void onLaunchBarSwChange(unsigned int newValue) {
    SimLbar = newValue;
}
DcsBios::IntegerBuffer launchBarSwBuffer(0x7480, 0x2000, 13, onLaunchBarSwChange);

//****************PITOT HEAT switch position output
DcsBios::LED pitotHeatSw_(0x74c8, 0x0100, PITOT_MOSFET_PIN); //Added underscore to name. activates MOSFET

void onPitotHeatSwChange(unsigned int newValue) {
    SimPitot = newValue;
}
DcsBios::IntegerBuffer pitotHeatSwBuffer(0x74c8, 0x0100, 8, onPitotHeatSwChange);

//****************AUG PULL switch position output
DcsBios::LED bleedAirPull_(0x74c4, 0x8000, AUGPULL_MOSFET_PIN); //Added underscore to name. activates MOSFET

void onBleedAirPullChange(unsigned int newValue) {
    SimAugPull = newValue;
}
DcsBios::IntegerBuffer bleedAirPullBuffer(0x74c4, 0x8000, 15, onBleedAirPullChange);

//****************FUEL DUMP switch position output
void onFuelDumpSwChange(unsigned int newValue) {
    SimDump = newValue;
    if(PressDump == 1 && SimDump == 0){ //Physical switch on, but DCS turned off
      PressDump = 2; //We need go into manual mode
    }
    if(PressDump == 1 && SimDump == 1){ //physical switch is on and DCS stays on in first attempt
      digitalWrite(DUMP_MOSFET_PIN, HIGH);
    }
    if(PressDump == 0 && SimDump == 0){ //DCS and physical switches are off
      digitalWrite(DUMP_MOSFET_PIN, LOW);
    }
    if(PressDump == 0 && SimDump == 1){ //Switch turned on in DCS before physical
      digitalWrite(DUMP_MOSFET_PIN, HIGH);
    }
}
DcsBios::IntegerBuffer fuelDumpSwBuffer(0x74b4, 0x0100, 8, onFuelDumpSwChange);

//****************3-POSITION MAG SWITCH OUTPUT
//****************CRANK switch position output
void onEngineCrankSwChange(unsigned int newValue) {
    if (newValue == 1){ //if crank switch in middle position
      digitalWrite(CRANK_MOSFET_PIN, LOW); 
    }
    else{//if crank switch in left/right
      digitalWrite(CRANK_MOSFET_PIN, HIGH); //5v to activate MOSFET and feed 12v to crank switch solenoid
    }
}
DcsBios::IntegerBuffer engineCrankSwBuffer(0x74c2, 0x0600, 9, onEngineCrankSwChange);

//****************MC switch position output
void onMcSwChange(unsigned int newValue) {
    if (newValue == 1){ //if switch is in middle position
      digitalWrite(MC_MOSFET_PIN, LOW); 
    }
    else{//if switch is in up/down
      digitalWrite(MC_MOSFET_PIN, HIGH); //5v to activate MOSFET and feed 12v to switch solenoid
    }
}
DcsBios::IntegerBuffer mcSwBuffer(0x74c0, 0x0600, 9, onMcSwChange);

//****************CANOPY switch position output
void onCanopySwChange(unsigned int newValue) {
    if (newValue == 2){ //if switch is in up position
      digitalWrite(CANOPY_MOSFET_PIN, HIGH); 
    }
    else{//if switch is in middle or down position
      digitalWrite(CANOPY_MOSFET_PIN, LOW); //5v to activate MOSFET and feed 12v to switch solenoid
    }
}
DcsBios::IntegerBuffer canopySwBuffer(0x74ce, 0x0300, 8, onCanopySwChange);

//****************GROUND POWER 1 switch position output
void onGndPwr1SwChange(unsigned int newValue) {
    if (newValue == 1){ //if switch is in middle position
      digitalWrite(GP1_MOSFET_PIN, LOW); 
    }
    else{//if switch is in up/down
      digitalWrite(GP1_MOSFET_PIN, HIGH); //5v to activate MOSFET and feed 12v to switch solenoid
    }
    SimGP1 = newValue;
}
DcsBios::IntegerBuffer gndPwr1SwBuffer(0x74ac, 0x3000, 12, onGndPwr1SwChange);

//****************GROUND POWER 2 switch position output
void onGndPwr2SwChange(unsigned int newValue) {
    if (newValue == 1){ //if switch is in middle position
      digitalWrite(GP2_MOSFET_PIN, LOW); 
    }
    else{//if switch is in up/down
      digitalWrite(GP2_MOSFET_PIN, HIGH); //5v to activate MOSFET and feed 12v to switch solenoid
    }
    SimGP2 = newValue;
}
DcsBios::IntegerBuffer gndPwr2SwBuffer(0x74ac, 0xc000, 14, onGndPwr2SwChange);

//****************GROUND POWER 3 switch position output
void onGndPwr3SwChange(unsigned int newValue) {
    if (newValue == 1){ //if switch is in middle position
      digitalWrite(GP3_MOSFET_PIN, LOW); 
    }
    else{//if switch is in up/down
      digitalWrite(GP3_MOSFET_PIN, HIGH); //5v to activate MOSFET and feed 12v to switch solenoid
    }
    SimGP3 = newValue;
}
DcsBios::IntegerBuffer gndPwr3SwBuffer(0x74b0, 0x0300, 8, onGndPwr3SwChange);

//****************GROUND POWER 4 switch position output
void onGndPwr4SwChange(unsigned int newValue) {
    if (newValue == 1){ //if switch is in middle position
      digitalWrite(GP4_MOSFET_PIN, LOW); 
    }
    else{//if switch is in up/down
      digitalWrite(GP4_MOSFET_PIN, HIGH); //5v to activate MOSFET and feed 12v to switch solenoid
    }
    SimGP4 = newValue;
}
DcsBios::IntegerBuffer gndPwr4SwBuffer(0x74b0, 0x0c00, 10, onGndPwr4SwChange);

//****************DISABLE SOLENOIDS WHEN EXITING MISSION
void onAcftNameChange(char* newValue) {
    digitalWrite(APU_MOSFET_PIN, LOW);
    digitalWrite(LTDR_MOSFET_PIN, LOW);
    digitalWrite(HOOKBP_MOSFET_PIN, LOW);
    digitalWrite(LBAR_MOSFET_PIN, LOW);
    digitalWrite(PITOT_MOSFET_PIN, LOW);
    digitalWrite(AUGPULL_MOSFET_PIN, LOW);
    digitalWrite(DUMP_MOSFET_PIN, LOW);
    digitalWrite(CRANK_MOSFET_PIN, LOW);
    digitalWrite(MC_MOSFET_PIN, LOW);
    digitalWrite(CANOPY_MOSFET_PIN, LOW);
    digitalWrite(GP1_MOSFET_PIN, LOW);
    digitalWrite(GP2_MOSFET_PIN, LOW);
    digitalWrite(GP3_MOSFET_PIN, LOW);
    digitalWrite(GP4_MOSFET_PIN, LOW);
}
DcsBios::StringBuffer<24> AcftNameBuffer(0x0000, onAcftNameChange);

void setup() {
  //****************2-POSITION MAG SWITCH SETUP
  //****************APU SETUP
  pinMode(APU_ON_PIN, INPUT_PULLUP);
  
  //****************LTDR SETUP
  pinMode(LTDR_ON_PIN, INPUT_PULLUP);

  //****************HOOK BYPASS SETUP
  pinMode(HOOKBP_ON_PIN, INPUT_PULLUP);

  //****************LAUNCH BAR SETUP
  pinMode(LBAR_ON_PIN, INPUT_PULLUP);

  //****************PITOT HEAT SETUP
  pinMode(PITOT_ON_PIN, INPUT_PULLUP);

  //****************AUG PULL SETUP
  pinMode(AUGPULL_ON_PIN, INPUT_PULLUP);

  //****************FUEL DUMP SETUP
  pinMode(DUMP_ON_PIN, INPUT_PULLUP);
  pinMode(DUMP_MOSFET_PIN, OUTPUT);

  //****************3-POSITION MAG SWITCH SETUP
  //****************CRANK SETUP
  pinMode(CRANK_ON_LEFT_PIN, INPUT_PULLUP);
  pinMode(CRANK_ON_RIGHT_PIN, INPUT_PULLUP);
  pinMode(CRANK_MOSFET_PIN, OUTPUT);

  //****************MC SETUP
  pinMode(MC_1_OFF_PIN, INPUT_PULLUP);
  pinMode(MC_2_OFF_PIN, INPUT_PULLUP);
  pinMode(MC_MOSFET_PIN, OUTPUT);

  //***************CANOPY SETUP
  pinMode(CANOPY_UP_PIN, INPUT_PULLUP);
  pinMode(CANOPY_DOWN_PIN, INPUT_PULLUP);
  pinMode(CANOPY_MOSFET_PIN, OUTPUT);

  //***************GROUND POWER 1 SETUP
  pinMode(GP1_A_PIN, INPUT_PULLUP);
  pinMode(GP1_B_PIN, INPUT_PULLUP);
  pinMode(GP1_MOSFET_PIN, OUTPUT);

  //***************GROUND POWER 2 SETUP
  pinMode(GP2_A_PIN, INPUT_PULLUP);
  pinMode(GP2_B_PIN, INPUT_PULLUP);
  pinMode(GP2_MOSFET_PIN, OUTPUT);

  //***************GROUND POWER 3 SETUP
  pinMode(GP3_A_PIN, INPUT_PULLUP);
  pinMode(GP3_B_PIN, INPUT_PULLUP);
  pinMode(GP3_MOSFET_PIN, OUTPUT);

  //***************GROUND POWER 4 SETUP
  pinMode(GP4_A_PIN, INPUT_PULLUP);
  pinMode(GP4_B_PIN, INPUT_PULLUP);
  pinMode(GP4_MOSFET_PIN, OUTPUT);

  Joystick.begin();

  DcsBios::setup();
}

void loop() {
  //****************2-POSITION MAG SWITCH LOGIC
  //****************APU SWITCH LOGIC
  StateApu = !digitalRead(APU_ON_PIN); //1 if in ON, 0 if in OFF. Inverts the read signal for correct state

  if (StateApu == 1 && PressApu == 0){//APU switch moved to on
    if (SimApu == 0) {
      Joystick.pressButton(JoyButtonApu);
      delay(pressDuration);
      Joystick.releaseButton(JoyButtonApu); //release keys
    }
    PressApu = 1; //APU switch has been manually moved to on
  }

  if (StateApu == 0 && PressApu == 1){//APU switch manually moved to off
    if (SimApu == 1) {
      Joystick.pressButton(JoyButtonApu);
      delay(pressDuration);
      Joystick.releaseButton(JoyButtonApu); //release keys
    }
    PressApu = 0; //APU switch is no longer in on
  }

  if (StateApu == 0){//APU switch released to off in DCS
    Joystick.releaseButton(JoyButtonApu); //release keys
    PressApu = 0;
  }

  //****************LTDR SWITCH LOGIC
  StateLTDR = !digitalRead(LTDR_ON_PIN); //1 if in ON, 0 if in OFF. Inverts the read signal for correct state

  if (StateLTDR == 1 && PressLTDR == 0){//LTDR switch moved to on
    if (SimLTDR == 0) {
      Joystick.pressButton(JoyButtonLTDR);
      delay(pressDuration);
      Joystick.releaseButton(JoyButtonLTDR); //release keys
    }
    PressLTDR = 1; //LTDR switch has been manually moved to on
  }

  //test if we can detect switch being moved to off
  if (StateLTDR == 0 && PressLTDR == 1){//LTDR switch manually moved to off
    if (SimLTDR == 1) {
      Joystick.pressButton(JoyButtonLTDR);
      delay(pressDuration);
      Joystick.releaseButton(JoyButtonLTDR); //release keys
    }
    PressLTDR = 0; //LTDR switch is no longer in on
  }

  if (StateLTDR == 0){//LTDR switch released to off in DCS
    Joystick.releaseButton(JoyButtonLTDR); //release keys
    PressLTDR = 0;
  }

  //****************HOOK BYPASS SWITCH LOGIC
  StateHookBp = !digitalRead(HOOKBP_ON_PIN); //1 if in ON, 0 if in OFF. Inverts the read signal for correct state

  if (StateHookBp == 1 && PressHookBp == 0){//Switch moved to on
    if (SimHookBp == 0) {
      Joystick.pressButton(JoyButtonHookBp);
      delay(pressDuration);
      Joystick.releaseButton(JoyButtonHookBp); //release keys
    }
    PressHookBp = 1; //Switch has been manually moved to on
  }

  //test if we can detect switch being moved to off
  if (StateHookBp == 0 && PressHookBp == 1){//Switch manually moved to off
    if (SimHookBp == 1) {
      Joystick.pressButton(JoyButtonHookBp);
      delay(pressDuration);
      Joystick.releaseButton(JoyButtonHookBp); //release keys
    }
    PressHookBp = 0; //Switch is no longer in on
  }

  if (StateHookBp == 0){//Switch released to off in DCS
    Joystick.releaseButton(JoyButtonHookBp); //release keys
    PressHookBp = 0;
  }

  //****************LAUNCH BAR SWITCH LOGIC
  StateLbar = !digitalRead(LBAR_ON_PIN); //1 if in ON, 0 if in OFF. Inverts the read signal for correct state

  if (StateLbar == 1 && PressLbar == 0){//Switch moved to on
    if (SimLbar == 0) {
      Joystick.pressButton(JoyButtonLbar);
      delay(pressDuration);
      Joystick.releaseButton(JoyButtonLbar); //release keys
    }
    PressLbar = 1; //Switch has been manually moved to on
  }

  //test if we can detect switch being moved to off
  if (StateLbar == 0 && PressLbar == 1){//Switch manually moved to off
    if (SimLbar == 1) {
      Joystick.pressButton(JoyButtonLbar);
      delay(pressDuration);
      Joystick.releaseButton(JoyButtonLbar); //release keys
    }
    PressLbar = 0; //Switch is no longer in on
  }

  if (StateLbar == 0){//Switch released to off in DCS
    Joystick.releaseButton(JoyButtonLbar); //release keys
    PressLbar = 0;
  }

  //****************AUG PULL SWITCH LOGIC
  StateAugPull = !digitalRead(AUGPULL_ON_PIN); //1 if in ON, 0 if in OFF. Inverts the read signal for correct state

  if (StateAugPull == 1 && PressAugPull == 0){//Switch moved to on
    if (SimAugPull == 0) {
      Joystick.pressButton(JoyButtonAugPull);
      delay(pressDuration);
      Joystick.releaseButton(JoyButtonAugPull); //release keys
    }
    PressAugPull = 1; //Switch has been manually moved to on
  }

  //test if we can detect switch being moved to off
  if (StateAugPull == 0 && PressAugPull == 1){//Switch manually moved to off
    if (SimAugPull == 1) {
      Joystick.pressButton(JoyButtonAugPull);
      delay(pressDuration);
      Joystick.releaseButton(JoyButtonAugPull); //release keys
    }
    PressAugPull = 0; //Switch is no longer in on
  }

  if (StateAugPull == 0){//Switch released to off in DCS
    Joystick.releaseButton(JoyButtonAugPull); //release keys
    PressAugPull = 0;
  }

  //****************FUEL DUMP SWITCH LOGIC
  StateDump = !digitalRead(DUMP_ON_PIN); //1 if in ON, 0 if in OFF. Inverts the read signal for correct state

  if (StateDump == 1 && PressDump == 0){//Switch moved to on, first try
    if (SimDump == 0) {
      Joystick.pressButton(JoyButtonDump);
      delay(pressDuration);
      Joystick.releaseButton(JoyButtonDump); //release keys
    }
    PressDump = 1; //First attempt
  }

  //test if we can detect switch being moved to off
  if (StateDump == 0 && (PressDump == 1 | PressDump == 3)){//Switch manually moved to off
    if (SimDump == 1) {
      Joystick.releaseButton(JoyButtonDump); //just in case it was already held in manually
      Joystick.pressButton(JoyButtonDump);
      delay(pressDuration);
      Joystick.releaseButton(JoyButtonDump); //release keys
    }
    PressDump = 0; //Switch is no longer in on
  }

  //Test if we can use manual mode
  if (StateDump == 1 && PressDump == 2){ //Holding physical switch in, but dcs turned off
    Joystick.pressButton(JoyButtonDump); //Turn it on again
    digitalWrite(DUMP_MOSFET_PIN, LOW); //Disable solenoid (so switch pops back if released)
    PressDump = 3; //We've turned it on and are holding it in
  }

  if (StateDump == 0){//Switch released to off in DCS
    Joystick.releaseButton(JoyButtonDump); //release keys
    PressDump = 0;
  }

  //****************3-POSITION MAG SWITCH LOGIC
  //****************CRANK SWITCH LOGIC
  if (digitalRead(CRANK_ON_LEFT_PIN) == LOW){ //read switch state without debounce
    StateCrank = 0; //Crank switch is in left position
  }

  if (digitalRead(CRANK_ON_RIGHT_PIN) == LOW){//Crank switch is in right position
    StateCrank = 2;
  }

  if (digitalRead(CRANK_ON_LEFT_PIN) == HIGH && digitalRead(CRANK_ON_RIGHT_PIN) == HIGH){//Crank switch is in center
    StateCrank = 1;
  }

  if (StateCrank == 0 && PressCrank == 0){//Crank switch is moved to left on
    Joystick.pressButton(JoyButtonCrankLeft);
    delay(pressDuration);
    Joystick.releaseButton(JoyButtonCrankLeft); //release keys
    PressCrank = 1; //Switch has been manually moved left/right
  }

  if (StateCrank == 2 && PressCrank == 0){//Crank switch is moved to right on
    Joystick.pressButton(JoyButtonCrankRight);
    delay(pressDuration);
    Joystick.releaseButton(JoyButtonCrankRight); //release keys
    PressCrank = 1; //Switch has been manually moved left/right
  }

  //test if we can detect switch being moved to off
  if (StateCrank == 1 && PressCrank == 1){//Crank switch manually moved to center
    Joystick.pressButton(JoyButtonCrankOff);
    delay(pressDuration);
    Joystick.releaseButton(JoyButtonCrankOff); //release keys
    PressCrank = 0; //Crank switch is no longer left/right
  }

  if (StateCrank == 1){//Crank switch released to center in DCS
    Joystick.releaseButton(JoyButtonCrankLeft); //release keys  
    Joystick.releaseButton(JoyButtonCrankOff);
    Joystick.releaseButton(JoyButtonCrankRight);
    PressCrank = 0; //Crank switch is no longer left/right
  }

  //****************MC SWITCH LOGIC
  if (digitalRead(MC_1_OFF_PIN) == LOW){ //read switch state without debounce
    StateMc = 0; //Switch is in up position
  }

  if (digitalRead(MC_2_OFF_PIN) == LOW){//Switch is in down position
    StateMc = 2;
  }

  if (digitalRead(MC_1_OFF_PIN) == HIGH && digitalRead(MC_2_OFF_PIN) == HIGH){//Switch is in center position
    StateMc = 1;
  }

  if (StateMc == 0 && PressMc == 0){//Switch is moved to up position
    Joystick.pressButton(JoyButtonMc1Off);
    delay(pressDuration);
    Joystick.releaseButton(JoyButtonMc1Off); //release keys
    PressMc = 1; //Switch has been manually moved up/down
  }

  if (StateMc == 2 && PressMc == 0){//Switch is moved to down position
    Joystick.pressButton(JoyButtonMc2Off);
    delay(pressDuration);
    Joystick.releaseButton(JoyButtonMc2Off); //release keys
    PressMc = 1; //Switch has been manually moved up/down
  }

  //test if we can detect switch being moved to off
  if (StateMc == 1 && PressMc == 1){//Switch manually moved to center
    Joystick.pressButton(JoyButtonMcNorm);
    delay(pressDuration);
    Joystick.releaseButton(JoyButtonMcNorm); //release keys
    PressMc = 0; //Switch is no longer up/down
  }

  if (StateMc == 1){//Switch released to center in DCS
    Joystick.releaseButton(JoyButtonMc1Off); //release keys  
    Joystick.releaseButton(JoyButtonMcNorm);
    Joystick.releaseButton(JoyButtonMc2Off);
    PressMc = 0; //Switch is no longer up/down
  }

  //****************CANOPY SWITCH LOGIC
  if (digitalRead(CANOPY_DOWN_PIN) == LOW){ //read switch state without debounce
    StateCanopy = 0; //Switch is in down position
  }

  if (digitalRead(CANOPY_UP_PIN) == LOW){//Switch is in up position
    StateCanopy = 2;
  }

  if (digitalRead(CANOPY_DOWN_PIN) == HIGH && digitalRead(CANOPY_UP_PIN) == HIGH){//Switch is in center position
    StateCanopy = 1;
  }

  if (StateCanopy == 0 && PressCanopy == 0){//Switch is moved to down position
    Joystick.pressButton(JoyButtonCanopyDown);
    PressCanopy = 1; //Switch has been manually moved up/down
  }

  if (StateCanopy == 2 && PressCanopy == 0){//Switch is moved to up position
    Joystick.releaseButton(JoyButtonCanopyDown);
    Joystick.pressButton(JoyButtonCanopyUp);
    delay(pressDuration);
    Joystick.releaseButton(JoyButtonCanopyUp); //release keys
    PressCanopy = 1; //Switch has been manually moved up/down
  }

  //test if we can detect switch being moved to off
  if (StateCanopy == 1 && PressCanopy == 1){//Switch manually moved to center
    Joystick.releaseButton(JoyButtonCanopyDown);
    Joystick.pressButton(JoyButtonCanopyOff);
    delay(pressDuration);
    Joystick.releaseButton(JoyButtonCanopyOff); //release keys
    PressCanopy = 0; //Switch is no longer up/down
  }

  if (StateCanopy == 1){//Switch released to center in DCS
    Joystick.releaseButton(JoyButtonCanopyDown); //release keys  
    Joystick.releaseButton(JoyButtonCanopyOff);
    Joystick.releaseButton(JoyButtonCanopyUp);
    PressCanopy = 0; //Switch is no longer up/down
  }

  //****************GROUND POWER 1 SWITCH LOGIC
  if (digitalRead(GP1_A_PIN) == LOW){ //read switch state without debounce
    StateGP1 = 0; //Switch is in up position
  }

  if (digitalRead(GP1_B_PIN) == LOW){//Switch is in down position
    StateGP1 = 2;
  }

  if (digitalRead(GP1_A_PIN) == HIGH && digitalRead(GP1_B_PIN) == HIGH){//Switch is in center position
    StateGP1 = 1;
  }

  if (StateGP1 == 0 && PressGP1 == 0){//Switch is moved to up position
    Joystick.pressButton(JoyButtonGP1A);
    if (SimGP2 == 1 && SimGP3 == 1 && SimGP4 == 1){ //Check if other ground power switches are off
      delay(3100); //First ground power switch needs 3 second hold-in to activate
    }
    else{ //one of the other switches are already on
      delay(pressDuration); //Normal delay to activate solenoid
    }
    Joystick.releaseButton(JoyButtonGP1A); //release keys
    PressGP1 = 1; //Switch has been manually moved up/down
  }

  if (StateGP1 == 2 && PressGP1 == 0){//Switch is moved to down position
    Joystick.pressButton(JoyButtonGP1B);
    if (SimGP2 == 1 && SimGP3 == 1 && SimGP4 == 1){ //Check if other ground power switches are off
      delay(3100); //First ground power switch needs 3 second hold-in to activate
    }
    else{ //one of the other switches are already on
      delay(pressDuration); //Normal delay to activate solenoid
    }      
    Joystick.releaseButton(JoyButtonGP1B); //release keys
    PressGP1 = 1; //Switch has been manually moved up/down
  }

  //test if we can detect switch being moved to off
  if (StateGP1 == 1 && PressGP1 == 1){//Switch manually moved to center
    Joystick.pressButton(JoyButtonGP1Auto);
    delay(pressDuration);
    Joystick.releaseButton(JoyButtonGP1Auto); //release keys
    PressGP1 = 0; //Switch is no longer up/down
  }

  if (StateGP1 == 1){//Switch released to center in DCS
    Joystick.releaseButton(JoyButtonGP1A); //release keys  
    Joystick.releaseButton(JoyButtonGP1Auto);
    Joystick.releaseButton(JoyButtonGP1B);
    PressGP1 = 0; //Switch is no longer up/down
  }

  //****************GROUND POWER 2 SWITCH LOGIC
  if (digitalRead(GP2_A_PIN) == LOW){ //read switch state without debounce
    StateGP2 = 0; //Switch is in up position
  }

  if (digitalRead(GP2_B_PIN) == LOW){//Switch is in down position
    StateGP2 = 2;
  }

  if (digitalRead(GP2_A_PIN) == HIGH && digitalRead(GP2_B_PIN) == HIGH){//Switch is in center position
    StateGP2 = 1;
  }

  if (StateGP2 == 0 && PressGP2 == 0){//Switch is moved to up position
    Joystick.pressButton(JoyButtonGP2A);
    if (SimGP1 == 1 && SimGP3 == 1 && SimGP4 == 1){ //Check if other ground power switches are off
      delay(3100); //First ground power switch needs 3 second hold-in to activate
    }
    else{ //one of the other switches are already on
      delay(pressDuration); //Normal delay to activate solenoid
    }
    Joystick.releaseButton(JoyButtonGP2A); //release keys
    PressGP2 = 1; //Switch has been manually moved up/down
  }

  if (StateGP2 == 2 && PressGP2 == 0){//Switch is moved to down position
    Joystick.pressButton(JoyButtonGP2B);
    if (SimGP1 == 1 && SimGP3 == 1 && SimGP4 == 1){ //Check if other ground power switches are off
      delay(3100); //First ground power switch needs 3 second hold-in to activate
    }
    else{ //one of the other switches are already on
      delay(pressDuration); //Normal delay to activate solenoid
    }      
    Joystick.releaseButton(JoyButtonGP2B); //release keys
    PressGP2 = 1; //Switch has been manually moved up/down
  }

  //test if we can detect switch being moved to off
  if (StateGP2 == 1 && PressGP2 == 1){//Switch manually moved to center
    Joystick.pressButton(JoyButtonGP2Auto);
    delay(pressDuration);
    Joystick.releaseButton(JoyButtonGP2Auto); //release keys
    PressGP2 = 0; //Switch is no longer up/down
  }

  if (StateGP2 == 1){//Switch released to center in DCS
    Joystick.releaseButton(JoyButtonGP2A); //release keys  
    Joystick.releaseButton(JoyButtonGP2Auto);
    Joystick.releaseButton(JoyButtonGP2B);
    PressGP2 = 0; //Switch is no longer up/down
  }

  //****************GROUND POWER 3 SWITCH LOGIC
  if (digitalRead(GP3_A_PIN) == LOW){ //read switch state without debounce
    StateGP3 = 0; //Switch is in up position
  }

  if (digitalRead(GP3_B_PIN) == LOW){//Switch is in down position
    StateGP3 = 2;
  }

  if (digitalRead(GP3_A_PIN) == HIGH && digitalRead(GP3_B_PIN) == HIGH){//Switch is in center position
    StateGP3 = 1;
  }

  if (StateGP3 == 0 && PressGP3 == 0){//Switch is moved to up position
    Joystick.pressButton(JoyButtonGP3A);
    if (SimGP1 == 1 && SimGP2 == 1 && SimGP4 == 1){ //Check if other ground power switches are off
      delay(3100); //First ground power switch needs 3 second hold-in to activate
    }
    else{ //one of the other switches are already on
      delay(pressDuration); //Normal delay to activate solenoid
    }
    Joystick.releaseButton(JoyButtonGP3A); //release keys
    PressGP3 = 1; //Switch has been manually moved up/down
  }

  if (StateGP3 == 2 && PressGP3 == 0){//Switch is moved to down position
    Joystick.pressButton(JoyButtonGP3B);
    if (SimGP1 == 1 && SimGP2 == 1 && SimGP4 == 1){ //Check if other ground power switches are off
      delay(3100); //First ground power switch needs 3 second hold-in to activate
    }
    else{ //one of the other switches are already on
      delay(pressDuration); //Normal delay to activate solenoid
    }      
    Joystick.releaseButton(JoyButtonGP3B); //release keys
    PressGP3 = 1; //Switch has been manually moved up/down
  }

  //test if we can detect switch being moved to off
  if (StateGP3 == 1 && PressGP3 == 1){//Switch manually moved to center
    Joystick.pressButton(JoyButtonGP3Auto);
    delay(pressDuration);
    Joystick.releaseButton(JoyButtonGP3Auto); //release keys
    PressGP3 = 0; //Switch is no longer up/down
  }

  if (StateGP3 == 1){//Switch released to center in DCS
    Joystick.releaseButton(JoyButtonGP3A); //release keys  
    Joystick.releaseButton(JoyButtonGP3Auto);
    Joystick.releaseButton(JoyButtonGP3B);
    PressGP3 = 0; //Switch is no longer up/down
  }

  //****************GROUND POWER 4 SWITCH LOGIC
  if (digitalRead(GP4_A_PIN) == LOW){ //read switch state without debounce
    StateGP4 = 0; //Switch is in up position
  }

  if (digitalRead(GP4_B_PIN) == LOW){//Switch is in down position
    StateGP4 = 2;
  }

  if (digitalRead(GP4_A_PIN) == HIGH && digitalRead(GP4_B_PIN) == HIGH){//Switch is in center position
    StateGP4 = 1;
  }

  if (StateGP4 == 0 && PressGP4 == 0){//Switch is moved to up position
    Joystick.pressButton(JoyButtonGP4A);
    if (SimGP1 == 1 && SimGP2 == 1 && SimGP3 == 1){ //Check if other ground power switches are off
      delay(3100); //First ground power switch needs 3 second hold-in to activate
    }
    else{ //one of the other switches are already on
      delay(pressDuration); //Normal delay to activate solenoid
    }
    Joystick.releaseButton(JoyButtonGP4A); //release keys
    PressGP4 = 1; //Switch has been manually moved up/down
  }

  if (StateGP4 == 2 && PressGP4 == 0){//Switch is moved to down position
    Joystick.pressButton(JoyButtonGP4B);
    if (SimGP1 == 1 && SimGP2 == 1 && SimGP3 == 1){ //Check if other ground power switches are off
      delay(3100); //First ground power switch needs 3 second hold-in to activate
    }
    else{ //one of the other switches are already on
      delay(pressDuration); //Normal delay to activate solenoid
    }      
    Joystick.releaseButton(JoyButtonGP4B); //release keys
    PressGP4 = 1; //Switch has been manually moved up/down
  }

  //test if we can detect switch being moved to off
  if (StateGP4 == 1 && PressGP4 == 1){//Switch manually moved to center
    Joystick.pressButton(JoyButtonGP4Auto);
    delay(pressDuration);
    Joystick.releaseButton(JoyButtonGP4Auto); //release keys
    PressGP4 = 0; //Switch is no longer up/down
  }

  if (StateGP4 == 1){//Switch released to center in DCS
    Joystick.releaseButton(JoyButtonGP4A); //release keys  
    Joystick.releaseButton(JoyButtonGP4Auto);
    Joystick.releaseButton(JoyButtonGP4B);
    PressGP4 = 0; //Switch is no longer up/down
  }

  DcsBios::loop();
}

