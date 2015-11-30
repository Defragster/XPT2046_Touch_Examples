// XPT2046_Touchscreen ; Button Functions
//
// +:ButtonInit with BUTTON*
// +:Reduce String RAM with (char *)
// +:self printing button struct sample code
//
// 0: Allow pushbutton/event input to tie to buttons?
// 0: Grayed Button
// 0: User defined text areas for display or edit of text?
// 0: Sentinal button could give 'window' constraints to allow selective redraw when it exits?
//
// 0: TAG wide verus narrow button  1%3 or 2,4 allow moved placement on rotation
// 0: Button Animation :: ButtonDraw( bValue, [special color, or animate type???]  );
//
// 0: Do Keyboard Test, SHIFT and Text Out USER Button display window
//


#include <SPI.h>
#include <Wire.h>

//#define LC 1
#if LC
#include "Adafruit_ILI9341.h"
#else
#include <ILI9341_t3.h>
#endif
#include <XPT2046_Touchscreen.h>

#define CS_PIN 8
//XPT2046_Touchscreen ts(CS_PIN);  // Param 2 - NULL - No interrupts and TeensyDuino version 1.26 libs
// Second PARAM on XPT2046_Touchscreen requires modified interrupt aware XPT2046_Touchscreen library
#define TIRQ_PIN  2
//XPT2046_Touchscreen ts(CS_PIN, 255);  // Param 2 - 255 - No interrupts
XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling
// Second PARAM on XPT2046_Touchscreen requires modified interrupt aware XPT2046_Touchscreen library

#define TFT_CS 10
#define TFT_DC  9
// MOSI=11, MISO=12, SCK=13
#ifdef LC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// ADAFRUIT MISSING COLORS
#define ILI9341_NAVY        0x000F      /*   0,   0, 128 */
#define ILI9341_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define ILI9341_ORANGE      0xFD20      /* 255, 165,   0 */
#define ILI9341_PINK        0xF81F
#else
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);
#endif

// -----------------------------------------
// --- Button Data Starts here
// -----------------------------------------
int16_t TS_Rotate = 1;  // INVALID until set on startup
static int16_t TS_iiSlide = -1;
#define TS_SLIDET 40  // Reject SLIDE buttons farther apart than this

struct TS_MAP {
  int16_t xt;
  int16_t xb;
  int16_t yl;
  int16_t yr;
};
// Zero is calibration for mapping to pixel, 1-4 follow screen rotation for mapping math
TS_MAP tsMap[5] = { {200, 3700, 325, 3670 }, { 0, 319, 0, 239 }, { 319, 0, 0, 239 }, { 319, 0, 239, 0 }, { 0, 319, 239, 0 } };
int16_t TS_xMax = 0; // Current Screen orientation x,y Max values
int16_t TS_yMax = 0;

struct TS_BUTTON {
  char *text; // Literal string put into FLASH on compile leaving a pointer that is user changeable
  int16_t tx, ty; // Top Left X,Y
  int16_t ww, hh; // Width, Height
  uint16_t fgc; // forground text/frame color
  uint16_t bgc; // background color button or empty frame
  uint8_t btype;  // for toggle/slider two bytes TYPE & info PAIRING number
  uint8_t info;  // for toggle/slider two bytes TYPE & info PAIRING number
  byte bState; // button state
  byte fontsz; // font size
  byte bId; // Button ID
  uint8_t TS_data; // Toggle FRAME stores state value here
};

#define TS_JITTER 250 // ms Threshold for Toggle/Slider re-activate
#define TS_FBUTN 0x01 // Frame Button
#define TS_RBUTN 0x03 // Rounded Button
#define TS_FRAME  0x10  // FRAME for two TOGGLE button areas
#define TS_TOGOFF 0x20  // Off Toggle
#define TS_TOGON  0x30
#define TS_ASLIDE 0x40 // Slide Button : A position
#define TS_BSLIDE 0x50 // Slide Button : B position
int TS_bCount = 0;  // Set at run time from sizeof()
// -----------------------------------------
// --- Button Data Ends here
// -----------------------------------------

// -----------------------------------------
// --- USER Button Data Starts here
//        { text *, tx, ty, ww, hh, fgc, bgc, btype, info, bState, fontsz, bId, TS_data }    << Struct looks like this
// REQUIRED LAST ITEM :: {(char *)".", -1, 0, 0, 0, 0, 0, TS_FBUTN, 0, 0, 0, 0, 0}
// -----------------------------------------
TS_BUTTON mybuttons2[] = {{(char *)"1", 15, 15, 70, 60, 65535, 15, TS_FBUTN, 0, 0, 3, 1, 0} ,
  {(char *)"2", 95, 15, 70, 60, 65535, 992, TS_FBUTN, 0, 0, 3, 2, 0} ,
  {(char *)"3", 175, 15, 70, 60, 65535, 1007, TS_FBUTN, 0, 0, 3, 3, 0} ,
  {(char *)"4", 255, 15, 70, 60, 65535, 30720, TS_FBUTN, 0, 0, 3, 4, 0} ,
  {(char *)"5", 15, 85, 70, 60, 65535, 30735, TS_FBUTN, 0, 0, 3, 5, 0} ,
  {(char *)"6", 95, 85, 70, 60, 65535, 31712, TS_FBUTN, 0, 0, 3, 6, 0} ,
  {(char *)"7", 175, 85, 70, 60, 0, 50712, TS_FBUTN, 0, 0, 3, 7, 0} ,
  {(char *)"8", 255, 85, 70, 60, 65535, 31727, TS_FBUTN, 0, 0, 3, 8, 0} ,
  {(char *)"9", 15, 155, 70, 60, 65535, 31, TS_FBUTN, 0, 0, 3, 9, 0} ,
  {(char *)"10", 95, 155, 70, 60, 65535, 2016, TS_FBUTN, 0, 0, 3, 10, 0} ,
  {(char *)"11", 175, 155, 70, 60, 65535, 2047, TS_FBUTN, 0, 0, 3, 11, 0} ,
  {(char *)"12", 255, 155, 70, 60, 0, 63488, TS_FBUTN, 0, 0, 3, 12, 0} ,
  {(char *)"13", 15, 225, 70, 60, 0, 63519, TS_FBUTN, 0, 0, 3, 13, 0} ,
  {(char *)"14", 95, 225, 70, 60, 0, 65504, TS_FBUTN, 0, 0, 3, 14, 0} ,
  {(char *)"15", 175, 225, 70, 60, 0, 65535, TS_FBUTN, 0, 0, 3, 15, 0} ,
  {(char *)"16", 255, 225, 70, 60, 0, 64800, TS_FBUTN, 0, 0, 3, 16, 0} ,
  {(char *)".", -1, 0, 0, 0, 0, 0, TS_FBUTN, 0, 0, 0, 0, 0}
};

TS_BUTTON mybuttons3[] = {{(char *)"1", 5, 20, 60, 40, 65535, 15, TS_FBUTN, 0, 0, 3, 1, 0} ,
  {(char *)"2", 75, 20, 60, 40, 65535, 992, TS_FBUTN, 0, 0, 3, 2, 0} ,
  {(char *)"3", 145, 20, 60, 40, 65535, 1007, TS_FBUTN, 0, 0, 3, 3, 0} ,
  {(char *)"4", 215, 20, 60, 40, 65535, 30720, TS_FBUTN, 0, 0, 3, 4, 0} ,
  {(char *)"5", 285, 20, 60, 40, 65535, 30735, TS_FBUTN, 0, 0, 3, 5, 0} ,
  {(char *)"6", 5, 70, 60, 40, 65535, 31712, TS_FBUTN, 0, 0, 3, 6, 0} ,
  {(char *)"7", 75, 70, 60, 40, 0, 50712, TS_FBUTN, 0, 0, 3, 7, 0} ,
  {(char *)"8", 145, 70, 60, 40, 65535, 31727, TS_FBUTN, 0, 0, 3, 8, 0} ,
  {(char *)"9", 215, 70, 60, 40, 65535, 31, TS_FBUTN, 0, 0, 3, 9, 0} ,
  {(char *)"10", 285, 70, 60, 40, 65535, 2016, TS_FBUTN, 0, 0, 3, 10, 0} ,
  {(char *)"11", 5, 120, 60, 40, 65535, 2047, TS_FBUTN, 0, 0, 3, 11, 0} ,
  {(char *)"12", 75, 120, 60, 40, 0, 63488, TS_FBUTN, 0, 0, 3, 12, 0} ,
  {(char *)"13", 145, 120, 60, 40, 0, 63519, TS_FBUTN, 0, 0, 3, 13, 0} ,
  {(char *)"14", 215, 120, 60, 40, 0, 65504, TS_FBUTN, 0, 0, 3, 14, 0} ,
  {(char *)"15", 285, 120, 60, 40, 0, 65535, TS_FBUTN, 0, 0, 3, 15, 0} ,
  {(char *)"16", 5, 170, 60, 40, 0, 64800, TS_FBUTN, 0, 0, 3, 16, 0} ,
  {(char *)"17", 75, 170, 60, 40, 0, 45029, TS_FBUTN, 0, 0, 3, 17, 0} ,
  {(char *)"18", 145, 170, 60, 40, 0, 63514, TS_FBUTN, 0, 0, 3, 18, 0} ,
  {(char *)"19", 215, 170, 60, 40, 65535, 0, TS_FBUTN, 0, 0, 3, 19, 0} ,
  {(char *)"20", 285, 170, 60, 40, 65535, 200, TS_FBUTN, 0, 0, 3, 20, 0} ,
  {(char *)"21", 5, 220, 60, 40, 65535, 3700, TS_FBUTN, 0, 0, 3, 21, 0} ,
  {(char *)"22", 75, 220, 60, 40, 65535, 325, TS_FBUTN, 0, 0, 3, 22, 0} ,
  {(char *)"23", 145, 220, 60, 40, 65535, 3670, TS_FBUTN, 0, 0, 3, 23, 0} ,
  {(char *)"24", 215, 220, 60, 40, 65535, 0, TS_FBUTN, 0, 0, 3, 24, 0} ,
  {(char *)"25", 285, 220, 60, 40, 65535, 319, TS_FBUTN, 0, 0, 3, 25, 0} ,
  {(char *)".", -1, 0, 0, 0, 0, 0, TS_FBUTN, 0, 0, 0, 0, 0}
};

TS_BUTTON mybuttons4[] = {{(char *)"1", 5, 5, 50, 50, 65535, 15, TS_FBUTN, 0, 0, 3, 1, 0} ,
  {(char *)"2", 65, 5, 50, 50, 65535, 992, TS_FBUTN, 0, 0, 3, 2, 0} ,
  {(char *)"3", 125, 5, 50, 50, 65535, 1007, TS_FBUTN, 0, 0, 3, 3, 0} ,
  {(char *)"4", 185, 5, 50, 50, 65535, 30720, TS_FBUTN, 0, 0, 3, 4, 0} ,
  {(char *)"5", 245, 5, 50, 50, 65535, 30735, TS_FBUTN, 0, 0, 3, 5, 0} ,
  {(char *)"6", 305, 5, 50, 50, 65535, 31712, TS_FBUTN, 0, 0, 3, 6, 0} ,
  {(char *)"7", 5, 65, 50, 50, 0, 50712, TS_FBUTN, 0, 0, 3, 7, 0} ,
  {(char *)"8", 65, 65, 50, 50, 65535, 31727, TS_FBUTN, 0, 0, 3, 8, 0} ,
  {(char *)"9", 125, 65, 50, 50, 65535, 31, TS_FBUTN, 0, 0, 3, 9, 0} ,
  {(char *)"10", 185, 65, 50, 50, 65535, 2016, TS_FBUTN, 0, 0, 3, 10, 0} ,
  {(char *)"11", 245, 65, 50, 50, 65535, 2047, TS_FBUTN, 0, 0, 3, 11, 0} ,
  {(char *)"12", 305, 65, 50, 50, 0, 63488, TS_FBUTN, 0, 0, 3, 12, 0} ,
  {(char *)"13", 5, 125, 50, 50, 0, 63519, TS_FBUTN, 0, 0, 3, 13, 0} ,
  {(char *)"14", 65, 125, 50, 50, 0, 65504, TS_FBUTN, 0, 0, 3, 14, 0} ,
  {(char *)"15", 125, 125, 50, 50, 0, 65535, TS_FBUTN, 0, 0, 3, 15, 0} ,
  {(char *)"16", 185, 125, 50, 50, 0, 64800, TS_FBUTN, 0, 0, 3, 16, 0} ,
  {(char *)"17", 245, 125, 50, 50, 0, 45029, TS_FBUTN, 0, 0, 3, 17, 0} ,
  {(char *)"18", 305, 125, 50, 50, 0, 63514, TS_FBUTN, 0, 0, 3, 18, 0} ,
  {(char *)"19", 5, 185, 50, 50, 65535, 0, TS_FBUTN, 0, 0, 3, 19, 0} ,
  {(char *)"20", 65, 185, 50, 50, 65535, 200, TS_FBUTN, 0, 0, 3, 20, 0} ,
  {(char *)"21", 125, 185, 50, 50, 65535, 3700, TS_FBUTN, 0, 0, 3, 21, 0} ,
  {(char *)"22", 185, 185, 50, 50, 65535, 325, TS_FBUTN, 0, 0, 3, 22, 0} ,
  {(char *)"23", 245, 185, 50, 50, 65535, 3670, TS_FBUTN, 0, 0, 3, 23, 0} ,
  {(char *)"24", 305, 185, 50, 50, 65535, 0, TS_FBUTN, 0, 0, 3, 24, 0} ,
  {(char *)".", -1, 0, 0, 0, 0, 0, TS_FBUTN, 0, 0, 0, 0, 0}
};

// -----------------------------------------
// --- USER Button Data Ends here
// -----------------------------------------
void setup(void)
{
  Serial.begin(9600);
  while ( !Serial && (millis() < 2000)) ;
  tft.begin();
  if (!ts.begin()) Serial.println("Unable to start touchscreen.");
  else Serial.println("Touchscreen started.");
  tft.fillScreen(ILI9341_BLUE);
  ButtonInit( 0 );
  BTest( (char *)"mybuttons2", 70, 60, 15, 15 ); // pass: Struct name, button width, height, starting top X, top Y
  tft.fillScreen(ILI9341_BLUE);
  BTest( (char *)"mybuttons3", 60, 40, 5, 20 );
  tft.fillScreen(ILI9341_BLUE);
  BTest( (char *)"mybuttons4", 50, 50, 5, 5 );
  ButtonInit( &mybuttons2[0] );
  tft.fillScreen(ILI9341_RED);
  ButtonDraw( 0 );
  int16_t x, y;
  while ( !TS_GetMap( x, y, true ));
  ButtonInit( &mybuttons3[0] );
  tft.fillScreen(ILI9341_GREEN);
  ButtonDraw( 0 );
  while ( !TS_GetMap( x, y, true ));
  ButtonInit( &mybuttons4[0] );
  tft.fillScreen(ILI9341_ORANGE);
  ButtonDraw( 0 );
}

void loop()
{
  // See if there's any  touch data for us
  static elapsedMillis userTime;
  uint16_t userDebounce = 555;
  static int16_t lastbValue;
  boolean istouched = false;
  int16_t bValue;
  int16_t x, y;
  istouched = TS_GetMap( x, y, true );
  if (istouched && ButtonHit(x , y, bValue))
  {
    if ( lastbValue == bValue ) userDebounce = userTime;
    if (userDebounce > 50) {
      Serial.print("\nbutton #");
      Serial.print(bValue);
    }
    if ( 1 == bValue ) {
      Serial.println();
      tft.fillScreen(ILI9341_CYAN);
      ButtonRotate( 1 + TS_Rotate );
      ButtonDraw( 0 );
      Serial.print("\nRotate on #1 to rotation #");
      Serial.println(TS_Rotate);
    }
    if ( 2 == bValue ) {
      ButtonInit( &mybuttons2[0] );
      Serial.println("ButtonInit( &mybuttons2[0] );");
      tft.fillScreen(ILI9341_RED);
      ButtonDraw( 0 );
    }
    if ( 3 == bValue ) {
      ButtonInit( &mybuttons3[0] );
      Serial.println("ButtonInit( &mybuttons3[0] );");
      tft.fillScreen(ILI9341_GREEN);
      ButtonDraw( 0 );
    }
    if ( 4 == bValue ) {
      ButtonInit( &mybuttons4[0] );
      Serial.println("ButtonInit( &mybuttons4[0] );");
      tft.fillScreen(ILI9341_ORANGE);
      ButtonDraw( 0 );
    }

    lastbValue = bValue;
    userTime = 0;
  }
}

// -----------------------------------------
// --- Button Code Starts here
// -----------------------------------------
TS_BUTTON TS_NoButton[] = { {(char *)"X", -1, 0, 0, 0, 0, 0, TS_FBUTN, 0, 0, 0} };
TS_BUTTON *TS_pbtn = TS_NoButton;

#define TS_LIMIT 10 // millisecond limit to check touch value
#define TS_MINPRESS 800 // Z Pressure return minimum to use point
#define TS_DEBOUNCE 3 // TX_Map() debounce factor
boolean TS_GetMap( int16_t &xx, int16_t &yy, boolean twoHits )
{
  static int16_t lastxx = 500, lastyy = 500; // if twoHits: require two hits same point
  static TS_Point pp;
  static elapsedMillis tsLimit;
  static int16_t xxo = 500, yyo = 500;  // Store old/last returned x,y point
  if ( tsLimit < TS_LIMIT ) return false;
  if (!ts.touched()) return false;
  if ( tsLimit >= TS_SLIDET ) TS_iiSlide = -1;
  tsLimit = 0;
  pp = ts.getPoint();
  if ( pp.z < TS_MINPRESS ) return false;
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
  if (  twoHits && (xx != lastxx || yy != lastyy) ) {
    lastxx = xx; lastyy = yy;
  }
  if ( 500 == lastyy) TS_iiSlide = -1;
  lastxx = 500; lastyy = 500;
  return true;
}

// int8_t ButtonType( int16_t idBut ) {   return buttons[idBut].btype; }

void ButtonDraw( int16_t idBut ) {
  int ii = 0;
  while ( ii < TS_bCount ) {
    if ((( TS_pbtn[ii].tx + TS_pbtn[ii].ww ) <= TS_xMax ) && (( TS_pbtn[ii].ty + TS_pbtn[ii].hh ) <= TS_yMax)) {
      if ( !idBut || idBut == TS_pbtn[ii].bId ) {
        if ( TS_TOGOFF == TS_pbtn[ii].btype || TS_FBUTN == TS_pbtn[ii].btype || TS_RBUTN == TS_pbtn[ii].btype || TS_BSLIDE == TS_pbtn[ii].btype ) {
          if (TS_RBUTN == TS_pbtn[ii].btype) {
            tft.fillRoundRect(TS_pbtn[ii].tx, TS_pbtn[ii].ty, TS_pbtn[ii].ww, TS_pbtn[ii].hh, TS_pbtn[ii].hh / 2, TS_pbtn[ii].bgc);
          }
          else
            tft.fillRect(TS_pbtn[ii].tx, TS_pbtn[ii].ty, TS_pbtn[ii].ww, TS_pbtn[ii].hh, TS_pbtn[ii].bgc);
          tft.setCursor(TS_pbtn[ii].tx + 6, TS_pbtn[ii].ty + (TS_pbtn[ii].hh / 3));
          tft.setTextColor(TS_pbtn[ii].fgc);  // ForeGround Text color same as Frame
          tft.setTextSize(TS_pbtn[ii].fontsz);
          tft.println(TS_pbtn[ii].text);
        }
        else if ( TS_TOGON == TS_pbtn[ii].btype || TS_ASLIDE == TS_pbtn[ii].btype )
          tft.fillRect(TS_pbtn[ii].tx, TS_pbtn[ii].ty, TS_pbtn[ii].ww, TS_pbtn[ii].hh, TS_pbtn[ii].fgc);
      }
    }
    ii++;
  }
  ii = 0;
  while ( ii < TS_bCount ) {
    if ((( TS_pbtn[ii].tx + TS_pbtn[ii].ww ) <= TS_xMax ) && (( TS_pbtn[ii].ty + TS_pbtn[ii].hh ) <= TS_yMax)) {
      if ( !idBut || idBut == TS_pbtn[ii].bId ) {
        if ( TS_FRAME == TS_pbtn[ii].btype || TS_FBUTN == TS_pbtn[ii].btype )
          tft.drawRect(TS_pbtn[ii].tx, TS_pbtn[ii].ty, TS_pbtn[ii].ww, TS_pbtn[ii].hh, TS_pbtn[ii].fgc);
      }
    }
    ii++;
  }
}

boolean ButtonHit( int16_t xx, uint16_t yy, int16_t &bHit ) {
  static elapsedMillis toggleTime;  // Debounce/Jitter Toggle and Slider
  int16_t ii = 0, iiOn = -1, iiHit = -1, idTog = 0, iiFrame = -1;
  if ( toggleTime > 10000 ) toggleTime = 1000;
  while ( ii < TS_bCount ) {
    if  ( (xx >= TS_pbtn[ii].tx && xx <= TS_pbtn[ii].tx + TS_pbtn[ii].ww) && (yy >= TS_pbtn[ii].ty && yy <= TS_pbtn[ii].ty + TS_pbtn[ii].hh)) {
      if ( TS_ASLIDE == TS_pbtn[ii].btype ) {
        TS_iiSlide = TS_pbtn[ii].info; // track slide start - do nothing now
        return false;
      }
      else if ( (TS_FBUTN == TS_pbtn[ii].btype) || TS_RBUTN == TS_pbtn[ii].btype ) {
        bHit =  TS_pbtn[ii].bId;
        TS_iiSlide = -1;
        return true;
      }
      else if (TS_TOGOFF == TS_pbtn[ii].btype) {
        iiHit = ii;
        idTog = (TS_pbtn[ii].info);
        TS_iiSlide = -1;
      }
      else if ( (TS_iiSlide == TS_pbtn[ii].info) && (TS_BSLIDE == TS_pbtn[ii].btype)) {
        iiHit = ii;
        idTog = (TS_pbtn[ii].info);
      }
    }
    ii++;
  }
  ii = 0;
  while ( ii < TS_bCount ) {
    if ( ((TS_TOGON == TS_pbtn[ii].btype) || (TS_ASLIDE == TS_pbtn[ii].btype)) && (idTog == TS_pbtn[ii].info) )
      iiOn = ii;
    if ( (TS_FRAME == TS_pbtn[ii].btype) && (idTog == TS_pbtn[ii].info) )
      iiFrame = ii;
    ii++;
  }

  if ( -1 != iiOn && -1 != iiFrame ) {
    if ( toggleTime < TS_JITTER ) {
      TS_iiSlide = -1;
      return false;
    }
    TS_pbtn[iiFrame].TS_data = !TS_pbtn[iiFrame].TS_data;
    bHit = TS_pbtn[iiHit].bId;
    if ( TS_iiSlide == TS_pbtn[iiOn].info ) {
      TS_pbtn[iiOn].btype = TS_BSLIDE;
      TS_pbtn[iiHit].btype = TS_ASLIDE;
    }
    else {
      TS_pbtn[iiOn].btype = TS_TOGOFF;
      TS_pbtn[iiHit].btype = TS_TOGON;
    }
    ButtonDraw( TS_pbtn[iiOn].bId );  // Limit Redraw to Framed TS_pbtn - not all buttons
    ButtonDraw( TS_pbtn[iiHit].bId );
    ButtonDraw( TS_pbtn[iiFrame].bId );
    TS_iiSlide = -1;
    toggleTime = 0;
    return true;
  }
  return false;
}

void  ButtonRotate( int setrotate )
{
  TS_Rotate = setrotate;
  if ( TS_Rotate > 4 ) TS_Rotate = 1;
  if ( TS_Rotate < 1 ) TS_Rotate = 4;
  tft.setRotation(TS_Rotate);
  if ((TS_Rotate % 2)) {
    TS_xMax = tsMap[1].xb;
    TS_yMax = tsMap[1].yr;
  }
  else {
    TS_xMax = tsMap[1].yr;
    TS_yMax = tsMap[1].xb;
  }
  Serial.print( "\nTS_xMax =" );
  Serial.println( TS_xMax );
  Serial.print( "TS_yMax =" );
  Serial.println( TS_yMax );
}

void ButtonInit( TS_BUTTON *userbuttons )
{
  if ( 0 == userbuttons ) {
    TS_pbtn = TS_NoButton;
    tft.fillScreen(ILI9341_BLUE);
    ButtonRotate( TS_Rotate );
  }
  else TS_pbtn = userbuttons;
  TS_bCount = 0;
  while ( -1 != TS_pbtn[TS_bCount].tx ) {
    TS_bCount++;
  }
  Serial.print( "Buttons count =" );
  Serial.println( TS_bCount );
  // Serial.println( sizeof( TS_BUTTON) );
  ButtonDraw( 0 );
}
// -----------------------------------------
// --- Button Code Ends here
// -----------------------------------------

// =====
// --- Test Buttons in a grid using the color array from ILI9341
//    pass: Struct name, button width, height, starting top X, top Y
// DOES NOT TEST FOR BAD BUTTONS CREATED - they are clipped at run time if left in place
// OUTPUT TO USB is a Cut and Paste structure for the button maps used in this example
// With valid modification and use it can create a button map.
// =====
uint16_t array_colors[] = {0x0000, 0x000F, 0x03E0, 0x03EF, 0x7800, 0x780F, 0x7BE0, 0xC618, 0x7BEF, 0x001F, 0x07E0, 0x07FF, 0xF800, 0xF81F, 0xFFE0, 0xFFFF, 0xFD20, 0xAFE5, 0xF81A, 0x0000 };
void BTest( char *userbutton, int intWidth, int intHeight, int intLeft, int intTop )
{
  uint16_t kk = 0; // index for the color array
  uint16_t xx, yy;
  char txtbuf[12];
  tft.setTextSize(3);
  Serial.print( "TS_BUTTON " );
  Serial.print( userbutton );
  Serial.print( "[] = {" );
  for (yy = intTop; yy < 240; yy += (intHeight + 10))
    for (xx = intLeft; xx < 320; xx += (intWidth + 10) )
    {
      tft.fillRect( xx , yy , intWidth, intHeight, 0xFFFF ^ array_colors[ kk ] );
      tft.fillRoundRect( xx , yy , intWidth, intHeight, intHeight / 2, array_colors[ kk ] );
      tft.setCursor( xx + 7, yy - 11 + intHeight / 2 ); // EDIT THIS FOR TEXT OFFSET from xx, yy
      tft.setTextColor( (array_colors[ kk ] & 0xFFEF ) > 0x8419 ? ILI9341_BLACK : ILI9341_WHITE );
      kk++;
      tft.println(kk);
      sprintf( txtbuf, "%d", kk );
      //        { text *, tx, ty, ww, hh, fgc, bgc, btype, info, bState, fontsz, bId, TS_data }    << Struct looks like this
      ShowBStruct( txtbuf, xx, yy, intWidth, intHeight, ( (array_colors[ kk ] & 0xFFEF ) > 0x8419 ? ILI9341_BLACK : ILI9341_WHITE ), array_colors[ kk ], (char *)"TS_FBUTN", 0, 0, 3, kk, 0);
      Serial.println( " ," );
    }
  ShowBStruct( (char *)".", -1, 0, 0, 0, 0, 0, (char *)"TS_FBUTN", 0, 0, 0, 0, 0);
  Serial.print( "\n};\n\n" );
  delay(2000);
}
// =====
// --- serial print button struct
// =====
//        { text *, tx, ty, ww, hh, fgc, bgc, btype, info, bState, fontsz, bId, TS_data }    << Struct looks like this
void ShowBStruct( char*text, int16_t tx, int16_t ty, int16_t ww, int16_t hh, uint16_t fgc, uint16_t bgc, char* btype, uint16_t info, byte bState, byte fontsz, uint16_t bId, uint8_t data) {
  Serial.print( "{" );
  Serial.print( "(char *)\"" );
  Serial.print( text );
  Serial.print( "\"" );
  Serial.print( ", " );
  if ( -1 == tx )
    Serial.print( "-1" );
  else
    Serial.print( tx );
  Serial.print( ", " );
  Serial.print( ty );
  Serial.print( ", " );
  Serial.print( ww );
  Serial.print( ", " );
  Serial.print( hh );
  Serial.print( ", " );
  Serial.print( fgc );
  Serial.print( ", " );
  Serial.print( bgc );
  Serial.print( ", " );
  Serial.print( btype );
  Serial.print( ", " );
  Serial.print( info );
  Serial.print( ", " );
  Serial.print( bState );
  Serial.print( ", " );
  Serial.print( fontsz );
  Serial.print( ", " );
  Serial.print( bId );
  Serial.print( ", " );
  Serial.print( data );
  Serial.print( "}" );
};

