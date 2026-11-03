### How to Build and Run  
1. Clone this repository and open the folder in VS Code with the PlatformIO extension installed.  
2. Duplicate `include/arduino_secrets.h.example` and rename it to `include/arduino_secrets.h`.  
3. Update the constants in `arduino_secrets.h` with your local Wi-Fi and MQTT credentials.  
4. Connect the Arduino MKR WiFi 1010 via USB and click the **PlatformIO: Upload**(1) arrow. The `platformio.ini` file will automatically fetch the libraries needed (`WiFiNINA` and `PubSubClient`).  

![alt text](image.png)  
**(1)**: PlatformIO: Upload (Right facing arrow on bottom row)  


### Parts (Bill of Materials)  
1x Arduino MKR WiFi 1010, 
8x push buttons, 
4x LED (RGBY), 
4 x 330ohm resistors
21x jumper wires

### pinout & truth table
see: documentation/Pinout and Truth Table.xlsx

### design rationale
The design rationale centers on a fast-paced, two-player sequence-matching arcade game with low cognitive load and high social engagement.  

- Core Mechanic: 2-player competitive sequence-matching race.  
- Hardware: Shared Arduino MKR WiFi 1010 managing 8 coloured buttons and 4 central LEDs.  
- Network: 216-byte MQTT payload updates (publishing only on score changes).  
- Housing: Medium-fidelity cardboard enclosure with strictly no 3D printing.  

The result is a functional tabletop travel console that generates randomized LED target sequences and drives 'territory captures' (i.e. players competiting to gain LEDs and control of the Vespera) on the 72-LED Vespera luminaire via MQTT.  

I learned that the physical messiness of hardware (specifically contact bounce and voltage constraints) necessitates robust, non-blocking software debouncing to ensure fairness and legibility in a shared environment.  