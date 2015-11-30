
// -----------------------------------------
// --- USER Button Data Starts here
//        { text *, tx, ty, ww, hh, fgc, bgc, btype, info, bState, fontsz, bId, TS_data }    << Struct looks like this
// REQUIRED LAST ITEM :: {(char *)".", -1, 0, 0, 0, 0, 0, TS_FBUTN, 0, 0, 0, 0, 0}
// -----------------------------------------

// -----------------------------------------
// --- USER Button Data Starts here
// -----------------------------------------
//        { text *, tx, ty, ww, hh, fgc, bgc, btype, info, bState, fontsz, bId, TS_data }    << Struct looks like this
TS_BUTTON mybuttons1[] = {
  {(char *)"Redraw", 10, 25, 100, 50, ILI9341_YELLOW, ILI9341_PINK, TS_RBUTN, 0, 0, 2, 1, 0 },
  {(char *)"???", 10, 80, 100, 50, ILI9341_BLACK, ILI9341_BLACK, TS_FRAME, 1, 0, 2, 101, 1 },
  {(char *)"ROT", 10, 80, 50, 50, ILI9341_RED, ILI9341_BLUE, TS_TOGON, 1, 0, 2, 11, 0},
  {(char *)"SIT", 60, 80, 50, 50, ILI9341_GREEN, ILI9341_BLUE, TS_TOGOFF , 1, 0, 2, 51, 0 },
  {(char *)"???", 10, 135, 100, 50, ILI9341_RED, ILI9341_BLACK, TS_FRAME, 2, 0, 2, 102, 1 },
  {(char *)"Sl.A", 60, 135, 50, 50, ILI9341_ORANGE, ILI9341_BLACK, TS_ASLIDE, 2, 0, 2, 2, 0 },
  {(char *)"Sl.B", 10, 135, 50, 50, ILI9341_CYAN, ILI9341_BLACK, TS_BSLIDE, 2, 0, 2, 52, 0 },
  {(char *)"???", 115, 25, 50, 105, ILI9341_YELLOW, ILI9341_BLACK, TS_FRAME, 3, 0, 2, 103, 1 },
  {(char *)"Clr", 115, 25, 50, 55, ILI9341_CYAN, ILI9341_BLACK, TS_TOGOFF, 3, 0, 2, 3, 0 },
  {(char *)"Hi!", 115, 80, 50, 50, ILI9341_CYAN, ILI9341_NAVY, TS_TOGON, 3, 0, 2, 53, 0 },
  {(char *)"Dbo", 165, 25, 50, 105, ILI9341_CYAN, ILI9341_PINK, TS_FBUTN, 0, 0, 2, 201, 0 },
  {(char *)"BOUNCE", 115, 135, 100, 50, ILI9341_BLACK, ILI9341_DARKCYAN, TS_FBUTN, 0, 0, 2, 202, 0 },
  {(char *)".", -1, 0, 0, 0, 0, 0, TS_FBUTN, 0, 0, 0, 0, 0}
};

// -----------------------------------------
// --- USER Button Data Ends here
// -----------------------------------------

