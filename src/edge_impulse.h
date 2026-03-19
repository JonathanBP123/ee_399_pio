#pragma once 
#define LED 2

// Setup edge impulse for mic wakeword detection
// Ensure Serial.begin() called before
// Includes mic I2S setup and LED pin init
void ei_setup();

// Put in loop to run inference
// Returns true if wakeword detected (`percent_pass` exceeded)
bool ei_result(float percent_pass);

// Pause EI capture
void pause_ei_capture();

// Resume inference
void resume_ei_capture();