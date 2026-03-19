#include "mic_i2s.h"
#include <fstream>

/* Using Ilya's config (EE 202) */
i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = DMA_BUFFER_COUNT,
    .dma_buf_len = DMA_BUFFER_LENGTH,
    .use_apll = false
};


i2s_pin_config_t i2s_mic_pins = {
    .bck_io_num = MIC_CLK_I2S,
    .ws_io_num = MIC_WS_I2S,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = MIC_SD_I2S
};

const uint16_t i2s_buf_len = 2 * DMA_BUFFER_LENGTH;
const i2s_bits_per_sample_t i2s_bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
const uint8_t i2s_bytes_per_sample = i2s_bits_per_sample / 8;
const int i2s_queue_len = 16;
QueueHandle_t i2s_queue = nullptr;
int16_t mic_read_buffer[i2s_buf_len] = {0};
const uint16_t i2s_read_size_bytes = i2s_buf_len * i2s_bytes_per_sample;

bool mic_setup() {
    pinMode(MIC_LR_I2S, OUTPUT); // pull LR low for left channel, high for right
    digitalWrite(MIC_LR_I2S, HIGH);

    esp_err_t i2s_err_install = i2s_driver_install(I2S_NUM_1, &i2s_config, i2s_queue_len, &i2s_queue);
    esp_err_t i2s_err_pins = i2s_set_pin(I2S_NUM_1, &i2s_mic_pins);

    return (i2s_err_install == ESP_OK) && (i2s_err_pins == ESP_OK);
}

void mic_serial_test() {
    int32_t test_raw_samples[SAMPLE_BUFFER_SIZE];
    size_t bytes_read = 0;
    i2s_read(I2S_NUM_1, test_raw_samples, sizeof(int32_t) * SAMPLE_BUFFER_SIZE, &bytes_read, portMAX_DELAY);
    int samples_read = bytes_read / sizeof(int32_t);

    // dump to serial channel
    for (int i = 0; i < samples_read; i++) {
        Serial.printf("%ld\n", test_raw_samples[i]); 
        // delay(250);
    }
}

void mic_samples_uart() {
    size_t bytes_read = 0;

    i2s_read( // 24 bit words, MSB first
        I2S_NUM_1,
        mic_read_buffer,
        i2s_read_size_bytes,
        &bytes_read,
        portMAX_DELAY
    );

    
    Serial.write(
        (uint8_t*)mic_read_buffer,
        bytes_read
    );
}

void mic_samples_server(String upload_url, String finalize_url) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected.");
        return;
    }

    const size_t total_bytes =
        SAMPLE_RATE *
        (I2S_BITS_PER_SAMPLE_16BIT / 8) *
        RECORD_TIME;

    HTTPClient http;

    Serial.println("Recording and streaming...");
    size_t total_sent = 0;
    while (total_sent < total_bytes) {
        size_t bytes_read = 0;
        esp_err_t result = i2s_read(
            I2S_NUM_1,
            mic_read_buffer,
            i2s_read_size_bytes,
            &bytes_read,
            portMAX_DELAY
        );

        if (result != ESP_OK || bytes_read == 0) {
            Serial.println("I2S read error");
            break;
        }

        http.begin(upload_url);
        http.addHeader("Content-Type", "application/octet-stream");
        int http_response_code = http.POST((uint8_t*)mic_read_buffer, bytes_read); // websocket, send BIN
        Serial.print("Raw HTTP response code: ");
        Serial.println(http_response_code);
        http.end();

        total_sent += bytes_read;
    }
    Serial.println("Done sending raw data. Converting to WAV...");

    http.begin(finalize_url);
    int finalize_http_response_code = http.POST("");
    Serial.print("WAV HTTP response code: ");
    Serial.println(finalize_http_response_code);
    http.end();
}

volatile bool is_audio_done = false;
using namespace websockets;
void onMessageCallback(WebsocketsMessage message) {
    if (message.isBinary()) {
        is_audio_done = false;
        Serial.println("Receiving binary audio data from server...");

        const char* data = message.rawData().c_str();
        size_t len = message.length();

        // WAV header
        size_t offset = 44;
        if (len <= offset) return;

        // Write to amp
        size_t bytes_written;
        esp_err_t result = i2s_write(
            I2S_NUM_0,
            data,
            len,
            &bytes_written,
            portMAX_DELAY
        );

        if (result == ESP_OK) {
            Serial.printf("Played %d bytes to speaker.\n", bytes_written);
        }
    }
    else if (message.isText()) {
        if (message.data() == "WAV CONVERT") {
            Serial.println("Server converting audio to WAV...");
        }
        else if (message.data() == "STT IN PROCESS") {
            Serial.println("Server converting speech to text...");
        }
        else if (message.data() == "LLM PROCESSING") {
            Serial.println("Server processing LLM request...");
        }
        else if (message.data() == "TTS IN PROCESS") {
            Serial.println("Server converting text to speech...");
        }
        else if (message.data() == "AUDIO SENDING") {
            Serial.println("Server about to send audio response...");
        }
        else if (message.data() == "DONE") {
            Serial.println("Server sent DONE signal.");
            is_audio_done = true;
        }
        else if (message.data() == "ERROR: RECONNECT") {
            Serial.println("Error: Connection closed.");
        }
    }
    else {
        Serial.print("Got Message: ");
        Serial.println(message.data());
    }
}

void onEventsCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("Connnection Opened");
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("Connnection Closed");
    } else if(event == WebsocketsEvent::GotPing) {
        Serial.println("Got a Ping!");
    } else if(event == WebsocketsEvent::GotPong) {
        Serial.println("Got a Pong!");
    }
}

WebsocketsClient ws_client;
void websock_setup(String websockets_server) {
    ws_client.onMessage(onMessageCallback);
    ws_client.onEvent(onEventsCallback);

    // Connect to server
    Serial.println("Connecting to server via Websockets...");
    ws_client.connect(websockets_server);

    // Send a ping
    bool ping_success = ws_client.ping(); 
    if (ping_success) {
        Serial.println("Ping success!");
    }
    else {
        Serial.println("Ping failed.");
    }
}

void mic_samples_websock() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected.");
        return;
    }

    const size_t total_bytes =
        SAMPLE_RATE *
        (I2S_BITS_PER_SAMPLE_16BIT / 8) *
        RECORD_TIME;

    Serial.println("Recording and streaming...");
    size_t total_sent = 0;
    while (total_sent < total_bytes) {
        size_t bytes_read = 0;
        esp_err_t result = i2s_read(
            I2S_NUM_1,
            mic_read_buffer,
            i2s_read_size_bytes,
            &bytes_read,
            portMAX_DELAY
        );

        if (result != ESP_OK || bytes_read == 0) {
            Serial.println("I2S read error");
            break;
        }

        // Serial.println("Sending binary mic data...");
        ws_client.sendBinary((const char*)mic_read_buffer, bytes_read);
        ws_client.poll();

        total_sent += bytes_read;
    }
    Serial.println("Done sending raw data. Converting to WAV...");

    ws_client.send("FINALIZE");
}

void play_websocket_audio() {
    Serial.println("Waiting for TTS response...");
    is_audio_done = false;
    unsigned long last_packet_time = millis();

    while (ws_client.available()) {
        ws_client.poll();

        if (is_audio_done) {
            break;
        }
    }

    Serial.println("TTS audio stream finished");
}

