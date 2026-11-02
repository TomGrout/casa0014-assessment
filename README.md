### How to Build and Run  
1. Clone this repository and open the folder in VS Code with the PlatformIO extension installed.  
2. Duplicate `include/arduino_secrets.h.example` and rename it to `include/arduino_secrets.h`.  
3. Update the constants in `arduino_secrets.h` with your local Wi-Fi and MQTT credentials.  
4. Connect the Arduino MKR WiFi 1010 via USB and click the **PlatformIO: Upload**(1) arrow. The `platformio.ini` file will automatically fetch the libraries needed (`WiFiNINA` and `PubSubClient`).  

![alt text](image.png)  
**(1)**: PlatformIO: Upload (Right facing arrow on bottom row)  


### Parts  
See resources/bill-of-materials.txt  
Follow 

### pinout & truth table


### design rationale

