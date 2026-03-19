#include <Arduino.h>
#include <WiFiManager.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "mic_i2s.h"
#include "amp_i2s.h"
#include "edge_impulse.h"
#include "wifi_setup.h"
#include "debugging.h"
#include "lcd.h"

// Server parameters
const int server_port = 5000;

// Device-Northwestern
String wifi_ssid = "Device-Northwestern";
const char* server_ip = "10.105.34.52"; // computer's IPv4 address

// Guest-Northwestern
// const char* server_ip = "10.104.225.210"; // Guest-Northwestern

String websockets_server_address_test = "ws://" + String(server_ip) + ":" + String(server_port) + "/ws_audio";
// String websockets_server_address_test = "ws://" + String(server_ip) + ":" + String(server_port) + "/wav_audio_send_test";


void stt_pipeline() {
    Serial.println("Wakeword detected!");

    pause_ei_capture();
    delay(500);

    digitalWrite(LED, HIGH);

    // Websocket send audio
    mic_samples_websock();

    // Send to speaker
    play_websocket_audio();

    // Clear old driver state & reinit mic
    i2s_driver_uninstall(I2S_NUM_1);
    mic_setup();

    resume_ei_capture();
    delay(1000);
}

void setup() {
    Serial.begin(921600); 

    // i2s mic
    ei_setup();

    // i2s amp/speaker
    amp_setup();    

    // lcd
    // lcd_setup();
    // tft_record_btn();

    wifi_setup(wifi_ssid); // for Device-Northwestern
    // wifi_setup(wifi_ssid, wifi_pwd); // for password-required networks
    websock_setup(websockets_server_address_test);

}

volatile bool detected = false;
void loop() {    
    float percent_pass = 0.85;
    detected = ei_result(percent_pass);
    if (detected) {
        stt_pipeline();
    }
    else {
        digitalWrite(LED, LOW);
    }
    

    // Screen button test
    if (touch_detected) {
        touch_detected = false;
        int tx = ft6336u.read_touch1_x();
        int ty = ft6336u.read_touch1_y();

        int actual_tx = constrain(ty, 0, 479);
        int actual_ty = constrain(320 - tx, 0, 319);

        if (ellipse_touched(actual_tx, actual_ty, 100, 100, 50)) {
            Serial.println("Button touched!");
            stt_pipeline();
        }
        touch_detected = false;
    }
    else {
        digitalWrite(LED, LOW);
    }


    // Testing speaker independent of pipeline
    // play_websocket_audio();

    // mic i2s tests
    // mic_serial_test();
    // mic_samples_uart();

    // speaker/amp i2s
    // amp_tone_test();
    // amp_speech_mimic_test();

    // lcd
    // tft_espi_loop_test();
    // tft_touch_test();
  }



// Old code
// HTTP method
/*
if (test_flag) {
    mic_samples_server(
    "http://" + String(server_ip) + ":" + String(server_port) + "/upload_pcm",
    "http://" + String(server_ip) + ":" + String(server_port) + "/finalize"
    );
    test_flag = false;
    delay(15000);
    }
*/




