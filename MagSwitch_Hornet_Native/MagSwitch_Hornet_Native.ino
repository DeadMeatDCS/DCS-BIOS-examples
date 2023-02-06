/*
  Tell DCS-BIOS to use a serial connection and use interrupt-driven
  communication. The main program will be interrupted to prioritize
  processing incoming data.
  
  This should work on any Arduino that has an ATMega328 controller
  (Uno, Pro Mini, many others).
*/

/*
  NATIVE, NON-HID MAG SWITCH EXAMPLE BY DEADMEAT (version 1.1)

  This example shows how you can control 2 and 3 position mag switches with DCS-BIOS.
  It can run on all DCS-BIOS compliant arduino boards and does not require a HID workaround.

  The implementation is non-blocking and allows mouse and keyboard interaction with switches as normal.
  The principle should work with all F/A-18C Hornet mag switches.
  
  EXAMPLE CODE COVERS THE FOLLOWING MAG SWITCHES (* = to do)
  - APU
  * LTD/R
  * HOOK BYPASS
  * LAUNCH BAR
  * PITOT HEAT
  * AUG PULL
  * FUEL DUMP (special, has manual override mode for dumping fuel below BINGO etc.)
  - ENGINE CRANK
  * MC (Mission Computer)
  * CANOPY (special, only held in up position)
  * GOUND POWER 1-4 (special, any first switch is held for 3,1 seconds to allow hold-in of rest)
  
  ***NOTE***
  This sketch requires an update to FA-18C_hornet.lua DCS-BIOS module for 2-position mag switches to work.
  Change all switches defined as defineToggleSwitchToggleOnly2 to defineToggleSwitch or use provided module file.

  -------------

  See https://i.imgur.com/y70f2Zz.jpg for example wiring diagram based on real mag switches.
  It should be straightforward to modify it to DIY mag switches and/or another control setup than the MOSFETs.

*/

//****************GENERAL SETTINGS AND INITIALIZATIONS
#define DCSBIOS_IRQ_SERIAL //Use this for non HID capable boards like Uno, Mega etc. with either ATmega328P or ATmega2560 chip
//#define DCSBIOS_DEFAULT_SERIAL //Use this instead for HID capable boards like Pro Micro or Leonardo with ATmega32u4 chip

#include "DcsBios.h"

const int debounceDelay = 50; //Increase this if switches flicker

//****************2-POSITION MAG SWITCH DEFINITIONS
//****************APU SWITCH DEFINITIONS
#define APU_ON_PIN        2
#define APU_MOSFET_PIN    3
byte apuTryOff = 0; //Flag to identify which case to consider when turning physical switch off
unsigned long apuCounter = 0; //Debounce counter


//****************3-POSITION MAG SWITCH DEFINITIONS
//****************CRANK SWITCH DEFINITIONS
#define CRANK_L_PIN       7
#define CRANK_R_PIN       8
#define CRANK_MOSFET_PIN  9
byte crankTryOff = 1; //Flag to identify which case to consider when turning physical switch off
unsigned long crankCounter = 0; //Debounce counter

/* paste code snippets from the reference documentation here */

//****************2-POSITION MAG SWITCH OUTPUT
//****************APU switch position output
void onApuControlSwChange(unsigned int newValue) { //Detect switch state has changed in DCS
    if(newValue == 1){ //switch has been turned to ON from OFF state
      digitalWrite(APU_MOSFET_PIN, HIGH); //turn on MOSFET to hold physical switch in ON position
      //delay(100); //uncomment this line for the Viper (BIT switch)
      if (digitalRead(APU_ON_PIN) == LOW){ //Physical switch is in ON position
        sendDcsBiosMessage("APU_CONTROL_SW", "0"); //Trick DCS-BIOS into releasing hold on switch without releasing in DCS
        apuTryOff = 1; //Allow physical off signal to turn off switch in DCS
      }
      else{ //Physical switch is in off position so DCS switch must have been turn on by mouse or keyboard
        apuTryOff = 2; //Allow checking for physical switch being turn on to match DCS position
      }
    }
    
    if(newValue == 0){ //switch has been turned to OFF from ON state in DCS
      digitalWrite(APU_MOSFET_PIN, LOW); //turn off MOSFET to release physical switch
      apuTryOff = 0; //disallow futher switch off messages since switch is now off
    }
}
DcsBios::IntegerBuffer apuControlSwBuffer(0x74c2, 0x0100, 8, onApuControlSwChange);

DcsBios::Switch2Pos apuControlSw("APU_CONTROL_SW", APU_ON_PIN); //Let DCS-BIOS detect physical switch movements

//****************3-POSITION MAG SWITCH OUTPUT
//****************CRANK switch position output
void onEngineCrankSwChange(unsigned int newValue) { //Detect changes to DCS switch position
    if(newValue == 1){ //DCS switch detected in middle OFF position
      digitalWrite(CRANK_MOSFET_PIN, LOW); //Turn off magnet
      crankTryOff = 1; //Disallow monitoring for an off signal - switch is already off
    }

    if (newValue == 2){ //DCS switch detected in right position
      digitalWrite(CRANK_MOSFET_PIN, HIGH); //Turn on magnet
      if (digitalRead(CRANK_R_PIN) == LOW){ //Physial switch is detected in position matching DCS switch
        sendDcsBiosMessage("ENGINE_CRANK_SW", "1"); //Trick DCS-BIOS into releasing blocking input
        crankTryOff = 0; //Allow a potential manual OFF switch signal to send switch left
      }
      else{ //Switch must have been moved right by mouse or keyboard
        crankTryOff = 3; //Allow a check for physical switch moving to right position to match DCS position
      }

    }
    if (newValue == 0){ //DCS switch detected in left position
      digitalWrite(CRANK_MOSFET_PIN, HIGH); //Turn on magnet
      if (digitalRead(CRANK_L_PIN) == LOW) { //Physial switch is detected in position matching DCS switch
        sendDcsBiosMessage("ENGINE_CRANK_SW", "1"); //Trick DCS-BIOS into releasing blocking input
        crankTryOff = 2; //Allow a potential manual OFF signal to send switch right
      }
      else{ //Switch must have been moved left by mouse or keyboard
        crankTryOff = 4; //Allow a check for physical switch moving to left position to match DCS position
      }
    }
}
DcsBios::IntegerBuffer engineCrankSwBuffer(0x74c2, 0x0600, 9, onEngineCrankSwChange);

DcsBios::Switch3Pos engineCrankSw("ENGINE_CRANK_SW", CRANK_L_PIN, CRANK_R_PIN); //Let DCS-BIOS detect physical switch movements

//****************DISABLE SOLENOIDS WHEN EXITING MISSION
void onAcftNameChange(char* newValue) {
    digitalWrite(APU_MOSFET_PIN, LOW);
    digitalWrite(CRANK_MOSFET_PIN, LOW);
}
DcsBios::StringBuffer<24> AcftNameBuffer(0x0000, onAcftNameChange);


void setup() {
  //****************2-POSITION MAG SWITCH SETUP
  //****************APU SETUP
  pinMode(APU_MOSFET_PIN, OUTPUT); //Output signal for the MOSFET
    
  //****************3-POSITION MAG SWITCH SETUP
  //****************CRANK SETUP
  pinMode(CRANK_MOSFET_PIN, OUTPUT); //Output signal for the MOSFET

  DcsBios::setup();
}

void loop() {
  DcsBios::loop();

  //****************2-POSITION MAG SWITCH LOGIC
  //****************APU SWITCH LOGIC
  if (digitalRead(APU_ON_PIN) == LOW && apuTryOff == 2){ //Physical switch was moved to ON to match DCS position
    apuTryOff = 1; //Allow detection of moving switch to OFF again
    //Remove the whole sendDcsBiosMessage line below to use logic with the Viper (BIT switch)
    sendDcsBiosMessage("APU_CONTROL_SW", "1"); //Reset DCS switch position to ON quickly to match physical switch
    
    apuCounter = millis(); //Record debounce timer index
  }
 
  if ((millis() - apuCounter) > debounceDelay){ //Let switch bounce settle before determining what to do
    if (digitalRead(APU_ON_PIN) == HIGH && apuTryOff == 1){ //Physical switch has been ON previously, but is now moved to OFF by user
      sendDcsBiosMessage("APU_CONTROL_SW", "1"); //tell DCS to turn the switch off as well
      apuTryOff = 0; //Do this only once - until switch has been turned back on again
    }
  }
  
  //****************3-POSITION MAG SWITCH LOGIC
  //****************CRANK SWITCH LOGIC
  if (digitalRead(CRANK_R_PIN) == LOW && crankTryOff == 3){ //Physical switch moved to right position to match DCS position
    crankTryOff = 0; //Allow detection of moving switch back to off again
    crankCounter = millis(); //Record debounce timer index
  }

 if (digitalRead(CRANK_L_PIN) == LOW && crankTryOff == 4){ //Physical switch moved to left position to match DCS position
    crankTryOff = 2; //Allow detection of moving switch back to off again
    crankCounter = millis(); //Record debounce timer index
  }

  if ((millis() - crankCounter) > debounceDelay){ //Let switch bounce settle before determining what to do
    if (digitalRead(CRANK_L_PIN) == HIGH && digitalRead(CRANK_R_PIN) == HIGH){ //Physical switch is in OFF position
      if (crankTryOff == 0){ //Physical switch has moved from Right On position, so DCS switch needs to move left to be turned off
        sendDcsBiosMessage("ENGINE_CRANK_SW", "0"); //Tell DCS to move switch left to turn off
        crankTryOff = 1; //Do this only once - until switch has been turned back on again
      }
      if (crankTryOff == 2){ //Physical switch has moved from Left On position, so DCS switch needs to move right to be turned off
        sendDcsBiosMessage("ENGINE_CRANK_SW", "2"); //Tell DCS to move switch right to turn off
        crankTryOff = 1; //Do this only once - until switch has been turned back on again
      }
    }
  }

}