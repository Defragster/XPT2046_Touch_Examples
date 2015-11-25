# XPT2046_Touch_Examples
Examples using the Teensy XPT_Touchscreen

The examples here develop a uniform structured way to implement and handle TOUCH BUTTONS

Additionally there is an interrupt aware XPT2046_Touchscreen library UPDATE to to TeensyDuino 1.26 released:
  \hardware\teensy\avr\libraries\XPT2046_Touchscreen

You can update these files there for testing or copy that directory to you personal Sketches\library folder
Then put these changed files over the source files copied to preserve the originals:
  user\libraries\XPT2046_Touchscreen
  
  If you change out the library code this sample allows it to be used specifying the Touch int pin:
  onoffbutton4_Pull  "Button Touch ready to use interrupt aware XPT2046_Touchscreen library"
