# DCS-BIOS examples
Example sketches for [DCS-BIOS, flight panels fork.](https://github.com/DCSFlightpanels/dcs-bios)

The goal here is to showcase functionality. My coding skills are not good enough to make things beautiful or *super* easy to use.

My intent is to provide the principles here that others may use or be inspired by to create their own implementation.

## MagSwitch_Hornet_HID_Joy
Sketch containing code for all Hornet mag switches using the HID approach I pioneered.

Implemented using a [HID Joystick library](https://github.com/MHeironimus/ArduinoJoystickLibrary) instead of the old Keyboard.h approach.

### Special features
- All switches implemented, including bleed air AUG PULL.
- Fuel dump switch allows manual hold-in override mode for dumping fuel below BINGO etc. where solenoid will not be on.
- Canopy switch is only solenoid held in up position. Down position is momentary.
- Ground power switches: any first switch thrown must be is held in for 3 seconds to active solenoid hold-in for the rest.

### Instructions
- *To be added..*

*Tested with DCS open beta, version*
```
2.8.2.35759
```

## MagSwitch_Hornet_Native
Sketch containing example code for a "native" mag switch implementation that doesn't rely on the HID approach.

Idea for unblocking command provided by kbastronomics.

Intended for Arduino boards with ATmega328P or Mega2560 chipsets to allow for a RS485 network setup.

*Tested with DCS open beta, version*
```
2.8.2.35759
```

## MagSwitch_Viper_HID_Joy
Sketch containing example code for all Viper mag switches using the HID approach. 
They all needed some tweaking from the Hornet principles to work, so I figured it was worth sharing.

Implemented using a [HID Joystick library](https://github.com/MHeironimus/ArduinoJoystickLibrary).

### Special features (they're all special...)
- BIT switch requires longer slightly longer press time to allow hold-in.
- JFS switch doesn't have a mappable "off" postion in DCS, so we're using a partly "native" approach.
- AP Pitch switch coded to allow "fast" switching from up/down through off to down/up.
- Anti-Skid/parking brake switch is only solenoid held in up ("PBRAKE") position. Down ("OFF") position is mechanically maintained.

*Tested with DCS open beta, version*
```
2.8.2.35759
```
