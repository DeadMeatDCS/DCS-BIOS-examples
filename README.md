# DCS-BIOS examples
Example sketches for [DCS-BIOS, flight panels fork.](https://github.com/DCSFlightpanels/dcs-bios)

The goal here is to showcase functionality. My coding skills are not good enough to make things beautiful or *super* easy to use.

My intent is to provide the principles here that others may use or be inspired by to create their own implementation.

## MagSwitch_Hornet_HID_Joy
Sketch containing code for all Hornet mag switches using the HID workaround I pioneered.

Implemented using a [HID Joystick library](https://github.com/MHeironimus/ArduinoJoystickLibrary) instead of the old Keyboard.h approach.

*Tested with DCS open beta, version*
```
2.8.2.35759
```

## MagSwitch_Hornet_Native
Sketch containing example code for a "native" mag switch implementation that doesn't rely on the HID workaround.

Idea for unblocking command provided by kbastronomics.

Intended for Arduino boards with ATmega328P or Mega2560 chipsets to allow for a RS485 network setup.

*Tested with DCS open beta, version*
```
2.8.2.35759
```
