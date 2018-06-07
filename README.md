# CS120B_Final Lab Project
## LED controller

## Purpose
This project practices embedded systems design patterns, while incorporating new software/hardware complexities not learned in class.

## Controls
### Startup
The system starts in what is called the "Pick state". While in this state, varying the potentiometer will change the output color.
Pressing the "Select" button to the left of the potentiometer will choose the current color as the first of three custom colors the user can select.
After picking all three colors, the system automatically goes into the "Pattern state". Here, the first of three pre-set patterns is initially displayed. The the pre-set patterns can be cycled through by repeatedly pressing the same Select button as before. However, if the Select button is held down for at least 1 second, then the system goes pack the the "Pick state", allowing new colors to be chosen.
### Cycle button
The "Cycle" button, to the left of the joystick, toggles the input between the potentiometer, and the temperature sensor.
Solid blue on the LED strip is ~40F
Solid red on on the LED strip is ~100F
### Joystick
At any time, the shade of the displayed color from any input can be either increased or decreased by simply pressing up on the joystick and then releasing, or down on the joystick and then releasing, respectively. Pressing the joystick to the left resets the color to the original base RGB values, without the offset of the shade. 
Pressing the joystick to the right once and releasing will turn the LED strip off, and from that state, any input from the joystick will turn the strip back on again, retaining the offset and color that was displayed before the strip was turned off.

## Notes
The system does pause while recieving user input, meaning, if a button is held down, the LED strip will update to show that change, but then will wait until the button is released to continue updating (this goes for the joystick as well).
