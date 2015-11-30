// XPT2046_Touchscreen : ButtonMap :: Button Functions
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

#include "Arduino.h"
#include "ButtonMap.h"
#include <ILI9341_t3.h>
#include <XPT2046_Touchscreen.h>

extern ILI9341_t3 tft;
extern XPT2046_Touchscreen ts;

// --- Button Code Starts here
TS_BUTTON TS_NoButton[] = { {(char *)"X", -1, 0, 0, 0, 0, 0, TS_FBUTN, 0, 0, 0} };
TS_BUTTON *TS_pbtn = TS_NoButton;

// Zero is calibration for mapping to pixel, 1-4 follow screen rotation for mapping math
TS_MAP tsMap[5] = { {200, 3700, 325, 3670 }, { 0, 319, 0, 239 }, { 319, 0, 0, 239 }, { 319, 0, 239, 0 }, { 0, 319, 239, 0 } };
int16_t TS_xMax = 0; // Current Screen orientation x,y Max values
int16_t TS_yMax = 0;

int TS_bCount = 0;  // Set at run time from sizeof()
int16_t TS_Rotate = 1;
static int16_t TS_iiSlide = -1;

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

// Walk button struct
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

// Set TFT rotation and update the mapping that normalizes touch points there
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
}

// Call with Button Map for display and hit testing against
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
  ButtonDraw( 0 );
}
// -----------------------------------------
// --- Button Code Ends here
// -----------------------------------------
