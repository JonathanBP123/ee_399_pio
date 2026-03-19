import serial
import numpy as np
from scipy.io.wavfile import write

SERIAL_PORT = 'COM7'
BAUD_RATE = 921600

SAMPLE_RATE = 44100
DURATION = 15  # seconds

BYTES_PER_SAMPLE = 2           # int16
I2S_CHUNK_SAMPLES = 2048       # matches ESP32
I2S_CHUNK_BYTES = I2S_CHUNK_SAMPLES * BYTES_PER_SAMPLE

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

num_samples = SAMPLE_RATE * DURATION
audio_data = np.zeros(num_samples, dtype=np.int16)

samples_read = 0

print("Reading samples...")

while samples_read < num_samples:
    remaining_samples = num_samples - samples_read
    bytes_to_read = min(remaining_samples * BYTES_PER_SAMPLE, I2S_CHUNK_BYTES)

    raw_bytes = ser.read(bytes_to_read)

    # Ensure we only convert whole samples
    valid_bytes = (len(raw_bytes) // BYTES_PER_SAMPLE) * BYTES_PER_SAMPLE
    if valid_bytes == 0:
        continue

    chunk = np.frombuffer(raw_bytes[:valid_bytes], dtype='<i2')  # little-endian int16

    audio_data[samples_read:samples_read + len(chunk)] = chunk
    samples_read += len(chunk)

    print(".", end="", flush=True)

ser.close()

print("\nSaving WAV...")
write("recorded_audio.wav", SAMPLE_RATE, audio_data)
print("Saved to recorded_audio.wav")
