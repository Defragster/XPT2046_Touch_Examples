# XPT2046_Touch_Examples
Examples using the Teensy XPT_Touchscreen

MARK3 examples are the latest.  ButtonCode moved to separate file.  Button mapping struct evolved. Stored the Mapping Data in 'sketch'.h header file so it isn't always in the way.

The examples here develop a uniform structured way to implement and handle TOUCH BUTTONS

Additionally there is an interrupt aware XPT2046_Touchscreen library UPDATE to to TeensyDuino 1.26 released:
  \hardware\teensy\avr\libraries\XPT2046_Touchscreen

PJRC pulled interrupt awareness into the tree for what should be TeensyDuino 1.27 when released

You can update these files there for testing or copy that directory to you personal Sketches\library folder
Then put these changed files over the source files copied to preserve the originals:
  user\libraries\XPT2046_Touchscreen
  
  If you change out the library code this sample allows it to be used specifying the Touch int pin:
  onoffbuttonMark3  "Button Touch ready to use interrupt aware XPT2046_Touchscreen library"

Purged older samples here - it was an evolving project - started buttons - started in sketch interrupts - moved interrupt code to core library - if you are looking start with the later examples. Any code that doesn't have attachInterrupt in will work with the old code without int pin - or with the updated library when it compiles withthe pin specified. If it has the sketch based interrupt code it won't work with the libray version. Once I get the button code finalized with expected updates I'll clean this mess up - hopefully before TeensyDuino ships the interrupt aware version. Feedback or questions at: https://forum.pjrc.com/threads/31634-ILI9341-and-XPT2046-for-Teensy-Touchscreen-320x240-display.

If you get a compile error on "XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);" you don't have the interrupt aware code, comment that line and remove comment from the other line: //XPT2046_Touchscreen ts(CS_PIN);  // Param 2 - NULL - No interrupts and TeensyDuino version 1.26 libs

To use on Teensy LC needed IFDEF or alternate code shown here: https://forum.pjrc.com/threads/31867-Color-Touchscreen-and-LC?p=89858&viewfull=1#post89858
