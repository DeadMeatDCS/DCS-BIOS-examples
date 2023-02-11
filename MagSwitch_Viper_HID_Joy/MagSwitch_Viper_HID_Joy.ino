/*
  Tell DCS-BIOS to use a serial connection and use the default Arduino Serial
  library. This will work on the vast majority of Arduino-compatible boards,
  but you can get corrupted data if you have too many or too slow outputs
  (e.g. when you have multiple character displays), because the receive
  buffer can fill up if the sketch spends too much time updating them.
  
  If you can, use the IRQ Serial connection instead.
*/

/*
  VIPER MAG SWITCH HID WORKAROUND EXAMPLE BY DEADMEAT (Joystick v1.0)
  - Joystick version: throw physical switch to trigger HID joystick button press. Assign to appropriate command in DCS

  ALL VIPER MAG SWITCHES SUPPORTED (* = to do)
  - BIT switch (special, needs 150 ms hold-in to engage)
  - JFS swtich (special, off position not mappable in DCS, so have to use partly "native" code)
  - AP pitch switch (special, allows fast switching between up and down positions)
  - Parking brake/anti-skid switch (special, only up position is solenoid held)
  

  KNOWN ISSUES
  - No switch debounce
  - Blocking button press code
  - Will need to add board ID if using multiple boards. See Joystick library documentation.
  - If no other joystick is plugged in, DCS assigns this board to x/y/z axes. Clear axis bindings to resolve
*/

#define DCSBIOS_DEFAULT_SERIAL

#include "DcsBios.h"

#include <Joystick.h>
Joystick_ Joystick; // Create a joystick

//***GENERAL SETTINGS
const int pressDuration = 100; //ms to hold joystick button pressed

//****************2-POSITION MAG SWITCH DEFINITIONS
//****************BIT SWITCH DEFINITIONS
int PressBit = 0;                  //count if APU has been pushed
int StateBit = 0;                  //APU switch position
byte SimBit = 0;                   //APU sim switch position (may not be in this position at start)

int JoyButtonBit = 0;              //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)

#define BIT_ON_PIN          2
#define BIT_MOSFET_PIN      3

//****************3-POSITION MAG SWITCH DEFINITIONS
//****************JFS SWITCH DEFINITIONS
int PressJfs = 0;               //Count if switch has been pushed
int StateJfs = 1;               //Switch position
int SimJfs = 1;                 //DCS switch position

int JoyButtonJfsDown = 1;       //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)
int JoyButtonJfsUp = 2;

#define JFS_DOWN_PIN 		    10
#define JFS_UP_PIN  		    11
#define JFS_MOSFET_PIN		  12

//****************AP PITCH SWITCH DEFINITIONS
int PressApPitch = 0;               //count if switch has been pushed
int StateApPitch = 1;               //Switch position
int SimApPitch = 1;                 //DCS switch position

int JoyButtonApPitchDown = 3;       //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)
int JoyButtonApPitchOff = 4;
int JoyButtonApPitchUp = 5;

#define AP_PITCH_DOWN_PIN     7     //"ATT" hold
#define AP_PITCH_UP_PIN       8     //"ALT" hold
#define AP_PITCH_MOSFET_PIN   9

//****************PARKING BRAKE/ANTI-SKID SWITCH DEFINITIONS
int PressPBrake = 0;               //count if switch has been pushed
int StatePBrake = 1;               //Switch position
int SimPBrake = 1;                 //DCS switch position

int JoyButtonPBrakeDown = 6;       //Index for assigned joystick button starting at 0 (button 1 = 0, 2 = 1 etc.)
int JoyButtonPBrakeUp = 7;
                                   //"Anti-skid" position is the middle position. Not mapped.
#define PBRAKE_DOWN_PIN       10   //"OFF" position, down
#define PBRAKE_UP_PIN         11   //"Parking brake" position, up
#define PBRAKE_MOSFET_PIN     12

/* paste code snippets from the reference documentation here */

//****************2-POSITION MAG SWITCH OUTPUT
//****************BIT switch position output
DcsBios::LED bitSw_(0x4400, 0x0004, BIT_MOSFET_PIN); //Added underscore to name. Activates MOSFET

void onBitSwChange(unsigned int newValue) {
    SimBit = newValue;
}
DcsBios::IntegerBuffer bitSwBuffer(0x4400, 0x0004, 2, onBitSwChange);

//****************3-POSITION MAG SWITCH OUTPUT
//****************CRANK switch position output
void onJfsSwChange(unsigned int newValue) {
    SimJfs = newValue;
    if (newValue == 1){ //if switch in middle position
      digitalWrite(JFS_MOSFET_PIN, LOW); 
    }
    else{//if switch in up/down position
      digitalWrite(JFS_MOSFET_PIN, HIGH); //5v to activate MOSFET and feed 12v to crank switch solenoid
    }
}
DcsBios::IntegerBuffer jfsSwBuffer(0x4424, 0x0003, 0, onJfsSwChange);

//****************AP PITCH switch position output
void onApPitchSwChange(unsigned int newValue) {
    SimApPitch = newValue;
    if (newValue == 1){ //if switch is in middle position
      digitalWrite(AP_PITCH_MOSFET_PIN, LOW); 
    }
    else{//if switch is in up/down
      digitalWrite(AP_PITCH_MOSFET_PIN, HIGH); //5v to activate MOSFET and feed 12v to switch solenoid
    }
}
DcsBios::IntegerBuffer apPitchSwBuffer(0x4400, 0x0300, 8, onApPitchSwChange);

//****************PARKING BRAKE/ANTI-SKID switch position output
void onAntiSkidSwChange(unsigned int newValue) {
    SimPBrake = newValue;
    if (newValue == 2){ //if switch is in up position
      digitalWrite(PBRAKE_MOSFET_PIN, HIGH); //5v to activate MOSFET and feed 12v to switch solenoid
    }
    else{//if switch is in middle or down position
      digitalWrite(PBRAKE_MOSFET_PIN, LOW); 
    }
}
DcsBios::IntegerBuffer antiSkidSwBuffer(0x441e, 0x0180, 7, onAntiSkidSwChange);

//****************DISABLE SOLENOIDS WHEN EXITING MISSION
void onAcftNameChange(char* newValue) {
    digitalWrite(BIT_MOSFET_PIN, LOW);
    digitalWrite(JFS_MOSFET_PIN, LOW);
    digitalWrite(AP_PITCH_MOSFET_PIN, LOW);
    digitalWrite(PBRAKE_MOSFET_PIN, LOW);
}
DcsBios::StringBuffer<24> AcftNameBuffer(0x0000, onAcftNameChange);

void setup() {
  //****************2-POSITION MAG SWITCH SETUP
  //****************BIT SETUP
  pinMode(BIT_ON_PIN, INPUT_PULLUP);
  
  //****************3-POSITION MAG SWITCH SETUP
  //****************JFS SETUP
  pinMode(JFS_DOWN_PIN, INPUT_PULLUP);
  pinMode(JFS_UP_PIN, INPUT_PULLUP);
  pinMode(JFS_MOSFET_PIN, OUTPUT);

  //****************AP PITCH SETUP
  pinMode(AP_PITCH_DOWN_PIN, INPUT_PULLUP);
  pinMode(AP_PITCH_UP_PIN, INPUT_PULLUP);
  pinMode(AP_PITCH_MOSFET_PIN, OUTPUT);

  //***************PARKING BRAKE/ANTI-SKID SETUP
  pinMode(PBRAKE_UP_PIN, INPUT_PULLUP);
  pinMode(PBRAKE_DOWN_PIN, INPUT_PULLUP);
  pinMode(PBRAKE_MOSFET_PIN, OUTPUT);

  Joystick.begin();

  DcsBios::setup();
}

void loop() {
  //****************2-POSITION MAG SWITCH LOGIC
  //****************BIT SWITCH LOGIC
  StateBit = !digitalRead(BIT_ON_PIN); //1 if in ON, 0 if in OFF. Inverts the read signal for correct state

  if (StateBit == 1 && PressBit == 0){//Switch moved to on
    if (SimBit == 0) {
      Joystick.pressButton(JoyButtonBit);
      delay(pressDuration+50); //This one needs to be held longer than normal.. 150 ms seems to be enough
      Joystick.releaseButton(JoyButtonBit); //release keys
    }
    PressBit = 1; //Switch has been manually moved to on
  }

  if (StateBit == 0 && PressBit == 1){//Switch manually moved to off
    if (SimBit == 1) {
      Joystick.pressButton(JoyButtonBit);
      delay(pressDuration);
      Joystick.releaseButton(JoyButtonBit); //release keys
    }
    PressBit = 0; //Switch is no longer in on
  }

  if (StateBit == 0){//Switch released to off in DCS
    Joystick.releaseButton(JoyButtonBit); //release keys
    PressBit = 0;
  }

  //****************3-POSITION MAG SWITCH LOGIC
  //****************JFS SWITCH LOGIC
  if (digitalRead(JFS_DOWN_PIN) == LOW){ //read switch state without debounce
    StateJfs = 0; //Switch is in down position
  }

  if (digitalRead(JFS_UP_PIN) == LOW){//Switch is in up position
    StateJfs = 2;
  }

  if (digitalRead(JFS_DOWN_PIN) == HIGH && digitalRead(JFS_UP_PIN) == HIGH){//Switch is in center
    StateJfs = 1;
  }

  if (StateJfs == 0 && PressJfs == 0){//Switch is moved to down
    Joystick.pressButton(JoyButtonJfsDown);
    delay(pressDuration);
    Joystick.releaseButton(JoyButtonJfsDown); //release keys
    PressJfs = 1; //Switch has been manually moved up/down
  }

  if (StateJfs == 2 && PressJfs == 0){//Switch is moved to up
    Joystick.pressButton(JoyButtonJfsUp);
    delay(pressDuration);
    Joystick.releaseButton(JoyButtonJfsUp); //release keys
    PressJfs = 1; //Switch has been manually moved up/down
  }

  //test if we can detect switch being moved to off
  if (StateJfs == 1 && PressJfs == 1){//Switch manually moved to center
    if (SimJfs == 2){ //Moving from up position
      sendDcsBiosMessage("JFS_SW", "0"); //Send switch a step down
    }
    if (SimJfs == 0){ //Moving from down position
      sendDcsBiosMessage("JFS_SW", "2"); //Send switch a step down
    }
    PressJfs = 0; //Switch is no longer up/down
  }

  if (StateJfs == 1){//Switch released to center in DCS
    Joystick.releaseButton(JoyButtonJfsDown); //release keys  
    Joystick.releaseButton(JoyButtonJfsUp);
    PressJfs = 0; //Switch is no longer up/down
  }

  //****************AP PITCH SWITCH LOGIC
  if (digitalRead(AP_PITCH_DOWN_PIN) == LOW){ //read switch state without debounce
    StateApPitch = 0; //Switch is in down position
  }

  if (digitalRead(AP_PITCH_UP_PIN) == LOW){//Switch is in up position
    StateApPitch = 2;
  }

  if (digitalRead(AP_PITCH_DOWN_PIN) == HIGH && digitalRead(AP_PITCH_UP_PIN) == HIGH){//Switch is in center position
    StateApPitch = 1;
  }

  if (StateApPitch == 0 && PressApPitch == 0){//Switch is moved to down position
    Joystick.pressButton(JoyButtonApPitchDown);
    delay(pressDuration);
    Joystick.releaseButton(JoyButtonApPitchDown); //release keys
    PressApPitch = 1; //Switch has been manually moved up/down
  }

  if (StateApPitch == 2 && PressApPitch == 0){//Switch is moved to up position
    Joystick.pressButton(JoyButtonApPitchUp);
    delay(pressDuration);
    Joystick.releaseButton(JoyButtonApPitchUp); //release keys
    PressApPitch = 1; //Switch has been manually moved up/down
  }

  //Test fast switching
  if (StateApPitch != SimApPitch && PressApPitch == 1 && SimApPitch != 1){ //Physical switch has moved from up/down to down/up but DCS didn't catch it
    if (StateApPitch == 0){                             //Switch moved to down
      Joystick.pressButton(JoyButtonApPitchDown);       //Press to move to middle position
      delay(pressDuration);
      Joystick.releaseButton(JoyButtonApPitchDown);     //release keys
      Joystick.pressButton(JoyButtonApPitchDown);       //Press a second time to down position
      delay(pressDuration);
      Joystick.releaseButton(JoyButtonApPitchDown);     //release keys
    }
    if (StateApPitch == 2){                                  //Switch moved up
      Joystick.pressButton(JoyButtonApPitchUp);         //Press to move to middle position
      delay(pressDuration);
      Joystick.releaseButton(JoyButtonApPitchUp);       //release keys
      Joystick.pressButton(JoyButtonApPitchUp);         //Press a second time to up position
      delay(pressDuration);
      Joystick.releaseButton(JoyButtonApPitchUp);       //release keys
    }
  }

  //test if we can detect switch being moved to off
  if (StateApPitch == 1 && PressApPitch == 1){//Switch manually moved to center
    Joystick.pressButton(JoyButtonApPitchOff);
    delay(pressDuration);
    Joystick.releaseButton(JoyButtonApPitchOff); //release keys
    PressApPitch = 0; //Switch is no longer up/down
  }

  if (StateApPitch == 1){//Switch released to center in DCS
    Joystick.releaseButton(JoyButtonApPitchDown); //release keys  
    Joystick.releaseButton(JoyButtonApPitchOff);
    Joystick.releaseButton(JoyButtonApPitchUp);
    PressApPitch = 0; //Switch is no longer up/down
  }

  //****************PARKING BRAKE/ANTI-SKID SWITCH LOGIC
  if (digitalRead(PBRAKE_DOWN_PIN) == LOW){ //read switch state without debounce
    StatePBrake = 0; //Switch is in down position
  }

  if (digitalRead(PBRAKE_UP_PIN) == LOW){//Switch is in up position
    StatePBrake = 2;
  }

  if (digitalRead(PBRAKE_DOWN_PIN) == HIGH && digitalRead(PBRAKE_UP_PIN) == HIGH){//Switch is in center position
    StatePBrake = 1;
  }

  if (StatePBrake == 0 && PressPBrake == 0){//Switch is moved to down position
    Joystick.pressButton(JoyButtonPBrakeDown);
    PressPBrake = 1; //Switch has been manually moved up/down
  }

  if (StatePBrake == 2 && PressPBrake == 0){//Switch is moved to up position
    Joystick.releaseButton(JoyButtonPBrakeDown);
    Joystick.pressButton(JoyButtonPBrakeUp);
    delay(pressDuration);
    Joystick.releaseButton(JoyButtonPBrakeUp); //release keys
    PressPBrake = 1; //Switch has been manually moved up/down
  }

  //test if we can detect switch being moved to off
  if (StatePBrake == 1 && PressPBrake == 1){//Switch manually moved to center
    if (SimPBrake == 2){ //Moving from up position
      Joystick.releaseButton(JoyButtonPBrakeDown);
      sendDcsBiosMessage("ANTI_SKID_SW", "0"); //Send switch a step down
    }
    if (SimPBrake == 0){ //Moving from down position
      sendDcsBiosMessage("ANTI_SKID_SW", "2"); //Send switch a step down
    }
    PressPBrake = 0; //Switch is no longer up/down
  }

  if (StatePBrake == 1){//Switch released to center in DCS
    Joystick.releaseButton(JoyButtonPBrakeDown); //release keys 
    Joystick.releaseButton(JoyButtonPBrakeUp);
    PressPBrake = 0; //Switch is no longer up/down
  }

  DcsBios::loop();
}

