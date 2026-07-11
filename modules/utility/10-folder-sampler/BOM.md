(AI generated)

# Bill of Materials for Eurorack Sample Folder Player

## Components

### Microcontroller and Logic
- **1x STM32H743 MCU**  
  - High-performance microcontroller to handle audio playback, SD card interface, and CV processing.

- **1x Audio Codec (CS4385)**  
  - 8-channel DAC for high-quality audio output (6 channels used, 2 unused for now).

### Input Components
- **6x Trigger Inputs**  
  - For triggering sample playback.

- **6x Resistors (10k\u03a9)**  
  - Pull-down resistors for trigger inputs.

- **6x LEDs**  
  - One LED per trigger input for visual feedback.

- **6x Current-Limiting Resistors (330\u03a9)**  
  - For LEDs connected to trigger inputs.

- **2x CV Inputs**  
  - For pitch control of channels 1 and 2 (1V/oct scaling).

- **2x Potentiometers (10k\u03a9)**  
  - For manual pitch adjustment of channels 1 and 2.

- **2x Buttons**  
  - For manual folder navigation.

### Analog Circuitry
- **1x TL074 Quad Op-Amp**  
  - For scaling and buffering CV inputs.

- **1x MCP6004 Op-Amp**  
  - For scaling CV to 0-3.3V for the STM32 ADC.

- **4x Resistors (10k\u03a9)**  
  - For voltage divider and scaling circuits.

- **2x Diodes (1N4148)**  
  - For overvoltage protection on CV inputs.

- **2x Capacitors (100nF)**  
  - For filtering CV inputs.

### SD Card Interface
- **1x MicroSD Card Socket**  
  - For loading and storing sample files.

- **1x SD Card (32GB or larger)**  
  - To store WAV files in the specified folder structure.

### Output Components
- **6x Audio Outputs**  
  - Individual outputs for each triggered sample.

- **6x Resistors (10k)**  
  - For output impedance matching.

### Visual Feedback
- **4x LEDs**  
  - Represent the binary folder number (0-127).

- **4x Current-Limiting Resistors (330)**  
  - For binary display LEDs.

### Power Supply
- **1x Eurorack Power Header**  
  - For connecting to Eurorack +/-12V rails.

- **1x 3.3V Voltage Regulator (e.g., LM1117)**  
  - To power the STM32 and digital components.

- **1x 5V Voltage Regulator (e.g., 7805)**  
  - To power the audio codec and other components.

- **2x Capacitors (100\u03bcF and 0.1\u03bcF)**  
  - For power supply decoupling.

### Miscellaneous
- **1x PCB**  
  - Custom-designed PCB for the module.

- **1x Enclosure**  
  - Eurorack-compatible panel and enclosure.
