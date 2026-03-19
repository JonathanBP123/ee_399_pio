#pragma once
#include <driver/i2s.h>
#include <Arduino.h>
#include <math.h>

#define AMP_SD_I2S  GPIO_NUM_27
#define AMP_CLK_I2S GPIO_NUM_14
#define AMP_LR_I2S  GPIO_NUM_12
#define AMP_SHTDN   GPIO_NUM_26
// gain at 9 dB - GAIN floating

#define AMP_SAMPLE_RATE 24000
#define AMP_TONE_FREQ 400.0f
#define AMPLITUDE 1.0f

extern i2s_config_t i2s_config_amp;
extern i2s_pin_config_t i2s_amp_pins;

// Install driver, set pins, zero DMA buffer
void amp_setup();


// Play single tone to test amp/speaker
void amp_tone_test();

// Test to more closely mimic human speech
void amp_speech_mimic_test();