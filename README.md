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

<img width="3840" height="2160" alt="image" src="https://github.com/user-attachments/assets/25355148-e3eb-4aab-af9b-f568621e96bd" />  
circuit diagram  

### inspiration  

the game concept was inspired by Mastermind the Board Game, but adding 2 player interactivity makes the concept a lot more engaging, while simplifying the rules down to gear towards children.  
<img width="2000" height="2000" alt="image" src="https://github.com/user-attachments/assets/d44f16c6-536a-4146-89e9-d7a669798f80" />  

<img width="480" height="640" alt="image" src="https://github.com/user-attachments/assets/0f44cce4-9897-47a7-aac0-0d54ba57daeb" />  

and Simon - Follow light sequence, Bop, Twist, or Pull to score points.  
<img width="195" height="181" alt="image" src="https://github.com/user-attachments/assets/6e895174-2b35-49f4-8f4b-82b4ce30bb02" />  


### product  

<img width="3024" height="4032" alt="final-design" src="https://github.com/user-attachments/assets/571a2306-b02c-4daf-84d5-ef890e7d575f" />  
with cover  
<img width="3024" height="4032" alt="IMG_8845" src="https://github.com/user-attachments/assets/76611f37-a78a-461e-91fc-8da97ff0bad6" />  
without cover  

