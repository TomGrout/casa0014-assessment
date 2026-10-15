#include <Arduino.h>
#include <Arduino_LSM9DS1.h>

// from ble 33 nano docs arduino.com

float x, y, z;

int angleX = 0;

int angleY = 0;

unsigned long previousMillis = 0;


void setup() {
  Serial.begin(9600);

  while (!Serial);


  if (!IMU.begin()) {

    Serial.println("Failed to initialize IMU!");

    while (1);

  }


  Serial.print("Accelerometer sample rate = ");

  Serial.print(IMU.accelerationSampleRate());

  Serial.println(" Hz");
}

void loop() {
  unsigned long currentMillis = millis();


  if (IMU.accelerationAvailable() && (currentMillis - previousMillis >= 1000)) {

    previousMillis = currentMillis;


    IMU.readAcceleration(x, y, z);


    // Calculate tilt angles in degrees

    angleX = atan2(x, sqrt(y * y + z * z)) * 180 / PI;

    angleY = atan2(y, sqrt(x * x + z * z)) * 180 / PI;


    // Determine the tilting direction based on angleX and angleY

    if (angleX > 30) {  // Tilting up

      Serial.print("Tilting up ");

      Serial.print(angleX);

      Serial.println(" degrees");

    } else if (angleX < 30) {  // Tilting down

      Serial.print("Tilting down ");

      Serial.print(-angleX);

      Serial.println(" degrees");

    }


    if (angleY > 30) {  // Tilting left

      Serial.print("Tilting left ");

      Serial.print(angleY);

      Serial.println(" degrees");

    } else if (angleY < 30) {  // Tilting right

      Serial.print("Tilting right ");

      Serial.print(-angleY);

      Serial.println(" degrees");

    }

  }
}
