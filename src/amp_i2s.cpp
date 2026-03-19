#include "amp_i2s.h"

i2s_config_t i2s_config_amp = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = AMP_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 32,
        .dma_buf_len = 512,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };


i2s_pin_config_t i2s_amp_pins = {
    .bck_io_num = AMP_CLK_I2S,
    .ws_io_num = AMP_LR_I2S,
    .data_out_num = AMP_SD_I2S,
    .data_in_num = I2S_PIN_NO_CHANGE
};

void amp_setup() {
    Serial.println("Setting up amp...");
    i2s_driver_install(I2S_NUM_0, &i2s_config_amp, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &i2s_amp_pins);
    i2s_zero_dma_buffer(I2S_NUM_0);
    pinMode(AMP_SHTDN, INPUT_PULLUP); // shutdown/mode pin pulled high (left channel output)
}

void amp_tone_test() {
    static float phase = 0.0f;
    static int16_t samples[256];


    for (int i = 0; i < 256; i++) {
        float value = sinf(phase) * AMPLITUDE;
        samples[i] = (int16_t)(value * 32767);


        phase += 2.0f * PI * AMP_TONE_FREQ / AMP_SAMPLE_RATE;
        if (phase >= 2.0f * PI) phase -= 2.0f * PI;
    }


    size_t bytes_written;
    i2s_write(
        I2S_NUM_0,
        samples,
        sizeof(samples),
        &bytes_written,
        portMAX_DELAY
    );
}

#include "math.h"

// Constants for the "vowel" effect
#define VOWEL_F1 700.0f  // First Formant (approx. "Ah")
#define VOWEL_F2 1200.0f // Second Formant
#define RESONANCE 0.95f  // How "sharp" the vowel sounds

void amp_speech_mimic_test() {
    static float phase = 0.0f;
    static float y1_1 = 0, y1_2 = 0; // Filter states for Formant 1
    static float y2_1 = 0, y2_2 = 0; // Filter states for Formant 2
    static int16_t samples[256];

    // Calculate filter coefficients (simple 2nd order resonator)
    float omega1 = 2.0f * PI * VOWEL_F1 / AMP_SAMPLE_RATE;
    float cos_w1 = cosf(omega1);
    float a1 = 2.0f * RESONANCE * cos_w1;
    float a2 = -RESONANCE * RESONANCE;

    for (int i = 0; i < 256; i++) {
        // 1. Source: A Sawtooth wave is "buzzier" and richer in harmonics than a sine
        float saw = (phase / PI) - 1.0f; 
        
        // 2. Filter: Pass the saw through a resonator to create a "vowel"
        // Formant 1
        float out1 = saw + a1 * y1_1 + a2 * y1_2;
        y1_2 = y1_1;
        y1_1 = out1;

        // 3. Mixing and Soft Clipping (Speech isn't perfectly linear)
        float combined = out1 * 0.5f; // Keep it from clipping harshly
        samples[i] = (int16_t)(combined * AMPLITUDE * 32767);

        // Update phase for the fundamental pitch (Glottal frequency)
        phase += 2.0f * PI * AMP_TONE_FREQ / AMP_SAMPLE_RATE;
        if (phase >= 2.0f * PI) phase -= 2.0f * PI;
        
        // Very basic DC offset removal
        y1_1 *= 0.99f; 
    }

    size_t bytes_written;
    i2s_write(I2S_NUM_0, samples, sizeof(samples), &bytes_written, portMAX_DELAY);
}