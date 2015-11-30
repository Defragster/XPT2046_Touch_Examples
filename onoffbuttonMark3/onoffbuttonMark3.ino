// XPT2046_Touchscreen ; Button Functions
//
// TAG wide verus narrow button  1%3 or 2,4 allow moved placement on rotation
// Button Animation :: ButtonDraw( bValue, [special color, or animate type???]  );
#include <SPI.h>
#include <Wire.h>



#include <ILI9341_t3.h>
#include <XPT2046_Touchscreen.h>
#include "Buttonmap.h"
#include "onoffbuttonMark3.h"


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
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);



void setup(void)
{
  Serial.begin(9600);
  while ( !Serial && (millis() < 2000)) ;
  tft.begin();
  if (!ts.begin()) Serial.println("Unable to start touchscreen.");
  else Serial.println("Touchscreen started.");
  ButtonInit( &mybuttons1[0] );
  tft.fillScreen(ILI9341_BLUE); // clear screen
  ButtonDraw( 0 );
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
      case 11:
        yout = 220, xout = 10;
        cii = 1;
        Serial.print("\nGreen btn hit : Rotate =");
        tft.setCursor(xout, yout);
        tft.setTextColor(ILI9341_BLACK);
        tft.setTextSize(3);
        tft.print("... WAIT ...");
        delay(300);
        ButtonRotate( 1 + TS_Rotate );
        tft.fillScreen(ILI9341_BLUE); // clear screen
        ButtonDraw( 0 );
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
        for (int ii = 0; ii < 7; ii++) cc[ii] = 0; // CLear :: cc[7] = {0, 0, 0, 0, 0, 0, 0};
        ButtonDraw( 0 );
        yout = 190, xout = 115;
        break;
      case 53:
        cii = 3;
        Serial.print("\nNavy btn hit");
        yout = 190, xout = 115;
        break;
      case 10:
        cii = 4;
        if ( lastbValue == bValue )
          Serial.print("1."); // DOES AUTO REPEAT
        else
          Serial.print("\nbutton 1_");
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
      if ( cc[cii] > 13 ) {
        cc[cii] = 1;
        Serial.println();
        tft.fillRect(xout, yout, 100, 10, ILI9341_BLACK);
      }
      tft.setCursor(xout, yout);
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(1);
      tft.print(bValue); // Shows BUTTON ID received on screen
      for ( int ii = 1; ii < cc[cii]; ii++) tft.print("|"); // Added char for each unbounced button touch
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

