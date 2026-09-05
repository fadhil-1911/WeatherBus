//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    WeatherBus
//                   Version: 1.0
//             Last Updated: 2026-09-05
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
  Module  : Sensor Node - Main Application
  Transport : ESP-NOW
  Phase   : PHASE 2A - Multi Node Dummy
*/

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

#include "../common/weatherbus_protocol.h"

using namespace WeatherBus;

// =====================================================
// CHANGE THIS FOR EACH NODE
// =====================================================

constexpr uint8_t NODE_ID = 2;

// Node 1:
// 25.50 C / 70.00 %

// Node 2:
// 26.50 C / 71.00 %

// =====================================================
// Base MAC
// =====================================================
uint8_t baseMac[] = {
    0x0C, 0x4E, 0xA0,
    0x4D, 0x7C, 0x40
};
/*
uint8_t baseMac[] = {

    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00
}; */

// =====================================================
// Dummy Sensor
// =====================================================

void getDummySensorData(
    float& temperature,
    float& humidity
) {

    if (NODE_ID == 1) {

        temperature = 25.50f;
        humidity = 70.00f;

    } else if (NODE_ID == 2) {

        temperature = 26.50f;
        humidity = 71.00f;

    } else {

        temperature = 20.00f + NODE_ID;

        humidity = 60.00f + NODE_ID;
    }
}

// =====================================================
// Send SENSOR_DATA
// =====================================================

void sendSensorData(
    uint16_t requestSequence
) {

    SensorDataPacket packet{};

    packet.header.protocolVersion =
        PROTOCOL_VERSION;

    packet.header.packetType =
        static_cast<uint8_t>(
            PacketType::SENSOR_DATA
        );

    packet.header.nodeId =
        NODE_ID;

    packet.header.flags = 0;

    // Same sequence as request
    packet.header.sequence =
        requestSequence;

    packet.header.payloadLength =
        sizeof(SensorDataPayload);

    // CRC disabled in Phase 2A
    packet.header.crc16 = 0;

    packet.header.reserved = 0;

    getDummySensorData(
        packet.payload.temperature,
        packet.payload.humidity
    );

    esp_err_t result =
        esp_now_send(
            baseMac,
            reinterpret_cast<uint8_t*>(&packet),
            sizeof(packet)
        );

    if (result == ESP_OK) {

        Serial.printf(
            "TX: SENSOR_DATA | Node=%u | Seq=%u | T=%.2f C | RH=%.2f %%\n",
            NODE_ID,
            requestSequence,
            packet.payload.temperature,
            packet.payload.humidity
        );

    } else {

        Serial.printf(
            "TX ERROR: %d\n",
            result
        );
    }
}

// =====================================================
// Receive DATA_REQUEST
// =====================================================

void onDataReceived(
    const uint8_t* mac,
    const uint8_t* data,
    int len
){

    if (len != sizeof(Header)) {

        Serial.println(
            "RX: Invalid packet size"
        );

        return;
    }

    Header request{};

    memcpy(
        &request,
        data,
        sizeof(request)
    );

    if (request.protocolVersion !=
        PROTOCOL_VERSION) {

        return;
    }

    if (request.packetType !=
        static_cast<uint8_t>(
            PacketType::DATA_REQUEST
        )) {

        return;
    }

    if (request.nodeId != NODE_ID) {

        return;
    }

    Serial.printf(
        "RX: DATA_REQUEST | Node=%u | Seq=%u\n",
        NODE_ID,
        request.sequence
    );

    // -------------------------------------------------
    // Immediate response
    // -------------------------------------------------

    sendSensorData(
        request.sequence
    );
}

// =====================================================
// ESP-NOW setup
// =====================================================

bool setupEspNow() {

    WiFi.mode(WIFI_STA);

    Serial.print("Node MAC: ");

    Serial.println(
        WiFi.macAddress()
    );

    if (esp_now_init() != ESP_OK) {

        Serial.println(
            "ERROR: ESP-NOW initialization failed"
        );

        return false;
    }

    esp_now_register_recv_cb(
        onDataReceived
    );

    esp_now_peer_info_t peerInfo{};

    memcpy(
        peerInfo.peer_addr,
        baseMac,
        6
    );

    peerInfo.channel = 0;

    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {

        Serial.println(
            "ERROR: Failed to add Base peer"
        );

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

    Serial.println(
        "================================"
    );

    Serial.println(
        "WeatherBus V4.1"
    );

    Serial.printf(
        "PHASE 2A - Node %u\n",
        NODE_ID
    );

    Serial.println(
        "================================"
    );

    if (!setupEspNow()) {

        Serial.println(
            "SYSTEM HALTED"
        );

        while (true) {
            delay(1000);
        }
    }

    Serial.println(
        "ESP-NOW ready"
    );
}

// =====================================================
// Loop
// =====================================================

void loop() {

    // No polling required.
    // Node responds only when requested.

}