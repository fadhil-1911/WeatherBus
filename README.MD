# WeatherBus

**Version:** 1.0

**Last Updated:** 2026-09-06

---

**Project:** WeatherBus V1.0

**Transport:** ESP-NOW

**Phase:** PHASE 2B.1 - Invalid Packet Handling

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
* Added invalid packet size validation at Node.
* Added protocol version validation at Node.
* Added packet type validation at Node.
* Added Node ID validation at Node.
* Added payload length validation for DATA_REQUEST.
* Invalid packets are rejected without generating a SENSOR_DATA response.

---

## Hardware Test

Current Phase 2B.1 hardware configuration:

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

Previously verified:

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

## Phase 2B.1 Test Status

**PASS**

Normal operation verified after adding invalid packet handling:

* Base → Node DATA_REQUEST continues to operate normally.
* Node 1 responds correctly with real SHT41 data.
* Node 2 responds correctly with real SHT41 data.
* Request sequence remains synchronized.
* SHT41 temperature and humidity readings are received correctly.
* Response time remains below the 200 ms timeout.
* Invalid packet validation does not interfere with normal communication.

### Invalid Packet Handling

The Node now validates incoming DATA_REQUEST packets before processing them.

Validation includes:

1. Packet size
2. Protocol version
3. Packet type
4. Node ID
5. Payload length

If validation fails, the packet is rejected and no SENSOR_DATA response is generated.

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

The Base polls nodes sequentially.

Each Node responds only when a valid DATA_REQUEST addressed to that Node is received.

The Node performs packet validation before accessing the sensor or generating a response.

---

## Phase 2B.1 Result

**WeatherBus V1.0 Phase 2B.1 is functionally validated for normal operation with invalid packet handling implemented.**

The current hardware test is limited to **two Nodes**.

The software architecture uses logical Node IDs and can be extended when additional hardware becomes available.

---

## Next Phase

**PHASE 2B.2 - Timeout & Node Failure Handling**

Planned focus:

* Node response timeout
* Node offline detection
* Continued polling of available nodes
* Node recovery after returning online
* Stable operation during repeated node disconnect/reconnect events
