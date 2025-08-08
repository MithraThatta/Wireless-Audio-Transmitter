# Wireless-Audio-Transmitter
Takes in a 3.5mm jack as an analog audio input and outputs the recorded audio on a UDP port on a wifi network, using an STMF303RE for digitizing/processing audio data and an ESP32S3 to broadcast mono audio to wifi. Communication between the two MCUs is done via a high speed USART line.

An extensive passive front end analog circuit is used for ensuring received audio is within the 20hz to 20khz audio band(via low pass filter and anti aliasing by high pass filter) and ensuring that the ADC does not disrupt the signal when reading via a unity gain op amp. A second op amp is used as a voltage amplifier, to boost the sound of audio coming from things like songs and speeches(which have sounds coming from various sources at varying intensities), above the noise created by the limited range of the ADC and the thermal/resistor noise from the passive front end circuit.

Audio is playable via VLC or other UDP capable audio players, such as FFmpeg. 
To play audio via VLC,
  1. Press Media, Open network stream. For network URL, enter: udp://@192.168.4.1:8000
  2. Select "show more options" in the bottom left
  3. Choose your cache rate. Leaving it as default is fine, but buffering will be present. Play around with it for best quality
  4. In the box labelled "Edit Options', type : udp://@192.168.4.1:8000 :demux=rawaud :rawaud-samplerate=48000 :rawaud-channels=1 :rawaud-width=16
  5. Press play to hear the audio. Make sure you're connected to the ESP32S3's wifi network


##  Features

- **Analog front end**
  - 3.5 mm female audio jack for audio input from any source
  - AC coupling(10 µF) capacitor to block DC voltage
  - ~1 Hz single‑pole high‑pass (10 µF/100 kΩ) to block stray frequencies, input jack noise, etc
  - 1.65 volt rebiasing via 2 100kΩ resistors to fit audio within range of ADC(0 to 3.3 volts)
  - 19 KHz single‑pole low‑pass (827 Ω/10 nF) to block frequencies above the audio band and nyquist frequency
  - Unity‑gain op-amp buffer to isolate the ADC input and its associated current draw surges
  - Voltage amplifying op-amp to amplify audio(voltage) swings in softer audio, such as songs and speeches to cut through static
  - Link to circuit: https://rb.gy/817adk 
    <img width="908" height="537" alt="image" src="https://github.com/user-attachments/assets/f873230e-8bbc-4cbe-b7ad-9086f6fec328" />


 
- **STM32F303RE (Nucleo‑64)**  
  - Timer‑triggered ADC @ 48 KHz, 12 bit  
  - ADC-DMA pipeline stores ADC readings in a 96 uint16_t ping pong circular buffer, ADCBuf
  - On half/full DMA complete: Callbacks rebias (–2048) to 0v for audio standard, apply optional 8x digital gain, and store audio data in int 16_t UsbBuf or UsbBuf2 buffer
        - UsbBuf/UsbBuf2 hold 48 readings-1ms of audio data. ADCBuf holds 96 readings. While one UsbBuf fills, the ADC fills the other half of ADCBuf uninterrupted
  - UsbBuf/UsbBuf2 are then transferred via DMA to the UART1 TX pin, which sends the buffer to the ESP32S3
    <img width="486" height="439" alt="image" src="https://github.com/user-attachments/assets/faf94b69-d7c5-457a-b736-1667df30e61a" />

 
- **Adafruit Metro (ESP32‑S3)**  
  - Sets up softAP Wifi network
  - Listens on hardware Serial1 @ 3 Mbps
  - Reassembles 96 B frames, repacks to int16_t, broadcasts on UDP port 8000
  - Raw PCM audio stream playable in VLC (set demux to “rawaud”, sample rate 48000, signed 16‑bit, little endian, stereo=off)
        - Can also use FFmpeg or python scripts to listen to UDP port 8000 - VLC is just the simplest option


##  In Depth
- **Analog front end**
  - **Initial Plan:** The biggest initial challenges to work with were having to ensure the input audio was within the audio range (20hz to 20khz), removing the DC offset
    of audio data and making it ADC readable, and adhering to the Nyquist frequency rule-because I was sampling at 48 khz, I had to ensure that
    my audio didn't exceed 24 khz, or else the audio would "fold back" into lower frequencies and distort the existing audio. I realized I could just
    use a series capacitor to remove any DC offset due to a capacitors impedence to DC voltage in an AC circuit. I then "raised" that AC voltage up to
    a base DC voltage of 1.65 volts with a voltage divider, as the STM's ADC has a range of 0 to 3.3 volts. Rebiasing the AC voltage to 1.65 volts allows
    for the ADC to read maximal AC swing without the risk of clipping.

    I had unknowingly also made a high pass filter with my rebiasing and AC coupling. At high frequencies, the capacitor has a low impedence, and allows
    the output voltage to be almost identical to the input voltage. At low frequencies, the capacitor's impedence is high, blocking the input voltage
    more and more as the frequency decreases. This blocks stray current, noise, and other random sounds/heat that can disrupt the signal.

    After this, I built a simple low pass filter for anti aliasing, or making sure that audio at high frequencies above the Nyquist rate don't fold in
    to audio at lower frequencies, which would distort the audio. I did this by "flipping" the high pass filter. A resistor was in series with the voltage,
    and a capacitor offered a path for the voltage into ground. When frequency is high, the capacitor's low impedence means that the high frequency voltage
    goes straight into ground, protecting the rest of the circuit by blocking it. When frequency is lower than the cutoff frequency, the capacitor
    has relatively high impedence, meaning the voltage has no discharge path into ground and "proceeds" into the next stage of the circuit.

    For both filters, the cutoff frequencies can be calculated by the following: _Cutoff Frequency = 1/(2pi*R*C)_
    After these filters, I began the software component of the project, and mostly left the analog circuit alone. 
    
  - **After Trials and Debugging:**  During testing, no matter how snappy my software code was, and even after removing any digital gain, the audio
    was incredibly low quality. Outside of 20-20Khz sin waves, I couldn't discern anything that was playing-everything sounded like static. After consulting
    stack overflow and some other threads, I realized that my AC voltage was only swinging about 0.6 volts arond my DC voltage of 1.65 volts-which meant I was
    underusing my ADC. The nucleo board's onboard ADC has 12 bit clarity from 0-3.3 volts, and currently my passive front end only gave it a signal swinging from
    1 volt to 2.2 volts-which meant I was using just a third of the ADC's range, greatly diminshing audio quality. Additionally, most audio tends to be quieter
    and has a variety of tones all playing at once, which I was "mushing" all together into a single reading, which contributed to the distortion and static. The ADC,
    when reading, can also disrupt the front end circuit due to its current draw. To alleviate this, I connected 2 op amps to the end of the circuit:
    one wired as a voltage buffer, and one wired as a voltage amplifier(1.5x). This isolated the ADC's current draw from the front end and increased the AC swing to about
    0.9 volts. Although the static is still present, softer audio is discernible and understandable, and I can reliably use it to play instrumental songs.

    After further research, I believe some of the static may be due to electromagnetic interference between the resistors and wires, which are all placed in very close
    proximity. Although the op-amp at the end of the front end is high-Z and minimizes current flow, the actual filters in the beginning have a significant amount of
    current passing through them, and the resulting magnetic field and flux might be causing a lot of the static.


- **Software**
  - **Initial Plan:** This was relatively simple compared th the analog circuit. On the STM, Timer 6 was configured to trigger at 48khz and raise an update event flag.
    ADC1, then, was configured to make a reading at every timer 6 update event, and, via DMA, push it to a circular memory uint16_t buffer ADCBuf. When ADCBuf is half full or      completely full, the respective DMA callback triggers. In these callbacks, the appropriate half of ADCBuf is rebiased by subtracting 2048(equivalent to ADC reading of
    1.65 volts) and packaged into a 48 int16_t buffer(UsbBuf or UsbBuf2) to send. Meanwhile, as this was done via a separate DMA channel, the ADC-DMA pipeline
    would continue to run, and the other half of ADCBuf would fill while the current half was being processed. Similarly, while one buffer, UsbBuf was sending,
    UsbBuf2 would be being processed by the callback. This ensures that the audio processing never stops. 

    I had originally intended for this project to be an analog to USB audio adapter, and wanted to stream the audio to my laptop via USB. the USB breakout boards I had
    all proved faulty though, so I ended up having to think of a workaround, which either had to be wifi or bluetooth. As my only other MCU, the ESP32S3, does not
    support bluetooth audio streaming, I had to use wifi. Using ArduinoIDE for a quick prototype, I had the ESP32S3 open its own softAP wifi network and open a UDP
    port(8000) within it. Everytime the ESP32S3 received a 96 byte buffer over USART, it would repackage it from serial back into buffer form and stream it to the UDP port.

    
  - **After Trials and Debugging:** My STM and ESP32S3 boards communicated via USART, and based on the number of dropped audio packets and buffering,
    it was very clear that my initial USART rate of 2Mb/s baud was way too low. Even though the transfer of a 96 byte buffer takes less than half a millisecond,
    it still didn't leave enough time for the ESP32S3 to process it in time, leading to the serial buffer accumulating and the ESP32S3 "lagging", dropping
    packets regularly. I somewhat fixed this by upping the baud rate to 3Mb/s, but it was clear that the ESP32S3 board was still struggling due to
    how hot it would get and the persistent buffering in the audio playback. Additionally, the wifi stack was getting constrained due to how fast the ESP32S3 had to
    receive, repackage, and broadcast audio data, and my laptop would randomly get booted from the wifi network while listening to audio. I fixed this by
    including yield() after every CPU intensive task, which alleviated the problem but also introduced more buffering to the stream.

    It became clear that these were limitations of the ArduinoIDE, as no matter how bare bones the code was, the buffering continued. I am currently working on
    rewriting the Arduino code in Espressif IDE to "bare-metal" the ESP32S3 and make use of the dual cores, something which was very finnicky when I tried
    doing it in ArduinoIDE. Once this is done, i plan to add stereo audio support.

    
