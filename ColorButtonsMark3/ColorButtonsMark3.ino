#include <SPI.h>
#include <Wire.h>
#include "ButtonMap.h"
#include "ColorButtonsMark3.h"

#include <ILI9341_t3.h>
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
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);

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
  delay(300);
  tft.fillScreen(ILI9341_BLUE);
  BTest( (char *)"mybuttons3", 60, 40, 5, 20 );
  delay(300);
  tft.fillScreen(ILI9341_BLUE);
  BTest( (char *)"mybuttons4", 50, 50, 5, 5 );
  delay(300);
  ButtonInit( &mybuttons4[0] );
  tft.fillScreen(ILI9341_ORANGE);
  ButtonDraw( 0 );
}

void loop()
{
  static elapsedMillis userTime;
  uint16_t userDebounce = 555;
  static int16_t lastbValue;
  boolean istouched = false;
  int16_t bValue;
  int16_t x, y;
  istouched = TS_GetMap( x, y, true );
  if (istouched && ButtonHit(x , y, bValue))
  { // touch data ready?
    if ( lastbValue == bValue ) userDebounce = userTime;
    if (userDebounce > 50) {
      Serial.print("\nbutton #");
      Serial.print(bValue);
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
        Serial.print("\nButtonInit( &mybuttons2[0] );");
        tft.fillScreen(ILI9341_RED);
        ButtonDraw( 0 );
      }
      if ( 3 == bValue ) {
        ButtonInit( &mybuttons3[0] );
        Serial.print("\nButtonInit( &mybuttons3[0] );");
        tft.fillScreen(ILI9341_GREEN);
        ButtonDraw( 0 );
      }
      if ( 4 == bValue ) {
        ButtonInit( &mybuttons4[0] );
        Serial.print("\nButtonInit( &mybuttons4[0] );");
        tft.fillScreen(ILI9341_ORANGE);
        ButtonDraw( 0 );
      }
    }

    lastbValue = bValue;
    userTime = 0;
  }
}


// =====
// --- Test Buttons in a grid using the color array from ILI9341
//    pass: Struct name, button width, height, starting top X, top Y
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
      if ((yy + intHeight > TS_yMax) || (xx + intWidth > TS_xMax)) break;
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
  if ( -1 == tx ) // Watch for Sentinel value print special
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

