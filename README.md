# XPT2046_Touch_Examples
Examples using the Teensy XPT_Touchscreen

MARK3 examples are the latest.  ButtonCode moved to separate file.  Button mapping struct evolved. Stored the Mapping Data in 'sketch'.h header file so it isn't always in the way.

The examples here develop a uniform structured way to implement and handle TOUCH BUTTONS

There is interrupt awareness in PJRC XPT2046_Touchscreen library starting in TeensyDuino 1.27b2:
  \hardware\teensy\avr\libraries\XPT2046_Touchscreen

Purged some older samples - it was an evolving project - started buttons - started in sketch interrupts - moved interrupt code to core library - if you are looking start with the MARK3 and any later examples. Any code with attachInterrupt is deprecatated with the release of TeensyDuino 1.27

Feedback or questions at: https://forum.pjrc.com/threads/31634-ILI9341-and-XPT2046-for-Teensy-Touchscreen-320x240-display.

If you get a compile error on "XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);" you don't have the interrupt aware code, comment that line and remove comment from the other line: //XPT2046_Touchscreen ts(CS_PIN);  // Param 2 - NULL - No interrupts and TeensyDuino version 1.26 libs

To use on Teensy LC you must use the Adafruit_ILI9341 library not the ILI9341_t3 library, and "Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);"
