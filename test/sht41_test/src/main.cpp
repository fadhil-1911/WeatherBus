#include <Wire.h>
#include <Adafruit_SHT4x.h>

#define SDA_PIN 8
#define SCL_PIN 9

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("==============================");
    Serial.println("      SHT41 SENSOR TEST");
    Serial.println("==============================");

    // Start I2C
    Wire.begin(SDA_PIN, SCL_PIN);

    Serial.println("I2C started");
    Serial.println("SDA: GPIO 8");
    Serial.println("SCL: GPIO 9");
    Serial.println();

    // Initialize SHT41
    if (!sht4.begin()) {
        Serial.println("ERROR: SHT41 tidak dikesan!");
        Serial.println("Expected I2C address: 0x44");
        return;
    }

    Serial.println("SHT41 detected!");
    Serial.println("I2C address: 0x44");

    // Set high precision mode
    sht4.setPrecision(SHT4X_HIGH_PRECISION);

    // Heater OFF
    sht4.setHeater(SHT4X_NO_HEATER);

    Serial.println("Precision: HIGH");
    Serial.println("Heater: OFF");
    Serial.println();
    Serial.println("Reading sensor...");
    Serial.println();
}

void loop() {

    sensors_event_t humidity;
    sensors_event_t temperature;

    if (!sht4.getEvent(&humidity, &temperature)) {
        Serial.println("ERROR: Failed to read SHT41");
        delay(2000);
        return;
    }

    Serial.print("Temperature : ");
    Serial.print(temperature.temperature, 2);
    Serial.println(" °C");

    Serial.print("Humidity    : ");
    Serial.print(humidity.relative_humidity, 2);
    Serial.println(" %RH");

    Serial.println("------------------------------");

    delay(2000);
}