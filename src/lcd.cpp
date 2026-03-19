#include "lcd.h"

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
FT6336U ft6336u(I2C_SDA, I2C_SCL, RST_N_PIN, INT_N_PIN);
volatile bool touch_detected = false;

// Touch ISR
void IRAM_ATTR handle_touch() {
    touch_detected = true;
}


// Print the header for a display screen
void lcd_header(const char *string)
{
  tft.setTextSize(1);
  tft.setTextColor(TFT_MAGENTA, TFT_BLUE);
  tft.fillRect(0, 0, 480, 30, TFT_BLUE);
  tft.setTextDatum(TC_DATUM);
  tft.drawString(string, 239, 2, 4); // Font 4 for fast drawing with background
}

// Draw a + mark centred on x,y
void lcd_drawDatum(int x, int y)
{
  tft.drawLine(x - 5, y, x + 5, y, TFT_GREEN);
  tft.drawLine(x, y - 5, x, y + 5, TFT_GREEN);
}

void lcd_setup() {
    Serial.println("Setting up LCD...");
    tft.init();

    // Set the rotation before we calibrate
    tft.setRotation(1); // NOTE: 1 is normally oriented 

    // Clear the screen
    tft.fillScreen(TFT_BLACK);

    // Init touch 
    Serial.println("Setting up touch controller...");
    ft6336u.begin(); 
    Serial.print("FT6336U Firmware Version: ");
    Serial.println(ft6336u.read_firmware_id());
    Serial.print("FT6336U Device Mode: ");
    Serial.println(ft6336u.read_device_mode());

    // Touch interrupt
    attachInterrupt(digitalPinToInterrupt(INT_N_PIN), handle_touch, FALLING);
}


// TFT eSPI example
void tft_espi_loop_test() {
    int xpos =  0;
    int ypos = 40;

    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // Select different fonts to draw on screen using the print class
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    tft.fillScreen(TFT_NAVY); // Clear screen to navy background

    lcd_header("Draw free fonts using print class");

    // For comaptibility with Adafruit_GFX library the text background is not plotted when using the print class
    // even if we specify it.
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setCursor(xpos, ypos);    // Set cursor near top left corner of screen

    tft.setTextFont(GLCD);     // Select the orginal small GLCD font by using NULL or GLCD
    tft.println();             // Move cursor down a line
    tft.print("Original GLCD font");    // Print the font name onto the TFT screen
    tft.println();
    tft.println();

    tft.setFreeFont(FSB9);   // Select Free Serif 9 point font, could use:
    // tft.setFreeFont(&FreeSerif9pt7b);
    tft.println();          // Free fonts plot with the baseline (imaginary line the letter A would sit on)
    // as the datum, so we must move the cursor down a line from the 0,0 position
    tft.print("Serif Bold 9pt");  // Print the font name onto the TFT screen

    tft.setFreeFont(FSB12);       // Select Free Serif 12 point font
    tft.println();                // Move cursor down a line
    tft.print("Serif Bold 12pt"); // Print the font name onto the TFT screen

    tft.setFreeFont(FSB18);       // Select Free Serif 12 point font
    tft.println();                // Move cursor down a line
    tft.print("Serif Bold 18pt"); // Print the font name onto the TFT screen

    tft.setFreeFont(FSB24);       // Select Free Serif 24 point font
    tft.println();                // Move cursor down a line
    tft.print("Serif Bold 24pt"); // Print the font name onto the TFT screen


    delay(4000);

    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // Now use drawString() so we can set background colours and the datum
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    tft.fillScreen(TFT_BLACK);

    lcd_header("Draw with background using drawString()");

    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.setTextDatum(TC_DATUM); // Centre text on x,y position

    xpos = tft.width() / 2; // Half the screen width
    ypos = 50;

    tft.setFreeFont(FSB9);                              // Select the font
    tft.drawString("Serif Bold 9pt", xpos, ypos, GFXFF);  // Draw the text string in the selected GFX free font
    ypos += tft.fontHeight(GFXFF);                      // Get the font height and move ypos down

    tft.setFreeFont(FSB12);
    tft.drawString("Serif Bold 12pt", xpos, ypos, GFXFF);
    ypos += tft.fontHeight(GFXFF);

    tft.setFreeFont(FSB18);
    tft.drawString("Serif Bold 18pt", xpos, ypos, GFXFF);
    ypos += tft.fontHeight(GFXFF);

    tft.setFreeFont(FSB24);
    tft.drawString("Serif Bold 24pt", xpos, ypos, GFXFF);
    ypos += tft.fontHeight(GFXFF);

    // Set text padding to 120 pixels wide area to over-write old values on screen
    tft.setTextPadding(120);
    for (int i = 0; i <= 20; i++) {
        tft.drawFloat(i / 10.0, 1, xpos, ypos, GFXFF);
        delay(200);
    }

    delay(4000);

    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // Same again but with colours that show bounding boxes
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


    tft.fillScreen(TFT_DARKGREY);

    lcd_header("Show background filled bounding boxes");

    tft.setTextColor(TFT_YELLOW, TFT_BLACK);

    tft.setTextDatum(TC_DATUM); // Centre text on x,y position

    xpos = tft.width() / 2; // Half the screen width
    ypos = 50;

    tft.setFreeFont(FSB9);                              // Select the font
    tft.drawString("Serif Bold 9pt", xpos, ypos, GFXFF);  // Draw the text string in the selected GFX free font
    ypos += tft.fontHeight(GFXFF);                        // Get the font height and move ypos down

    tft.setFreeFont(FSB12);
    tft.drawString("Serif Bold 12pt", xpos, ypos, GFXFF);
    ypos += tft.fontHeight(GFXFF);

    tft.setFreeFont(FSB18);
    tft.drawString("Serif Bold 18pt", xpos, ypos, GFXFF);
    ypos += tft.fontHeight(GFXFF);

    tft.setFreeFont(FSBI24);
    tft.drawString("Serif Bold Italic 24pt", xpos, ypos, GFXFF);
    ypos += tft.fontHeight(GFXFF);

    // Set text padding to 120 pixels wide area to over-write old values on screen
    tft.setTextPadding(120);
    for (int i = 0; i <= 20; i++) {
        tft.drawFloat(i / 10.0, 1, xpos, ypos, GFXFF);
        delay(200);
    }

    delay(4000);

    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // Now show setting the 12 datum positions works with free fonts
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    // Numbers, floats and strings can be drawn relative to a datum
    tft.fillScreen(TFT_BLACK);
    lcd_header("Draw text relative to a datum");

    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setFreeFont(FSS9);

    tft.setTextDatum(TL_DATUM);
    tft.drawString("[Top left}", 20, 60, GFXFF);
    lcd_drawDatum(20,60);

    tft.setTextDatum(TC_DATUM);
    tft.drawString("[Top centre]", 240, 60, GFXFF);
    lcd_drawDatum(240,60);

    tft.setTextDatum(TR_DATUM);
    tft.drawString("[Top right]", 460, 60, GFXFF);
    lcd_drawDatum(460,60);

    tft.setTextDatum(ML_DATUM);
    tft.drawString("[Middle left]", 20, 140, GFXFF);
    lcd_drawDatum(20,140);

    tft.setTextDatum(MC_DATUM);
    tft.drawString("[Middle centre]", 240, 140, GFXFF);
    lcd_drawDatum(240,140);

    tft.setTextDatum(MR_DATUM);
    tft.drawString("[Middle right]", 460, 140, GFXFF);
    lcd_drawDatum(460,140);

    tft.setTextDatum(BL_DATUM);
    tft.drawString("[Bottom Left]", 20, 220, GFXFF);
    lcd_drawDatum(20,220);

    tft.setTextDatum(BC_DATUM);
    tft.drawString("[Bottom centre]", 240, 220, GFXFF);
    lcd_drawDatum(240,220);

    tft.setTextDatum(BR_DATUM);
    tft.drawString("[Bottom right]", 460, 220, GFXFF);
    lcd_drawDatum(460,220);

    tft.setTextDatum(L_BASELINE);
    tft.drawString("[Left baseline]", 20, 300, GFXFF);
    lcd_drawDatum(20,300);

    tft.setTextDatum(C_BASELINE);
    tft.drawString("[Centre baseline]", 240, 300, GFXFF);
    lcd_drawDatum(240,300);

    tft.setTextDatum(R_BASELINE);
    tft.drawString("[Right baseline]", 460, 300, GFXFF);
    lcd_drawDatum(460,300);

    //while(1);
    delay(8000);
}

// TFT touch test
void tft_touch_test() {
    if(digitalRead(INT_N_PIN) != -1) {
        Serial.print("FT6336U TD Status: ");
        Serial.println(ft6336u.read_td_status());
        Serial.print("FT6336U Touch Event/ID 1: (");
        Serial.print(ft6336u.read_touch1_event()); Serial.print(" / "); Serial.print(ft6336u.read_touch1_id()); Serial.println(")");
        Serial.print("FT6336U Touch Position 1: (");
        Serial.print(ft6336u.read_touch1_x()); Serial.print(" , "); Serial.print(ft6336u.read_touch1_y()); Serial.println(")");
        Serial.print("FT6336U Touch Weight/MISC 1: (");
        Serial.print(ft6336u.read_touch1_weight()); Serial.print(" / "); Serial.print(ft6336u.read_touch1_misc()); Serial.println(")");
        Serial.print("FT6336U Touch Event/ID 2: (");
        Serial.print(ft6336u.read_touch2_event()); Serial.print(" / "); Serial.print(ft6336u.read_touch2_id()); Serial.println(")");
        Serial.print("FT6336U Touch Position 2: (");
        Serial.print(ft6336u.read_touch2_x()); Serial.print(" , "); Serial.print(ft6336u.read_touch2_y()); Serial.println(")");
        Serial.print("FT6336U Touch Weight/MISC 2: (");
        Serial.print(ft6336u.read_touch2_weight()); Serial.print(" / "); Serial.print(ft6336u.read_touch2_misc()); Serial.println(")");
        lcd_drawDatum(ft6336u.read_touch1_x(), ft6336u.read_touch1_y());
        delay(500);
        tft.fillScreen(TFT_BLACK);
    }
}

// Display LLM response text
void tft_display_response(String response_text) {
    tft.fillScreen(TFT_NAVY); // Clear screen to navy background
    lcd_header(response_text.c_str());
}

// Button touched helper
bool ellipse_touched(int touch_x, int touch_y, int el_center_x, int el_center_y, int el_radius) {
    long dx = touch_x - el_center_x;
    long dy = touch_y - el_center_y;
    return (dx*dx + dy*dy) <= (el_radius*el_radius);
}

// Display button on TFT for start/stop recording
void tft_record_btn() {
    tft.fillScreen(TFT_BLACK);

    tft.fillEllipse(100, 100, 50, 50, TFT_YELLOW);
}

