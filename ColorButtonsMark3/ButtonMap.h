// -----------------------------------------
// --- Support structures and values for Button Mapping
// -----------------------------------------

#ifndef _ButtonMap_h_
#define _ButtonMap_h_

#define TS_LIMIT 10 // millisecond limit to check touch value
#define TS_MINPRESS 800 // Z Pressure return minimum to use point
#define TS_DEBOUNCE 3 // TS_GetMap() debounce factor x,y point proximity called 'same point'

#define TS_SLIDET 40  // millis Reject SLIDE buttons farther apart than this

struct TS_MAP {
  int16_t xt;
  int16_t xb;
  int16_t yl;
  int16_t yr;
};

//        { text *, tx, ty, ww, hh, fgc, bgc, btype, info, bState, fontsz, bId, TS_data }    << Struct looks like this
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
// -----------------------------------------
// --- Button Data Ends here
// -----------------------------------------

extern int16_t TS_Rotate;
extern int16_t TS_xMax; // Current Screen orientation x,y Max values
extern int16_t TS_yMax;

void ButtonInit( TS_BUTTON *userbuttons );
void  ButtonRotate( int setrotate );
boolean ButtonHit( int16_t xx, uint16_t yy, int16_t &bHit );
boolean TS_GetMap( int16_t &xx, int16_t &yy, boolean twoHits );
void ButtonDraw( int16_t idBut );

#endif
