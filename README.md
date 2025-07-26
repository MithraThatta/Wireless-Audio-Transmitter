# Wireless-Audio-Transmitter
Takes in a 3.5mm jack as an analog audio input and outputs the recorded audio on a UDP port on a wifi network, using an STMF303RE for digitizing/processing audio data and an ESP32S3 to broadcast to wifi. Communication between the two MCUs is done via a high speed USART line.

An extensive passive front end analog circuit is used for ensuring received audio is within the 20hz to 20khz audio band(via low pass filter and anti aliasing by high pass filter) and ensuring that the ADC does not disrupt the signal when reading via a unity gain op amp. A second op amp is used as a voltage amplifier, to boost the sound of audio coming from things like songs and speeches(which have sounds coming from various sources at varying intensities), above the noise created by the limited range of the ADC and the thermal/resistor noise from the passive front end circuit.

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

    
  - 
