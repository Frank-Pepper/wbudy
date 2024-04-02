demo
=====================
This project contains a an example using several of the peripherals
on the LPCXpresso base Board.

With the joystick it is possible to draw on the OLED display
With the Rotary witch it is possible to control the 7-segment display
With the trimpot it is possible to control the RGB LED
Moving the board (accelerometer) will control a running pattern on the LEDs
Click SW3 button to start playing a melody on the Speaker

The project makes use of code from the following library projects:
- CMSISv1p30_LPC17xx : for CMSIS 1.30 files relevant to LPC17xx
- MCU_Lib        	 : for LPC17xx peripheral driver files
- EaBaseBoard_Lib    : for Embedded Artists LPCXpresso Base Board peripheral drivers

These library projects must exist in the same workspace in order
for the project to successfully build.

