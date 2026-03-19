from flask import Flask, request, jsonify, send_file, render_template_string
import wave
import io
import os
import whisper
from openai import OpenAI
from flask_sock import Sock
import math
import time
from dotenv import load_dotenv

app = Flask(__name__)
sock = Sock(app)

CHANNELS = 1
SAMPLE_WIDTH = 2 # 2 bytes for 16-bit audio
SAMPLE_RATE = 44100
html_path = 'index.html'
raw_file = "output.raw"
wav_file = "recording.wav"
speech_path_file = "tts.wav"
HTML_PAGE = ''
with open(html_path, 'r', encoding='utf-8') as f:
    HTML_PAGE = f.read()

load_dotenv()
oai_key = os.getenv("OAI_API_KEY")
model = whisper.load_model("base")
llm_client = OpenAI(api_key=oai_key)

# Send to Whisper STT
def audio_to_whisper(wav_file):
    print("Sending to Whisper...")
    whisper_result = model.transcribe(wav_file, language="en")
    whisper_text = whisper_result["text"]
    print("Audio converted:")
    print(whisper_text)
    print("model.transcribe dict:")
    print(whisper_result)

    return whisper_text

# Send Whisper Text to LLM
def whisper_to_llm(whisper_text):
    print("Sending to OpenAI LLM for processing...")
    llm_response = llm_client.responses.create(
        model="gpt-4.1-mini", # fast, cheap
        input=whisper_text
    )
    print("LLM response text:")
    llm_response_text = llm_response.output_text
    print(llm_response_text)

    return llm_response_text

# Send LLM text to speech
def llm_to_speech(llm_text):
    print("Sending to OAI TTS...")
    with llm_client.audio.speech.with_streaming_response.create(
        model="gpt-4o-mini-tts",
        voice="fable",
        input=llm_text,
        response_format="wav",
        instructions="""
        Speak in a deep, relaxed baritone male voice with a warm Southern American country accent.
        Slow-paced delivery with a gentle drawl.
        Slightly elongated vowels.
        Soft consonants.
        Friendly, confident tone.
        Natural conversational rhythm like a rural Texas storyteller.
        """
    ) as response: 
        response.stream_to_file(speech_path_file)
    print("TTS finished.")

@app.route("/")
def index():
    return render_template_string(HTML_PAGE)

# Receive audio over Websockets
@sock.route("/ws_audio")
def ws_receive_data(ws):
    audio_buffer = io.BytesIO() # in-memory buffer instead of file for speed
    while True:
        data = ws.receive()

        if data is None:
            break
        if isinstance(data, bytes):
            audio_buffer.write(data)
        elif data == "FINALIZE":
            print("Converting to WAV...")

            pcm_data = audio_buffer.getvalue()

            if not pcm_data:
                print("Error: No audio data received from buffer.")
                break
            try:
                ws.send("WAV CONVERT")
                with wave.open(wav_file, 'wb') as wf:
                    wf.setnchannels(CHANNELS)
                    wf.setsampwidth(SAMPLE_WIDTH)
                    wf.setframerate(SAMPLE_RATE)
                    wf.writeframes(pcm_data)
                print(f"WAV created successfully: {wav_file}")

                # OAI pipeline
                ws.send("STT IN PROCESS")
                input_text = audio_to_whisper(wav_file)
                if not input_text:
                    print("Detected silence or noise. Skipping LLM.")
                    ws.send("ERROR: EMPTY SPEECH")
                    return 
                try:
                    ws.send("LLM PROCESSING")
                    response_text = whisper_to_llm(input_text)
                except Exception as e:
                    print(f"LLM error: {e}")
                    ws.send("ERROR: LLM FAULT")
                ws.send("TTS IN PROCESS")
                llm_to_speech(response_text)

                # Send tts back to esp
                print("Sending TTS file back to ESP in chunks...")
                ws.send("AUDIO SENDING")
                with open(speech_path_file, 'rb') as f:
                    chunk_size = 1024
                    while True:
                        chunk = f.read(chunk_size)
                        if not chunk:
                            break
                        ws.send(chunk)

                time.sleep(0.1)
                ws.send("DONE")
                print("Sent DONE signal to ESP")

                audio_buffer = io.BytesIO()
                print("Buffer cleared. Waiting for next recording...")

            except Exception as e:
                print(f"Error during audio data processing: {e}")
                ws.send("ERROR: RECONNECT")



# Debugging
AMP_TONE_FREQ = 440.0   # 440Hz (A4 note)
AMP_SAMPLE_RATE = 44100 # Standard CD quality
AMPLITUDE = 0.5         # 50% volume
CHUNK_SIZE_BYTES = 1024
PI = math.pi
@sock.route('/wav_audio_send_test')
def wav_audio_send_test(ws):
    print("TEST: Sending audio to ESP in chunks...")
    with open(speech_path_file, 'rb') as f:
        chunk_size = 1024
        while True:
            chunk = f.read(chunk_size)
            if not chunk:
                break
            ws.send(chunk)
    # phase = 0.0
    # total_chunks = 200
    
    # try:
    #     for _ in range(total_chunks):
    #         num_samples = CHUNK_SIZE_BYTES // 2
    #         samples = []

    #         for _ in range(num_samples):
    #             value = math.sin(phase) * AMPLITUDE
    #             sample = int(value * 32767)
    #             samples.append(sample)

    #             phase += 2.0 * math.pi * AMP_TONE_FREQ / AMP_SAMPLE_RATE
    #             if phase >= 2.0 * math.pi:
    #                 phase -= 2.0 * math.pi

    #         # Pack all 512 samples into one 1024-byte binary blob
    #         # 'h' is signed short (16-bit), '<' is little-endian
    #         binary_data = struct.pack(f'<{len(samples)}h', *samples)
            
    #         # Send as BINARY (Flask-sock sends bytes as binary automatically)
    #         ws.send(binary_data)
            
    #         # Small throttle to prevent network congestion
    #         # 512 samples at 44.1kHz is ~11ms of audio. 
    #         time.sleep(0.01)
    #     print("Test stream finished.")
    
    # except Exception as e:
    #     print(f"Connection closed or error: {e}")


@app.route('/test', methods=['POST'])
def upload_test():
    if 'file' not in request.files:
        return "No file uploaded", 400
    
    file = request.files['file']

    # Store in memory
    # file_bytes = file.read()
    # buffer = io.BytesIO(file_bytes)
    # buffer.seek(0)

    temp_path = "temp_upload.mp3"
    file.save(temp_path)

    print("Sending to Whisper...")
    whisper_result = model.transcribe(temp_path, language="en")
    whisper_text = whisper_result["text"]
    print("Audio converted:")
    print(whisper_text)
    print("model.transcribe dict:")
    print(whisper_result)

    os.remove(temp_path)

    # HERE - testing integration with OpenAI LLM
    print("Sending to OpenAI LLM for processing...")
    llm_response = llm_client.responses.create(
        model="gpt-4.1-mini", # fast, cheap
        input=whisper_text
    )
    print("LLM response text:")
    llm_response_text = llm_response.output_text
    print(llm_response_text)
    

    # return send_file(buffer, mimetype="audio/wav", as_attachment=False)
    return jsonify({
        "text": whisper_result["text"],
        "segments": whisper_result["segments"],
        "language": whisper_result["language"]
    })

@app.route("/where")
def where():
    return {
        "cwd": os.getcwd(),
        "files_here": os.listdir(os.getcwd())
    }

@app.route("/api/data", methods=['POST'])
def receive_data():
    if request.method == 'POST':
        data = request.json
        print("Received data from ESP: ", data)
        return jsonify({"status": "success", "message": "Data received"}), 200

@app.route("/api/status", methods=['GET'])
def get_status():
    return jsonify({"status": "ready", "timestamp": "..."}), 200

# Previous HTTP audio code
@app.route('/upload_pcm', methods=['POST'])
def upload_pcm():
    pcm_data = request.data
    with open(raw_file, 'ab') as f:
        f.write(pcm_data)

    return "Chunk appended.", 200

@app.route('/finalize', methods=['POST', 'GET'])
# Convert raw audio to WAV
# Upload audio to Whisper STT
def finalize_recording():
    if request.method == 'POST':
        print("Converting raw audio to WAV")
        if not os.path.exists(raw_file):
            return jsonify({"error": "No raw file found"}), 400
        
        with open(raw_file, "rb") as f:
            pcm_data = f.read()

        with wave.open(wav_file, "wb") as wf:
            wf.setnchannels(CHANNELS)
            wf.setsampwidth(SAMPLE_WIDTH)
            wf.setframerate(SAMPLE_RATE)
            wf.writeframes(pcm_data)

        os.remove(raw_file)
        if os.path.exists(wav_file):
            print("WAV created.")
        else:
            print("Error: WAV file not created")
            return jsonify({"error": "WAV file not created"}), 404

        # Send to Whisper STT
        print("Sending to Whisper...")
        whisper_result = model.transcribe(wav_file, language="en")
        whisper_text = whisper_result["text"]
        print("Audio converted:")
        print(whisper_text)
        print("model.transcribe dict:")
        print(whisper_result)

        return jsonify({
            "status": "Success: created WAV file, Whisper STT",
            "whisper_text": whisper_text
            }), 200
    
    else: # GET request from frontend
        if not os.path.exists(wav_file):
            return jsonify({"error": "No WAV file found"}), 404

        return send_file(wav_file, mimetype="audio/wav", as_attachment=False)
    



if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000) # accessible externally