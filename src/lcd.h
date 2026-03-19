#pragma once

#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include "examples\480 x 320\Free_Font_Demo\Free_Fonts.h" // Include the header file attached to this sketch
// #include "FS.h" - TODO: might not need

// From Arduino-FT6336U GitHub (https://github.com/aselectroworks/Arduino-FT6336U/blob/master/examples/ReadTouchParam/ReadTouchParam.ino)
#include "FT6336U.h"

#define I2C_SDA 13
#define I2C_SCL 17
#define RST_N_PIN 4 // IMPORTANT - set to unused but valid pin num
#define INT_N_PIN 15
extern FT6336U ft6336u;
extern volatile bool touch_detected;


#ifndef LOAD_GLCD
ERROR_Please_enable_LOAD_GLCD_in_User_Setup
#endif

#ifndef LOAD_FONT2
ERROR_Please_enable_LOAD_FONT2_in_User_Setup!
#endif

#ifndef LOAD_FONT4
ERROR_Please_enable_LOAD_FONT4_in_User_Setup!
#endif

#ifndef LOAD_FONT6
ERROR_Please_enable_LOAD_FONT6_in_User_Setup!
#endif

#ifndef LOAD_FONT7
ERROR_Please_enable_LOAD_FONT7_in_User_Setup!
#endif

#ifndef LOAD_FONT8
//ERROR_Please_enable_LOAD_FONT8_in_User_Setup!
#endif

#ifndef LOAD_GFXFF
ERROR_Please_enable_LOAD_GFXFF_in_User_Setup!
#endif



void lcd_header(const char *string);
void lcd_drawDatum(int x, int y);

// Init touch screen (TFT eSPI & touch controller)
void lcd_setup();

// TFT eSPI example
void tft_espi_loop_test();

// TFT touch test
void tft_touch_test();

// Display LLM response text
void tft_display_response(String response_text);

// Button touched helper
bool ellipse_touched(int touch_x, int touch_y, int el_center_x, int el_center_y, int el_radius);

// Display button on TFT for start/stop recording
void tft_record_btn();