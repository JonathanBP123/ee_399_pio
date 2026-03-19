#pragma once
#include <driver/i2s.h>
#include <Arduino.h>
#include <WiFi.h>
#include <HttpClient.h>
#include <ArduinoWebsockets.h>

#define MIC_LR_I2S  GPIO_NUM_32
#define MIC_WS_I2S  GPIO_NUM_33
#define MIC_CLK_I2S GPIO_NUM_25
#define MIC_SD_I2S  GPIO_NUM_36 // SENSOR_VP, input only

#define SAMPLE_BUFFER_SIZE  128
#define SAMPLE_RATE         44100 // bits per sec
#define DMA_BUFFER_COUNT    3
#define DMA_BUFFER_LENGTH   1024
#define RECORD_TIME         5 // sec of audio after wakeword trigger


// Pull LR pin low for left channel
// Install driver, set pins
// Returns true if successful
bool mic_setup();

// Stream I2S mic data to terminal over serial
void mic_serial_test();

// Stream I2S mic bitstream over UART
void mic_samples_uart();

// Send I2S mic bitstream to Flask server via HTTP Stream
void mic_samples_server(String upload_url, String finalize_url);

// Set up Websockets callbacks, `websockets_server is url:port`
void websock_setup(String websockets_server);

// Send I2S mic bitstream to Flask server via Websockets
void mic_samples_websock();

// Play audio response from server
void play_websocket_audio();