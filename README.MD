# WeatherBus

**Version:** 1.0

**Last Updated:** 2026-09-07

---

**Project:** WeatherBus V1.0

**Transport:** ESP-NOW

**Current Phase:** PHASE 2B.5 - Sensor Data Validation

---

## Updates

* Added ESP-NOW initialization and peer configuration
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
* Added Node ID validation at Base.
* Added current-node validation at Base.
* Added SENSOR_DATA sequence validation at Base.
* Responses with unexpected sequence numbers are rejected.
* Added sensor temperature validation at Base.
* Added sensor humidity validation at Base.
* Added non-finite floating-point value validation using `isfinite()`.
* Invalid sensor values are rejected without being accepted as valid SENSOR_DATA.

---

## Hardware Test

Current Phase 2B.5 hardware configuration:

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

Verified:

* Invalid packet size handling.
* Protocol version validation.
* Packet type validation.
* Node ID validation.
* Payload length validation for DATA_REQUEST.
* Invalid packets are rejected without generating a SENSOR_DATA response.

Normal communication remained unaffected after invalid packet handling was implemented.

---

## Phase 2B.2 Test Status

**PASS**

Verified:

* Node response timeout.
* Node offline detection.
* Continued polling of available nodes.
* Automatic node recovery after reconnection.
* Repeated node disconnect/reconnect operation.

The Base continues polling other nodes when one node becomes unavailable.

---

## Phase 2B.3 Test Status

**PASS**

Verified:

* SHT41 communication failure detection.
* SENSOR_READ_FAILED handling.
* No fake sensor data is transmitted when the SHT41 read fails.
* Node firmware continues running during sensor failure.
* SHT41 operation recovers after the sensor is reconnected.

---

## Phase 2B.4 Test Status

**PASS — LOCKED**

Verified:

* Base stores the expected sequence for each DATA_REQUEST.
* Node echoes the request sequence in SENSOR_DATA.
* Base rejects responses with unexpected sequence numbers.
* Sequence mismatch does not generate a valid WEATHERBUS RX result.
* Timeout occurs when no valid matching response is received within 200 ms.
* Normal sequence synchronization resumes after the test condition is removed.

---

## Phase 2B.5 Test Status

**PASS — LOCKED**

### Sensor Data Validation

The Base now validates the actual sensor values after packet, Node ID, and sequence validation.

Temperature validation:

```text
-40.0 °C ≤ Temperature ≤ 85.0 °C
```

Humidity validation:

```text
0.0 % ≤ Humidity ≤ 100.0 %
```

Non-finite floating-point values are also rejected:

```text
NaN
+Infinity
-Infinity
```

### Validation Order

The Base now follows this validation sequence:

1. Packet size
2. Protocol version
3. Packet type
4. Payload length
5. Node ID
6. Current Node
7. Sequence
8. Temperature
9. Humidity
10. Accept SENSOR_DATA
11. Mark Node online

Invalid sensor data is rejected before the packet is accepted as valid SENSOR_DATA.

### Negative Test

A temporary invalid temperature of:

```text
150.00 °C
```

was intentionally transmitted by Node 2.

Expected rejection was observed:

```text
RX: Invalid temperature | 150.00 C
TIMEOUT: Node 2 did not respond within 200 ms
```

The Base did not generate a valid WEATHERBUS RX block for the invalid packet.

Node 1 continued normal operation while Node 2 was producing invalid sensor data.

### Post-Restore Regression

Node 2 was restored to normal sensor operation.

Regression testing verified:

* Node 1 continues to return valid SHT41 data.
* Node 2 returns valid SHT41 data.
* Request and response sequences remain synchronized.
* No sequence mismatch occurred.
* No timeout occurred during normal operation.
* No invalid sensor data was reported.
* Response time remained well below the 200 ms timeout.
* Sequential polling continues correctly.

Therefore:

**WeatherBus V1.0 Phase 2B.5 is functionally validated and LOCKED.**

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

The Base validates the received SENSOR_DATA before accepting it as valid data.

The Base validates:

```text
Packet
  ↓
Node
  ↓
Sequence
  ↓
Temperature
  ↓
Humidity
  ↓
Valid SENSOR_DATA
```

---

## Current Phase Status

```text
Phase 1       PASS
Phase 2A      PASS
Phase 2B.1    PASS
Phase 2B.2    PASS
Phase 2B.3    PASS
Phase 2B.4    PASS 🔒
Phase 2B.5    PASS 🔒
```

The current implementation has been tested using:

* 1 × Base
* 2 × Nodes
* 2 × SHT41 sensors

The current hardware test is limited to two Nodes.

The software architecture uses logical Node IDs and can be extended when additional hardware becomes available.

---

## Next Phase

**PHASE 2B.6 - Duplicate / Stale Response Handling**

Planned focus:

* Handling duplicate SENSOR_DATA responses.
* Handling stale responses from previous requests.
* Ensuring an already-completed request cannot be completed twice.
* Verifying that delayed packets do not corrupt the current polling cycle.
* Maintaining stable sequential polling after duplicate or stale packets.

---

## Phase 2B Objective

Phase 2B is progressively strengthening the reliability of WeatherBus communication.

Current protection layers include:

```text
Invalid Packet
      ↓
Node Failure
      ↓
Sensor Failure
      ↓
Sequence Validation
      ↓
Sensor Data Validation
```

Each layer is independently tested before moving to the next phase.
