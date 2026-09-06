//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    WeatherBus
//                   Version: 1.0
//             Last Updated: 2026-09-06
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
  Module    : Sensor Node - Main Application
  Transport : ESP-NOW
  Phase     : PHASE 2B.1 - Invalid Packet Handling
*/

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_now.h>

#include "../common/weatherbus_protocol.h"

using namespace WeatherBus;

// =====================================================
// NODE CONFIGURATION
// =====================================================

constexpr uint8_t NODE_ID = 2;

// =====================================================
// SHT41 CONFIGURATION
// =====================================================

constexpr uint8_t SHT41_ADDRESS = 0x44;
constexpr uint8_t SHT41_SDA = 8;
constexpr uint8_t SHT41_SCL = 9;

constexpr uint8_t SHT41_MEASURE_HIGH_PRECISION = 0xFD;

// =====================================================
// Base MAC
// =====================================================

uint8_t baseMac[] = {
    0x0C, 0x4E, 0xA0,
    0x4D, 0x7C, 0x40};

// =====================================================
// SHT41 CRC-8
// Polynomial: 0x31
// =====================================================

uint8_t sht41Crc8(const uint8_t* data, uint8_t length) {
    uint8_t crc = 0xFF;

    for (uint8_t i = 0; i < length; i++) {
        crc ^= data[i];

        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

// =====================================================
// Setup SHT41
// =====================================================

bool setupSHT41() {
    Wire.begin(SHT41_SDA, SHT41_SCL);
    Wire.setClock(100000);
    delay(10);

    Wire.beginTransmission(SHT41_ADDRESS);
    Wire.write(SHT41_MEASURE_HIGH_PRECISION);

    if (Wire.endTransmission() != 0) {
        Serial.println("ERROR: SHT41 communication failed");
        return false;
    }

    delay(10);
    Serial.println("SHT41 communication OK");

    return true;
}

// =====================================================
// Read SHT41
// =====================================================

bool readSHT41(float& temperature, float& humidity) {
    Wire.beginTransmission(SHT41_ADDRESS);
    Wire.write(SHT41_MEASURE_HIGH_PRECISION);

    if (Wire.endTransmission() != 0) {
        Serial.println("ERROR: SHT41 measurement command failed");
        return false;
    }

    delay(10);

    uint8_t received = Wire.requestFrom(
        SHT41_ADDRESS,
        (uint8_t)6);

    if (received != 6) {
        Serial.println("ERROR: SHT41 invalid response length");
        return false;
    }

    uint8_t data[6];

    for (uint8_t i = 0; i < 6; i++) {
        data[i] = Wire.read();
    }

    if (sht41Crc8(data, 2) != data[2]) {
        Serial.println("ERROR: SHT41 temperature CRC failed");
        return false;
    }

    if (sht41Crc8(&data[3], 2) != data[5]) {
        Serial.println("ERROR: SHT41 humidity CRC failed");
        return false;
    }

    uint16_t rawTemperature =
        ((uint16_t)data[0] << 8) | data[1];

    uint16_t rawHumidity =
        ((uint16_t)data[3] << 8) | data[4];

    temperature =
        -45.0f + 175.0f *
                     ((float)rawTemperature / 65535.0f);

    humidity =
        -6.0f + 125.0f *
                    ((float)rawHumidity / 65535.0f);

    if (humidity < 0.0f) {
        humidity = 0.0f;
    }

    if (humidity > 100.0f) {
        humidity = 100.0f;
    }

    return true;
}

// =====================================================
// Send SENSOR_DATA
// =====================================================

void sendSensorData(uint16_t requestSequence) {
    float temperature;
    float humidity;

    if (!readSHT41(temperature, humidity)) {
        Serial.println("ERROR: SENSOR_READ_FAILED");
        return;
    }

    SensorDataPacket packet{};

    packet.header.protocolVersion = PROTOCOL_VERSION;
    packet.header.packetType =
        static_cast<uint8_t>(PacketType::SENSOR_DATA);

    packet.header.nodeId = NODE_ID;
    packet.header.flags = 0;
    packet.header.sequence = requestSequence;
    packet.header.payloadLength = sizeof(SensorDataPayload);
    packet.header.crc16 = 0;
    packet.header.reserved = 0;

    packet.payload.temperature = temperature;
    packet.payload.humidity = humidity;

    esp_err_t result = esp_now_send(
        baseMac,
        reinterpret_cast<uint8_t*>(&packet),
        sizeof(packet));

    if (result == ESP_OK) {
        Serial.printf(
            "TX: SENSOR_DATA | Node=%u | Seq=%u | T=%.2f C | RH=%.2f %%\n",
            NODE_ID,
            requestSequence,
            temperature,
            humidity);
    } else {
        Serial.printf("TX ERROR: %d\n", result);
    }
}

// =====================================================
// Receive DATA_REQUEST
// =====================================================

void onDataReceived(
    const uint8_t* mac,
    const uint8_t* data,
    int len) {
    if (len != sizeof(Header)) {
        Serial.printf(
            "RX ERROR: Invalid packet size | Received=%d | Expected=%u\n",
            len,
            sizeof(Header));

        return;
    }

    Header request{};

    memcpy(&request, data, sizeof(request));

    if (request.protocolVersion != PROTOCOL_VERSION) {
        Serial.printf(
            "RX ERROR: Invalid protocol version | Received=%u | Expected=%u\n",
            request.protocolVersion,
            PROTOCOL_VERSION);

        return;
    }

    if (request.packetType != static_cast<uint8_t>(PacketType::DATA_REQUEST)) {
        Serial.printf("RX ERROR: Invalid packet type | Received=0x%02X\n",
                      request.packetType);
        return;
    }

    if (request.nodeId != NODE_ID) {
        Serial.printf("RX ERROR: Invalid Node ID | Received=%u | This Node=%u\n",
                      request.nodeId,
                      NODE_ID);

        return;
    }

    if (request.payloadLength != 0) {
        Serial.printf("RX ERROR: Invalid payload length | Received=%u | Expected=0\n",
                      request.payloadLength);

        return;
    }

    Serial.printf("RX: DATA_REQUEST | Node=%u | Seq=%u\n",
                  NODE_ID,
                  request.sequence);

    sendSensorData(request.sequence);
}

// =====================================================
// ESP-NOW setup
// =====================================================

bool setupEspNow() {
    WiFi.mode(WIFI_STA);

    Serial.print("Node MAC: ");
    Serial.println(WiFi.macAddress());

    if (esp_now_init() != ESP_OK) {
        Serial.println("ERROR: ESP-NOW initialization failed");
        return false;
    }

    esp_now_register_recv_cb(onDataReceived);

    esp_now_peer_info_t peerInfo{};

    memcpy(peerInfo.peer_addr, baseMac, 6);

    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("ERROR: Failed to add Base peer");
        return false;
    }

    return true;
}

// =====================================================
// Setup
// =====================================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println("WeatherBus V1.0");
    Serial.printf("PHASE 2B.1 - Node %u\n", NODE_ID);
    Serial.println("================================");

    if (!setupSHT41()) {
        Serial.println("SHT41 FAILED");
        Serial.println("SYSTEM HALTED");

        while (true) {
            delay(1000);
        }
    }

    if (!setupEspNow()) {
        Serial.println("SYSTEM HALTED");

        while (true) {
            delay(1000);
        }
    }

    Serial.println("ESP-NOW ready");
    Serial.println("Node ready");
}

// =====================================================
// Loop
// =====================================================

void loop() {
    // No polling required.
    //
    // Node responds only when a valid
    // DATA_REQUEST is received.
}