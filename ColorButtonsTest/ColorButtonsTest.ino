
// XPT2046_Touchscreen ; Button Functions
//
// TAG wide verus narrow button  1%3 or 2,4 allow moved placement on rotation
// Button Animation :: ButtonDraw( bValue, [special color, or animate type???]  );
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
//XPT2046_Touchscreen ts(CS_PIN);  // Param 2 - NULL - No interrupts
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
int16_t TS_Rotate = 1;
static int16_t TS_iiSlide = -1;
#define TS_SLIDET 40  // Reject SLIDE buttons farther apart than this

struct TS_MAP {int16_t xt; int16_t xb; int16_t yl; int16_t yr; };
// Zero is calibration for mapping to pixel, 1-4 follow screen rotation for mapping math
TS_MAP tsMap[5] = { {200, 3700, 325, 3670 }, { 0, 319, 0, 239 }, { 319, 0, 0, 239 }, { 319, 0, 239, 0 }, { 0, 319, 239, 0 } };
int16_t TS_xMax = 0;  // These TS_ values are updated per current rotation to xMax and yMax
int16_t TS_yMax = 0;

struct TS_BUTTON {
  int16_t tx, ty; // Top Left X,Y
  int16_t ww, hh; // Width, Height
  uint16_t fgc;
  uint16_t bgc; // background color button or empty frame
  uint8_t btype;  // for toggle/slider two bytes TYPE & info PAIRING number
  uint8_t info;  // for toggle/slider two bytes TYPE & info PAIRING number
  //  char *text; // May need large text!  Do union {text[8] || flag,char*} allow user call to set flag to '\1' unprintable to say use char*
  char text[10]; // May need large text!  Do union {text[8] || flag,char*} allow user call to set flag to '\1' unprintable to say use char*
  // Add font color :: FONT MATCHES FRAME
  // Add byte font size
  byte bId; // Button ID
  uint8_t data; // Toggle FRAME stores state value here
};
char *calf[] = { (char*)"123", (char*)"abcxyz" };

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
// --- USER Button Data Starts here
// -----------------------------------------
//        { tx, ty, ww, hh, fgc, bgc, btype, info, text[8], bId, data }    << Struct looks like this
TS_BUTTON buttons[] = {{15, 5, 50, 50, 65535, 15, TS_FBUTN, 0, "1", 1, 0} ,
  {75, 5, 50, 50, 65535, 992, TS_FBUTN, 0, "2", 2, 0} ,
  {135, 5, 50, 50, 65535, 1007, TS_FBUTN, 0, "3", 3, 0} ,
  {195, 5, 50, 50, 65535, 30720, TS_FBUTN, 0, "4", 4, 0} ,
  {255, 5, 50, 50, 65535, 30735, TS_FBUTN, 0, "5", 5, 0} ,
  {15, 65, 50, 50, 65535, 31712, TS_FBUTN, 0, "6", 6, 0} ,
  {75, 65, 50, 50, 0, 50712, TS_FBUTN, 0, "7", 7, 0} ,
  {135, 65, 50, 50, 65535, 31727, TS_FBUTN, 0, "8", 8, 0} ,
  {195, 65, 50, 50, 65535, 31, TS_FBUTN, 0, "9", 9, 0} ,
  {255, 65, 50, 50, 65535, 2016, TS_FBUTN, 0, "10", 10, 0} ,
  {15, 125, 50, 50, 65535, 2047, TS_FBUTN, 0, "11", 11, 0} ,
  {75, 125, 50, 50, 0, 63488, TS_FBUTN, 0, "12", 12, 0} ,
  {135, 125, 50, 50, 0, 63519, TS_FBUTN, 0, "13", 13, 0} ,
  {195, 125, 50, 50, 0, 65504, TS_FBUTN, 0, "14", 14, 0} ,
  {255, 125, 50, 50, 0, 65535, TS_FBUTN, 0, "15", 15, 0} ,
  {15, 185, 50, 50, 0, 64800, TS_FBUTN, 0, "16", 16, 0} ,
  {75, 185, 50, 50, 0, 45029, TS_FBUTN, 0, "17", 17, 0} ,
  {135, 185, 50, 50, 0, 63514, TS_FBUTN, 0, "18", 18, 0} ,
  {195, 185, 50, 50, 65535, 0, TS_FBUTN, 0, "19", 19, 0} ,
  {255, 185, 50, 50, 65535, 200, TS_FBUTN, 0, "20", 20, 0}
};

// -----------------------------------------
// --- Button Data Ends here
// -----------------------------------------
void setup(void)
{
  Serial.begin(9600);
  while ( !Serial && (millis() < 2000)) ;
  tft.begin();
  if (!ts.begin()) Serial.println("Unable to start touchscreen.");
  else Serial.println("Touchscreen started.");
  ButtonInit();
  Serial.println(sizeof calf);
  Serial.println(calf[0]);
  Serial.println(calf[1]);
  tft.fillScreen(ILI9341_BLUE);
  BTest();
  tft.fillScreen(ILI9341_BLUE);
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
      ButtonRotate( 1 + TS_Rotate );
      Serial.print("\nRotate on #1 to rotation #");
      Serial.println(TS_Rotate);
    }
    lastbValue = bValue;
    userTime = 0;
  }
}

// -----------------------------------------
// --- Button Code Starts here
// -----------------------------------------
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
    if ((( buttons[ii].tx + buttons[ii].ww ) <= TS_xMax ) && (( buttons[ii].ty + buttons[ii].hh ) <= TS_yMax)) {
      if ( !idBut || idBut == buttons[ii].bId ) {
        if ( TS_TOGOFF == buttons[ii].btype || TS_FBUTN == buttons[ii].btype || TS_RBUTN == buttons[ii].btype || TS_BSLIDE == buttons[ii].btype ) {
          if (TS_RBUTN == buttons[ii].btype) {
            tft.fillRoundRect(buttons[ii].tx, buttons[ii].ty, buttons[ii].ww, buttons[ii].hh, buttons[ii].hh / 2, buttons[ii].bgc);
          }
          else
            tft.fillRect(buttons[ii].tx, buttons[ii].ty, buttons[ii].ww, buttons[ii].hh, buttons[ii].bgc);
          tft.setCursor(buttons[ii].tx + 6, buttons[ii].ty + (buttons[ii].hh / 3));
          tft.setTextColor(buttons[ii].fgc);  // Text Same color as Frame?
          tft.setTextSize(2);  // BUGBUG - this to come from struct!
          tft.println(buttons[ii].text);
        }
        else if ( TS_TOGON == buttons[ii].btype || TS_ASLIDE == buttons[ii].btype )
          tft.fillRect(buttons[ii].tx, buttons[ii].ty, buttons[ii].ww, buttons[ii].hh, buttons[ii].fgc);
      }
    }
    ii++;
  }
  ii = 0;
  while ( ii < TS_bCount ) {
    if ((( buttons[ii].tx + buttons[ii].ww ) <= TS_xMax ) && (( buttons[ii].ty + buttons[ii].hh ) <= TS_yMax)) {
      if ( !idBut || idBut == buttons[ii].bId ) {
        if ( TS_FRAME == buttons[ii].btype || TS_FBUTN == buttons[ii].btype )
          tft.drawRect(buttons[ii].tx, buttons[ii].ty, buttons[ii].ww, buttons[ii].hh, buttons[ii].fgc);
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
    if  ( (xx >= buttons[ii].tx && xx <= buttons[ii].tx + buttons[ii].ww) && (yy >= buttons[ii].ty && yy <= buttons[ii].ty + buttons[ii].hh)) {
      if ( TS_ASLIDE == buttons[ii].btype ) {
        TS_iiSlide = buttons[ii].info; // track slide start - do nothing now
        return false;
      }
      else if ( (TS_FBUTN == buttons[ii].btype) || TS_RBUTN == buttons[ii].btype ) {
        bHit =  buttons[ii].bId;
        TS_iiSlide = -1;
        return true;
      }
      else if (TS_TOGOFF == buttons[ii].btype) {
        iiHit = ii;
        idTog = (buttons[ii].info);
        TS_iiSlide = -1;
      }
      else if ( (TS_iiSlide == buttons[ii].info) && (TS_BSLIDE == buttons[ii].btype)) {
        iiHit = ii;
        idTog = (buttons[ii].info);
      }
    }
    ii++;
  }
  ii = 0;
  while ( ii < TS_bCount ) {
    if ( ((TS_TOGON == buttons[ii].btype) || (TS_ASLIDE == buttons[ii].btype)) && (idTog == buttons[ii].info) )
      iiOn = ii;
    if ( (TS_FRAME == buttons[ii].btype) && (idTog == buttons[ii].info) )
      iiFrame = ii;
    ii++;
  }

  if ( -1 != iiOn && -1 != iiFrame ) {
    if ( toggleTime < TS_JITTER ) {
      TS_iiSlide = -1;
      return false;
    }
    buttons[iiFrame].data = !buttons[iiFrame].data;
    bHit = buttons[iiHit].bId;
    if ( TS_iiSlide == buttons[iiOn].info ) {
      buttons[iiOn].btype = TS_BSLIDE;
      buttons[iiHit].btype = TS_ASLIDE;
    }
    else {
      buttons[iiOn].btype = TS_TOGOFF;
      buttons[iiHit].btype = TS_TOGON;
    }
    ButtonDraw( buttons[iiOn].bId );  // Limit Redraw to Framed buttons - not all buttons
    ButtonDraw( buttons[iiHit].bId );
    ButtonDraw( buttons[iiFrame].bId );
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
  tft.fillScreen(ILI9341_BLUE);
  if ((TS_Rotate % 2)) {
    TS_xMax = tsMap[1].xb;
    TS_yMax = tsMap[1].yr;
  }
  else {
    TS_xMax = tsMap[1].yr;
    TS_yMax = tsMap[1].xb;
  }
  Serial.print( "TS_xMax =" );
  Serial.println( TS_xMax );
  Serial.print( "TS_yMax =" );
  Serial.println( TS_yMax );
  ButtonDraw( 0 );
}

void ButtonInit( void )
{
  TS_bCount = sizeof( buttons) / sizeof( TS_BUTTON);
  ButtonRotate( TS_Rotate );
  Serial.print( "Buttons Size =" );
  Serial.println( sizeof( buttons) );
  Serial.println( TS_bCount );
  Serial.println( sizeof( TS_BUTTON) );
}
// -----------------------------------------
// --- Button Code Ends here
// -----------------------------------------

// =====
// --- Test Buttons with 20 in a grid using the color array from ILI9341
// =====
uint16_t array_colors[] = {0x0000, 0x000F, 0x03E0, 0x03EF, 0x7800, 0x780F, 0x7BE0, 0xC618, 0x7BEF, 0x001F, 0x07E0, 0x07FF, 0xF800, 0xF81F, 0xFFE0, 0xFFFF, 0xFD20, 0xAFE5, 0xF81A, 0x0000 };
#define intLeft 5
#define intTop 5
#define intWidth 50
#define intHeight 50
void BTest()
{
  uint16_t kk = 0; // index for the color array
  uint16_t ii, jj;
  char txtbuf[12];
  tft.setTextSize(3);
  Serial.print( "TS_BUTTON buttons[] = {" );
  for (jj = intTop; jj < 240; jj += 60)
    for (ii = intLeft; ii < 300; ii += 60 )
    {
      if ( kk )   Serial.println( " ," );
      tft.fillRect( ii , jj , intWidth, intHeight, 0xFFFF ^ array_colors[ kk ] );
      tft.fillRoundRect( ii , jj , intWidth, intHeight, intHeight / 2, array_colors[ kk ] );
      tft.setCursor( ii + 7, jj - 11 + intHeight / 2 ); // EDIT THIS FOR TEXT OFFSET from ii, jj
      tft.setTextColor( (array_colors[ kk ] & 0xFFEF ) > 0x8419 ? ILI9341_BLACK : ILI9341_WHITE );
      kk++;
      tft.println(kk);
      sprintf( txtbuf, "%d", kk );
      ShowBStruct( ii, jj, intWidth, intHeight, ( (array_colors[ kk ] & 0xFFEF ) > 0x8419 ? ILI9341_BLACK : ILI9341_WHITE ), array_colors[ kk ], (char *)"TS_FBUTN", 0, txtbuf, kk, 0);
    }
  Serial.print( "\n};" );
  delay(2000);
}
// =====
// --- serial print button struct
// =====
//        { tx, ty, ww, hh, fgc, bgc, btype, info, text[8], bId, data }    << Struct looks like this
void ShowBStruct( uint16_t tx, uint16_t ty, uint16_t ww, uint16_t hh, uint16_t fgc, uint16_t bgc, char* btype, uint16_t info, char*text, uint16_t bId, uint16_t data) {
  Serial.print( "{" );
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
  Serial.print( "\"" );
  Serial.print( text );
  Serial.print( "\"" );
  Serial.print( ", " );
  Serial.print( bId );
  Serial.print( ", " );
  Serial.print( data );
  Serial.print( "}" );
};

