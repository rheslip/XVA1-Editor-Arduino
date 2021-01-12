# XVA1 Editor-Arduino

Arduino Patch Editor for the XVA1 FPGA Synth

Uses a 4 line x 20 character LCD, a menu encoder with switch for menu navigation and 4 encoders to edit parameter values (pots could be used too).

This editor can edit approximately 80% of the 400 or so parameters in the XVA1. I have a CTRLR editor for the PC that can edit virtually everything but this embedded editor is handy when you are working standalone.

There are two levels of menus - top level is by functional block/group/activity e.g. oscillator 1, oscillator 2, save patch etc. 2nd level is the parameters for that block e.g. waveform, detune, transpose, pulsewidth. We can edit 4 of these at a time with the 4 pots/encoders

Main encoder scrolls through the top level selections when the encoder button is pressed or the parameter list for the selection when not pressed. i.e. you can reach a parameter quickly with a press+rotation and then a rotation
The top level menu is split into a main menu for the most used blocks (oscillators, filters, LFOs, most FX) and a secondary menu for less used parameters to reduce scrolling. Double click the menu encoder to switch between main and secondary menus.

There is a performance page on the secondary menu which allows quick access to some of the most useful parameters. Its easy to add or remove items by cutting/pating from the other menus and recompiling.

I used an ESP32 for this implementation but in hindsight I should have used an AVR - Mega1284 or something with a lot of pins and at least 2 serial ports. ESP32 Arduino is not very stable and I encountered a number of compiler bugs and stability issues. 
I used ESP32 Arduino V1.0 because the later versions are even less stable.

