/***************************************************
  This is our touchscreen painting example for the Adafruit ILI9341 Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/


#include <SPI.h>
#include <Wire.h>      // this is needed even tho we aren't using it
#include <ILI9341_t3.h>
#include <XPT2046_Touchscreen.h>
#include <TimerOne.h>

// The XPT2046 uses hardware SPI on the shield, and #8
#define CS_PIN  8
#define TIRQ_PIN  2
uint16_t TS_usVal = 60000; // POLL rate from the timer interrupt on Touch Detect
uint16_t wait=0;

XPT2046_Touchscreen ts(CS_PIN);
// The display also uses hardware SPI, plus #9 & #10
// MOSI=11, MISO=12, SCK=13
#define TFT_CS 10
#define TFT_DC  9
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);

// -----------------------------------------
// --- Button Data Starts here
// -----------------------------------------
// Size of the color selection boxes and the paintbrush size
#define BOXSIZE 40
#define PENRADIUS 3
int oldcolor, currentcolor;
struct TS_MAP {
  int16_t xt;
  int16_t xb;
  int16_t yl;
  int16_t yr;
};
// [Zero] is calibration for mapping to pixal, 1-4 follow screen rotation for mapping math
TS_MAP tsMap[5] = { {200, 3700, 325, 3670 }, { 0, 319, 0, 239 }, { 319, 0, 0, 239 }, { 319, 0, 239, 0 }, { 0, 319, 239, 0 } };
int16_t TS_Rotate = 4;
#define TS_NoT 32000  // No Touch on TS
struct TS_BUTTON {
  int16_t tx, ty; // Top Left X,Y
  int16_t ww, hh; // Width, Height
  uint16_t fgc;
};

int TS_bCount = 0;  // Set at run time from sizeof()
TS_BUTTON buttons[] = {
  {0, 0, BOXSIZE, BOXSIZE, ILI9341_RED },   {BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_YELLOW},
  {BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, ILI9341_GREEN},   {BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, ILI9341_CYAN},
  {BOXSIZE * 4, 0, BOXSIZE, BOXSIZE, ILI9341_BLUE},   {BOXSIZE * 5, 0, BOXSIZE, BOXSIZE, ILI9341_MAGENTA}
};
// -----------------------------------------
// --- Button Data Ends here
// -----------------------------------------

void setup(void) {
  Serial.begin(9600);
  while ( !Serial && (millis() < 3000)) ;
  tft.begin();
  ts.begin();
  oldcolor = currentcolor = ILI9341_RED;
  ButtonInit();
  Serial.println(F("Touch Paint!"));
//  Timer1.initialize( TS_usVal);
//  Timer1.attachInterrupt(TS_isr); // Touch Test 'POLL' on Timer interrupt
  pinMode(TIRQ_PIN, INPUT);
  attachInterrupt(TIRQ_PIN, TS_isrPin, FALLING);
}

elapsedMillis AutoClear;
void loop()
{
  int16_t xx, yy;
  if ( TS_GetMap( xx, yy ) ) {
    AutoClear = 0;
    if (yy < BOXSIZE) {
      int16_t sColor;
      sColor = SelectPaint( xx, oldcolor );
      if ( -1 != sColor ) {
        oldcolor = currentcolor;
        currentcolor = sColor;
      }
    }
    else if (((yy - PENRADIUS) > BOXSIZE) && ((yy + PENRADIUS) < (tft.height()-154))) {
      tft.fillCircle(xx, yy, PENRADIUS, currentcolor);
    }
  }
  if (AutoClear > 1000) {
    AutoClear = 0;
  tft.fillRect(0, BOXSIZE, 239, 125, ILI9341_WHITE);
  }
}

// -----------------------------------------
// --- Button Code Starts here
// -----------------------------------------
volatile static byte inISR = 0;
volatile static boolean inGM = false;
volatile static int16_t iXX = TS_NoT;
volatile static int16_t iYY = TS_NoT;
elapsedMillis tsLimit;

#define TS_LIMIT 10 // millisecond limit to check touch value
#define TS_MINPRESS 800 // Z Pressure return minimum to use point
#define TS_DEBOUNCE 2 // TX_Map() debounce factor

void PaintInit( uint16_t aColor ) {
  int ii = 0;
  while ( ii < TS_bCount ) {
    tft.fillRect(buttons[ii].tx, buttons[ii].ty, buttons[ii].ww, buttons[ii].hh, buttons[ii].fgc);
    if  (aColor == buttons[ii].fgc)
      tft.drawRect(buttons[ii].tx, buttons[ii].ty, buttons[ii].ww, buttons[ii].hh, ILI9341_WHITE);
    ii++;
  }
}

uint16_t SelectPaint( int16_t xx, uint16_t oColor ) {
  int ii = 0, iio = -1, iin = -1;
  uint16_t rColor = -1;
  while ( ii < TS_bCount ) {
    if  (oColor == buttons[ii].fgc)
      iio = ii;
    if  (xx >= buttons[ii].tx && xx <= buttons[ii].tx + buttons[ii].ww)
      iin = ii;
    ii++;
  }
  if ( -1 != iin ) {
    rColor = buttons[iin].fgc;
    if ( rColor != oColor ) {
      if ( -1 != iio && iio != iin )
        tft.drawRect(buttons[iio].tx, buttons[iio].ty, buttons[iio].ww, buttons[iio].hh, buttons[iio].fgc);
      tft.drawRect(buttons[iin].tx, buttons[iin].ty, buttons[iin].ww, buttons[iin].hh, ILI9341_WHITE);
    }
  }
  return rColor;
}

void  ButtonRotate( int setrotate )
{
  TS_Rotate = setrotate;
  if ( TS_Rotate > 4 ) TS_Rotate = 1;
  if ( TS_Rotate < 1 ) TS_Rotate = 4;
  tft.setRotation(TS_Rotate);
  tft.fillScreen(ILI9341_BLACK);
  PaintInit( currentcolor );
}

void ButtonInit( void )
{
  TS_bCount = sizeof( buttons) / sizeof( TS_BUTTON);
  ButtonRotate( TS_Rotate );
}

static uint8_t isrCall = 0;
static int16_t xxo = TS_NoT, yyo = TS_NoT;  // Store old/last returned x,y point
boolean TS_GetMap( int16_t &xx, int16_t &yy )
{
  boolean bRet = false;
  if ( iXX == TS_NoT && isrCall ) {
    if ( 2 == isrCall )
    Serial.print("2:");
    Serial.println(isrCall);
    isrCall = 0;
    TS_isr( );    //  bRet = TS_iGetMap( iXX, iYY );
  }
  if ( tsLimit < TS_LIMIT ) { xxo = TS_NoT; yyo = TS_NoT; return false; }
  tsLimit = 0;
  if ( iXX != TS_NoT ) {
//    Serial.print("YY:");
    xx = iXX;
    yy = iYY;
    isrCall = 1;
    iXX = TS_NoT;
    bRet = true;
  }
  return bRet;
}

boolean TS_iGetMap( int16_t &xx, int16_t &yy )
{
  static TS_Point pp;
//  static int16_t xxo = TS_NoT, yyo = TS_NoT;  // Store old/last returned x,y point
  //  if ( tsLimit < TS_LIMIT && 0==inISR )     return false;
  inGM = true;
  /* if (!ts.touched())    {
    inGM = false;
    //attachInterrupt(TIRQ_PIN, TS_isrPin, FALLING);
    return false;
  } */
  //Serial.print("?");

  pp = ts.getPoint();
  if ( pp.z < TS_MINPRESS ) {
        if (!ts.touched()) wait = 0;
    attachInterrupt(TIRQ_PIN, TS_isrPin, FALLING);
    isrCall = 0;
    inGM = false;
    return false;
  }
      wait = 11;
  xx = map(pp.x, tsMap[0].xt, tsMap[0].xb, tsMap[TS_Rotate].xt, tsMap[TS_Rotate].xb);
  yy = map(pp.y, tsMap[0].yl, tsMap[0].yr, tsMap[TS_Rotate].yl, tsMap[TS_Rotate].yr);
  if (!(TS_Rotate % 2)) {
    int16_t swap = xx;
    xx = yy;
    yy = swap;
  }
  // Debounce by +/-# pixel to minimize point jitter
  if ( ((xxo - TS_DEBOUNCE) <= xx) && ((xxo + TS_DEBOUNCE) >= xx) ) xx = xxo; else xxo = xx;
  if ( ((yyo - TS_DEBOUNCE) <= yy) && ((yyo + TS_DEBOUNCE) >= yy) ) yy = yyo; else yyo = yy;
  inGM = false;
  return true;
}

//  Timer1.initialize(TS_usVal);
//  Timer1.attachInterrupt(TS_isr); // blinkLED to run every 0.15 seconds
void TS_isr( void )
{
  int16_t xx;
  int16_t yy;
  //wait++;
//  if (wait < 3) return;
  //wait = 0;
  ++inISR;
  Serial.print("_i");
  if ( inISR < 2  && false == inGM && iXX == TS_NoT ) {
  Serial.print("j");
    if (TS_iGetMap( xx, yy )) {
  Serial.print("k_");
      iXX = xx;
      iYY = yy;
    }
  }
  inISR--;
  return;
}

void TS_isrPin( void )
{
//  detachInterrupt(TIRQ_PIN);
if ( inISR ) return;
    Serial.println("XXX");
    tftD( 1, inGM, "inGM_xxx" );
  if ( false == inGM ) {
    Serial.println("yyy");
    tftD( 2, wait, "wait_yyy" );
    tftD( 3, wait, "3 3456789012345" );
    tftD( 4, wait, "4 3456789012345" );
    tftD( 5, wait, "5 3456789012345" );
    tftD( 6, wait, "6 3456789012345" );
    tftD( 7, wait, "7 3456789012345" );
    tftD( 0, wait, "0 3456789012345" );
    wait = 11;
    isrCall = 2;
  }
  return;
}

void tftD( int16_t line, int32_t val, const char *szinfo ){
  tft.fillRect(0, BOXSIZE+125+(20*line), 239, 18, ILI9341_YELLOW);
  tft.setCursor(0, 20*line+ BOXSIZE+125);
  tft.setTextColor(ILI9341_NAVY);
  tft.setTextSize(2);
  tft.print(val);
  tft.print("=");
  tft.print(szinfo);
  
}

