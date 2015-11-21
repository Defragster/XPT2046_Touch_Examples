// Teensy test ILI9341 display with XPT20464 Touch controller wired to SPI
// Not the simplest sketch perhaps - but,
// It simply shows how to orient the display and map returned touch values to pixels
// Demonstrates usage in any of 4 TFT rotations, and a way of writing debug strings to the screen & USB
// Touch points are returned from hardware fixed frame of reference, but display orientation is viewer relative
// Uses elapsedMillis time references to control poll rate of the TOUCH sensor without delay()
// http://Forum.pjrc.com :: defragster

#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <Wire.h>
#include <ILI9341_t3.h>

// SPI COMMON assumed :: MOSI=11, MISO=12, SCK=13
#define CS_PIN  8
XPT2046_Touchscreen ts(CS_PIN);
#define TFT_CS 10
#define TFT_DC  9
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);

int orientationLast;
void setup() {
  ts.begin(); // always returns TRUE
  while ( !Serial && (millis() < 2000)) ;
  Serial.println("Touchscreen started.");
  tft.begin();  // void return
  Serial.println("ILI9341 started.");
  orientationLast = ButtonRotate( 4 );  // Set mapping orientation, clear screen
  tftD( 7, orientationLast, "Orientation");
  setTarget();
}

elapsedMillis waitTouch;
elapsedMillis noTouch;
TS_Point pp; // Shared into TS_getMap : ideally not used outside

int16_t xx;
int16_t yy;
void loop() {
  if (waitTouch > 100) {
    if ( TS_getMap( xx, yy ) ) {
      tftD( 1, pp.z, "Pres.");
      tftD( 2, pp.x, " x ");
      tftD( 3, pp.y, " x ");
      tftD( 4, xx, " xx pix ");
      tftD( 5, yy, " yy pix ");
      Serial.println();
      noTouch = 0;
    }
    waitTouch = 0;
  }
  if (noTouch > 10000) {
    noTouch = 0;
    Serial.println("Touchscreen Not touched for 10 seconds. Rotating Screen.");
    orientationLast = ButtonRotate( 1 + orientationLast );
    tftD( 7, orientationLast, "Orientation");
    setTarget();
    Serial.println();
  }
}

// Display target touch points to test mapping
void setTarget() {
  tft.drawPixel( 200, 10, ILI9341_WHITE );
  tft.drawPixel( 200, 200, ILI9341_WHITE );
  tft.drawPixel( 10, 200, ILI9341_WHITE );
  tft.drawPixel( 10, 239, ILI9341_WHITE );
}

// ---- Screen Mapping Code :: START ------------------------
int16_t TS_Rotate = 4;  // Adjust TFT orientation to your use - pixel Mapping follows
struct TS_MAP {  // [Zero] is calibration for mapping to pixel, 1-4 follow screen rotation for mapping math
  int16_t xt; int16_t xb; int16_t yl; int16_t yr;
};
TS_MAP tsMap[5] = { {200, 3700, 325, 3670 }, { 0, 319, 0, 239 }, { 319, 0, 0, 239 }, { 319, 0, 239, 0 }, { 0, 319, 239, 0 } };
int  ButtonRotate( int setrotate )
{
  if ( setrotate > 4 ) setrotate = 1;
  if ( setrotate < 1 ) setrotate = 4;
  tft.setRotation(TS_Rotate = setrotate);
  tft.fillScreen(ILI9341_BLACK);
  return setrotate;
}

boolean TS_getMap( int16_t &xx, int16_t &yy )
{
  pp = ts.getPoint();
  if ( ts.touched())
    pp = ts.getPoint();
  else
    return false;
  xx = map(pp.x, tsMap[0].xt, tsMap[0].xb, tsMap[TS_Rotate].xt, tsMap[TS_Rotate].xb);
  yy = map(pp.y, tsMap[0].yl, tsMap[0].yr, tsMap[TS_Rotate].yl, tsMap[TS_Rotate].yr);
  if (!(TS_Rotate % 2)) {
    int16_t swap = xx;
    xx = yy;
    yy = swap;
  }
  return true;
}
// ---- Screen Mapping Code :: END ------------------------

// Sample DEBUG to TFT and echo to USB
void tftD( int16_t line, int32_t val, const char *szinfo ) {
  if ( line < 10 ) {
    tft.fillRect(5, (20 * line), 165, 19, ILI9341_YELLOW);
    tft.setCursor(8, 1 + 20 * line );
    tft.setTextColor(ILI9341_NAVY);
    tft.setTextSize(2);
    tft.print(szinfo);
    tft.print("=");
    tft.print(val);
  }
  Serial.print(szinfo);
  Serial.print("=");
  Serial.print(val);
  Serial.print(" ");
}


