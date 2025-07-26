# Wireless-Audio-Transmitter
Takes in a 3.5mm jack as an analog audio input and outputs the recorded audio on a UDP port on a wifi network, using an STMF303RE for digitizing/processing audio data and an ESP32S3 to broadcast to wifi. Communication between the two MCUs is done via a high speed USART line.

An extensive passive front end analog circuit is used for ensuring received audio is within the 20hz to 20khz audio band(via low pass filter and anti aliasing by high pass filter) and ensuring that the ADC does not disrupt the signal when reading via a unity gain op amp. A second op amp is used as a voltage amplifier, to boost the sound of audio coming from things like songs and speeches(which have sounds coming from various sources at varying intensities), above the noise created by the limited range of the ADC and the thermal/resistor noise from the passive front end circuit.

## 📋 Features

- **Analog front end**
  - AC coupling(10 µF) capacitor to block DC voltage
  - 20 Hz single‑pole low‑pass (827 Ω/10 nF) to block frequencies below the audio band lower threshold 
  - 1.65 volt rebiasing via 2 100kΩ resistors to fit audio within range of ADC(0 to 3.3 volts)
  - 24 kHz single‑pole high‑pass (10 µF/100 kΩ) for anti aliasing, adherence to Nyquist Frequency rule
  - Unity‑gain op-amp buffer to isolate the ADC input and its associated current draw surges
  - Voltage amplifying op-amp to amplify audio(voltage) swings in softer audio, such as songs and speeches
