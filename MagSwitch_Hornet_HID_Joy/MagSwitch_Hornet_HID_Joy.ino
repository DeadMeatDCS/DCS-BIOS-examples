/*
  Tell DCS-BIOS to use a serial connection and use the default Arduino Serial
  library. This will work on the vast majority of Arduino-compatible boards,
  but you can get corrupted data if you have too many or too slow outputs
  (e.g. when you have multiple character displays), because the receive
  buffer can fill up if the sketch spends too much time updating them.
  
  If you can, use the IRQ Serial connection instead.
*/

/*
  HORNET MAG SWITCH HID WORKAROUND EXAMPLE BY DEADMEAT (Joystick v1.0)
  - Joystick version: throw physical switch to trigger HID joystick button press. Assign to appropriate command in DCS

  MOST ;) HORNET MAG SWITCHES SUPPORTED
  - APU
  * FUEL DUMP (need fix to allow manual hold-in)
  - HOOK BYPASS
  - LAUNCH BAR
  - PITOT HEAT
  - AUG PULL
  - LTD/R
  - ENGINE CRANK
  - MC (MISSION COMPUTER)
  - CANOPY (special, only held in up position)
  * GROUND POWER 1-4 (missing. First must be held 3 seconds but allow disruption..)
  

  KNOWN ISSUES
  - No switch debounce
  - Blocking button press code
  - Fuel dump does not allow manual hold-in (yet)
  - GND PWR switches need additional logic (first thrown needs 3 sec hold-in)
*/

#define DCSBIOS_DEFAULT_SERIAL

#include "DcsBios.h"

#include <Joystick.h>

Joystick_ Joystick;

int pressDuration = 100; //ms to hold joystick button pressed

//****************2-POSITION MAG SWITCH DEFINITIONS
//****************APU SWITCH DEFINITIONS
int PressApu = 0;                  //count if APU has been pushed
int StateApu = 0;                  //APU switch position
byte SimApu = 0;                   // APU sim switch position (may not be in this position at start)

int JoyButtonApu = 0;              //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)

#define APU_ON_PIN          2
#define APU_MOSFET_PIN      3

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
//****************DOES NOT ALLOW MANUAL HOLD-IN (YET)
int PressDump = 0;                //Has switch been pushed?
int StateDump = 0;                //Switch position
byte SimDump = 0;                 //Switch poisition in DCS

int JoyButtonDump = 6;            //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)

#define DUMP_ON_PIN         4
#define DUMP_MOSFET_PIN     5

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

#define MC_1_OFF_PIN       7   //Up position
#define MC_2_OFF_PIN       8   //Down position
#define MC_MOSFET_PIN      9

//****************MC SWITCH DEFINITIONS
int PressCanopy = 0;               //count if switch has been pushed
int StateCanopy = 1;               //Switch position

int JoyButtonCanopyDown = 13;       //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)
int JoyButtonCanopyOff = 14;
int JoyButtonCanopyUp = 15;

#define CANOPY_DOWN_PIN       10   //Up position
#define CANOPY_UP_PIN         11   //Down position
#define CANOPY_MOSFET_PIN     12

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
//****************DOES NOT ALLOW MANUAL HOLD-IN (YET)
DcsBios::LED fuelDumpSw_(0x74b4, 0x0100, DUMP_MOSFET_PIN); //Added underscore to name. activates MOSFET

void onFuelDumpSwChange(unsigned int newValue) {
    SimDump = newValue;
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
  //****************DOES NOT ALLOW MANUAL HOLD-IN (YET)
  pinMode(DUMP_ON_PIN, INPUT_PULLUP);

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
  //****************DOES NOT ALLOW MANUAL HOLD-IN (YET)
  StateDump = !digitalRead(DUMP_ON_PIN); //1 if in ON, 0 if in OFF. Inverts the read signal for correct state

  if (StateDump == 1 && PressDump == 0){//Switch moved to on
    if (SimDump == 0) {
      Joystick.pressButton(JoyButtonDump);
      delay(pressDuration);
      Joystick.releaseButton(JoyButtonDump); //release keys
    }
    PressDump = 1; //Switch has been manually moved to on
  }

  //test if we can detect switch being moved to off
  if (StateDump == 0 && PressDump == 1){//Switch manually moved to off
    if (SimDump == 1) {
      Joystick.pressButton(JoyButtonDump);
      delay(pressDuration);
      Joystick.releaseButton(JoyButtonDump); //release keys
    }
    PressDump = 0; //Switch is no longer in on
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

  DcsBios::loop();
}

