# WeatherBus

**Version:** 1.0

**Last Updated:** 2026-09-06

---

**Project:** WeatherBus V1.0

**Transport:** ESP-NOW

**Phase:** PHASE 2A - Multi Node + SHT41

---

## Updates

* Added ESP-NOW initialization and peer configuration.
* Added DATA_REQUEST reception and validation.
* Added SENSOR_DATA response handling.
* Added WeatherBus protocol header validation.
* Replaced dummy temperature and humidity data with real SHT41 sensor readings.
* Added SHT41 I²C communication using GPIO 8 (SDA) and GPIO 9 (SCL).
* Added SHT41 communication detection during node startup.
* Added SHT41 measurement and data conversion.
* Added SHT41 CRC validation for temperature and humidity data.
* Added request sequence synchronization between Base and Node.
* Added multi-node sequential polling support.
* Added 200 ms response timeout handling at Base.
* Verified node offline timeout and automatic recovery.
* Verified continuous polling with two sensor nodes.

---

## Hardware Test

Current Phase 2A hardware configuration:

* 1 × WeatherBus Base
* 2 × WeatherBus Nodes
* 2 × SHT41 sensors
* ESP32-C3
* ESP-NOW wireless transport

### SHT41 Wiring

| SHT41 | ESP32-C3 |
| ----- | -------- |
| SDA   | GPIO 8   |
| SCL   | GPIO 9   |

### Node Configuration

Each node uses the same firmware.

The logical Node ID is manually configured in:

```cpp
constexpr uint8_t NODE_ID = 1;
```

For Node 2:

```cpp
constexpr uint8_t NODE_ID = 2;
```

---

## Phase 2A Test Status

**PASS**

Verified:

* Base → Node DATA_REQUEST
* Node → Base SENSOR_DATA
* Node 1 and Node 2 sequential polling
* Temperature and humidity from real SHT41 sensors
* Sequence synchronization
* Response time below 200 ms
* Node timeout handling
* Continued operation when a node is offline
* Automatic recovery when the node returns online

---

## Current Architecture

```text
                    WeatherBus Base
                         ESP32-C3
                            │
                    DATA_REQUEST
                            │
              ┌─────────────┴─────────────┐
              │                           │
           Node 1                      Node 2
          ESP32-C3                    ESP32-C3
              │                           │
            SHT41                       SHT41
              │                           │
        SENSOR_DATA                  SENSOR_DATA
              │                           │
              └─────────────┬─────────────┘
                            │
                         Base
```

The Base polls nodes sequentially. Each Node responds only when requested.

---

## Phase 2A Result

**WeatherBus V1.0 Phase 2A is functionally validated with two sensor nodes.**

The current hardware test is limited to **two Nodes**. The software architecture is designed around logical Node IDs and can be extended when additional hardware becomes available.
