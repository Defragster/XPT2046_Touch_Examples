// XPT2046_Touchscreen ; Button Functions
//
// TAG wide verus narrow button  1%3 or 2,4 allow moved placement on rotation
// Button Animation :: ButtonDraw( bValue, [special color, or animate type???]  );
#include <SPI.h>
#include <Wire.h>
#include <ILI9341_t3.h>
#include <XPT2046_Touchscreen.h>

// -----  TS_ IRQ DATA START ------
#include <TimerOne.h>
volatile static byte isrWake = 0; // This cascades a few more Touch POLL reads after no touch before enable interrupt
#define TS_INTWAIT  0
#define TS_INTACTIVE  1
#define TS_INTBUSY  2
#define TS_INTRETRY  3
static TS_Point TS_pp;
// -----  TS_ IRQ DATA END ------
uint16_t TS_usVal = 25000; // POLL rate from the timer interrupt on Touch Detect
#define TIRQ_PIN  2

#define CS_PIN 8
XPT2046_Touchscreen ts(CS_PIN);
#define TFT_CS 10
#define TFT_DC  9
// MOSI=11, MISO=12, SCK=13
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);

// -----------------------------------------
// --- Button Data Starts here
// -----------------------------------------
int16_t TS_Rotate = 1;
static int16_t TS_iiSlide = -1;
#define TS_SLIDET 40  // Reject SLIDE buttons farther apart than this

struct TS_MAP {
  int16_t xt;
  int16_t xb;
  int16_t yl;
  int16_t yr;
};
// Zero is calibration for mapping to pixal, 1-4 follow screen rotation for mapping math
TS_MAP tsMap[5] = { {200, 3700, 325, 3670 }, { 0, 319, 0, 239 }, { 319, 0, 0, 239 }, { 319, 0, 239, 0 }, { 0, 319, 239, 0 } };

struct TS_BUTTON {
  int16_t tx, ty; // Top Left X,Y
  int16_t ww, hh; // Width, Height
  uint16_t fgc;
  uint16_t bgc; // background color button or empty frame
  uint8_t btype;  // for toggle/slider two bytes TYPE & info PAIRING number
  uint8_t info;  // for toggle/slider two bytes TYPE & info PAIRING number
  char text[8]; // May need large text!  Do union {text[8] || flag,char*} allow user call to set flag to '\1' unprintable to say use char*
  // Add font color :: FONT MATCHES FRAME
  // Add byte font size
  byte bId; // Button ID
  uint8_t data; // Toggle FRAME stores state value here
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
// --- USER Button Data Starts here
// -----------------------------------------
//        { tx, ty, ww, hh, fgc, bgc, btype, info, text[8], bId, data }    << Struct looks like this
TS_BUTTON buttons[] = {
  {10, 25, 100, 50, ILI9341_YELLOW, ILI9341_PINK, TS_RBUTN, 0, "Redraw", 200, 0 },
  {10, 80, 100, 50, ILI9341_BLACK, ILI9341_BLACK, TS_FRAME, 1, "???", 101, 1 },
  {10, 80, 50, 50, ILI9341_RED, ILI9341_BLUE, TS_TOGON, 1, "ROT", 1 , 0}, {60, 80, 50, 50, ILI9341_GREEN, ILI9341_BLUE, TS_TOGOFF , 1, "SIT", 51, 0 },
  {10, 135, 100, 50, ILI9341_RED, ILI9341_BLACK, TS_FRAME, 2, "???", 102, 1 },
  {60, 135, 50, 50, ILI9341_ORANGE, ILI9341_BLACK, TS_ASLIDE, 2, "Sl.A", 2, 0 }, {10, 135, 50, 50, ILI9341_CYAN, ILI9341_BLACK, TS_BSLIDE, 2, "Sl.B", 52, 0 },
  {115, 25, 50, 105, ILI9341_YELLOW, ILI9341_BLACK, TS_FRAME, 3, "???", 103, 1 },
  {115, 25, 50, 55, ILI9341_CYAN, ILI9341_BLACK, TS_TOGOFF, 3, "Clr", 3, 0 }, {115, 80, 50, 50, ILI9341_CYAN, ILI9341_NAVY, TS_TOGON, 3, "Hi!", 53, 0 },
  {165, 25, 50, 105, ILI9341_CYAN, ILI9341_PINK, TS_FBUTN, 0, "Dbo", 201, 0 },
  {115, 135, 100, 50, ILI9341_BLACK, ILI9341_DARKCYAN, TS_FBUTN, 0, "BOUNCE", 202, 0 },
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
  TS_iBegin( TIRQ_PIN, TS_usVal );
}

void loop()
{
  // See if there's any  touch data for us
  static int8_t cc[7] = {0, 0, 0, 0, 0, 0, 0};
  int8_t cii = 0;
  static elapsedMillis userTime;
  uint16_t userDebounce = 555;
  static int16_t lastbValue;
  boolean istouched = false;
  int16_t bValue;
  int16_t x, y;
  int16_t xout = 10, yout = 225;
  istouched = TS_GetMap( x, y, true );
  if (istouched && ButtonHit(x , y, bValue))
  {
    if ( lastbValue == bValue ) userDebounce = userTime; // Can't user debouce Slider/Toggle use TS_JITTER
    switch ( bValue ) { // USER BUTTON REPONSE code is HERE - this is just example work
      case 1:
        yout = 220, xout = 10;
        cii = 1;
        Serial.print("\nGreen btn hit : Rotate =");
        tft.setCursor(xout, yout);
        tft.setTextColor(ILI9341_BLACK);
        tft.setTextSize(3);
        tft.print("... WAIT ...");
        delay(300);
        ButtonRotate( 1 + TS_Rotate );
        Serial.print(TS_Rotate);
        yout = 200, xout = 10;
        break;
      case 51:
        Serial.print("\nRed btn hit");
        cii = 1;
        yout = 200, xout = 10;
        break;
      case 2: // Orange
      case 52:  // CYAN
        cii = 2;
        yout = 210, xout = 10;
        Serial.print("\nCyan/Orange btn SLID!");
        break;
      case 3:
        cii = 3;
        Serial.print("\nCyan btn hit");
        tft.fillScreen(ILI9341_BLUE); // clear screen
        ButtonDraw( 0 );
        yout = 190, xout = 115;
        break;
      case 53:
        cii = 3;
        Serial.print("\nNavy btn hit");
        yout = 190, xout = 115;
        break;
      case 200:
        cii = 4;
        if ( lastbValue == bValue )
          Serial.print("200."); // DOES AUTO REPEAT
        else
          Serial.print("\nbutton 200_");
        ButtonDraw( bValue );
        yout = 190, xout = 10;
        break;
      case 201:
        cii = 5;
        yout = 200, xout = 115;
        if (userDebounce < 50) {
          cc[cii] -= 1;
          break;
        }
        Serial.print("\nbutton 201");
        break;
      case 202:
        cii = 0;
        if ( lastbValue == bValue )
          Serial.print("202."); // DOES AUTO REPEAT
        else
          Serial.print("\nbutton 202_");
        yout = 210, xout = 115;
        break;
      default:
        cii = 6;
        yout = 220, xout = 10;
        Serial.println(" == UNTRAPPED button");
        Serial.print(bValue);
        Serial.println(" == CHECK buttons STRUCT?");
        break;
    }
    if (xout) { // on screen fireworks on button press
      cc[cii] += 1; // Show extra char below for each processed button touch
      if ( cc[cii] > 11 ) {
        cc[cii] = 1;
        Serial.println();
        tft.fillRect(xout, yout, 100, 10, ILI9341_BLACK);
      }
      tft.setCursor(xout, yout);
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(1);
      tft.print(bValue); // Shows BUTTON ID received on screen
      for ( int ii = 0; ii <= cc[cii]; ii++) tft.print("|"); // Added char for each unbounced button touch
      tft.print("."); // dot is new on end - | shows change from each prior touch
      if ( lastbValue != bValue ) {
        tft.fillRect(10, 220, 20, 10, ILI9341_BLACK);
        tft.setCursor(10, 220);
        tft.print(bValue); // Shows BUTTON ID received on screen
      }
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
  if (!TS_itouched()) return false;
  if ( tsLimit >= TS_SLIDET ) TS_iiSlide = -1;
  tsLimit = 0;
  pp = TS_igetPoint();
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
    // if ( 0 > TS_iiSlide)       return false;
  }
  if ( 500 == lastyy) TS_iiSlide = -1;
  lastxx = 500; lastyy = 500;
  return true;
}

// int8_t ButtonType( int16_t idBut ) {   return buttons[idBut].btype; }

void ButtonDraw( int16_t idBut ) {
  int ii = 0;
  while ( ii < TS_bCount ) {
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
    ii++;
  }
  ii = 0;
  while ( ii < TS_bCount ) {
    if ( !idBut || idBut == buttons[ii].bId ) {
      if ( TS_FRAME == buttons[ii].btype || TS_FBUTN == buttons[ii].btype )
        tft.drawRect(buttons[ii].tx, buttons[ii].ty, buttons[ii].ww, buttons[ii].hh, buttons[ii].fgc);
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
  ButtonDraw( 0 );
}

void ButtonInit( void )
{
  TS_bCount = sizeof( buttons) / sizeof( TS_BUTTON);
  ButtonRotate( TS_Rotate );
}

// -----------------------------------------
// --- Button Code Ends here
// -----------------------------------------


// -----------------------------------------
// --- Touch interrupt Code Starts here
// -----------------------------------------
void TS_iBegin( int16_t PinInt, uint16_t usVal )
{
  if ( usVal ) {
    Timer1.initialize( usVal);
    Timer1.attachInterrupt( TS_isrTime ); // Touch Test 'POLL' on Timer interrupt
  }
  if ( PinInt >= 0 ) {
    pinMode( PinInt, INPUT );
    attachInterrupt( PinInt, TS_isrPin, FALLING );
  }
}

void TS_isrTime( void )
{
  isrWake = TS_INTACTIVE;
  return;
}

void TS_isrPin( void )
{
  if ( TS_usVal ) { // When first TOUCH_INT recorded turn off the timer polling
    TS_usVal = 0;
    Timer1.stop();
    Timer1.detachInterrupt();
    Serial.println( "TIMER:isrOff" );
  }
  if ( TS_INTWAIT == isrWake )
    isrWake = TS_INTACTIVE;
  return;
}

bool TS_itouched() {
  bool iftouched;
  if ( TS_INTWAIT == isrWake ) {
    return false;
  }
  else if ( TS_INTACTIVE == isrWake )
    isrWake = TS_INTBUSY;
  iftouched = ts.touched();
  if ( !iftouched ) {
    if ( TS_INTBUSY == isrWake )
      isrWake = TS_INTRETRY;
    else if ( TS_INTRETRY == isrWake )
      isrWake = TS_INTWAIT;
  }
  else if ( TS_INTRETRY == isrWake ) {
    isrWake = TS_INTBUSY;
  }
  return iftouched;
}


TS_Point TS_igetPoint() {
  if ( TS_INTWAIT == isrWake )
    return TS_Point(0, 0, 0);
  else if ( TS_INTACTIVE == isrWake )
    isrWake = TS_INTBUSY;
  TS_pp = ts.getPoint();
  if ( !TS_pp.z )
  {
    if ( TS_INTBUSY == isrWake )
      isrWake = TS_INTRETRY;
    else if ( TS_INTRETRY == isrWake )
      isrWake = TS_INTWAIT;
  }
  else if ( TS_INTRETRY == isrWake ) {
    isrWake = TS_INTBUSY;
  }
  return ts.getPoint();
}

// -----------------------------------------
// --- Touch interrupt Code ends here
// -----------------------------------------
