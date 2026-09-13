# WeatherBus

**Version:** 1.0

**Last Updated:** 2026-09-13

---

**Project:** WeatherBus V1.0

**Transport:** ESP-NOW

**Current Phase:** PHASE 2B.7 - Node Recovery & Repeated Failure

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
* Added Node ID validation at Base.
* Added current-node validation at Base.
* Added SENSOR_DATA sequence validation at Base.
* Responses with unexpected sequence numbers are rejected.
* Added sensor temperature validation at Base.
* Added sensor humidity validation at Base.
* Added non-finite floating-point value validation using `isfinite()`.
* Invalid sensor values are rejected without being accepted as valid SENSOR_DATA.
* Added Base response-state validation.
* SENSOR_DATA received when no request is active is rejected.
* Added late response handling after request timeout.
* Verified that delayed SENSOR_DATA does not corrupt the current polling cycle.
* Verified Node failure, timeout, recovery, and continued sequential polling.
* Verified Node recovery without restarting the Base.
* Verified repeated normal operation after Node reconnection.

---

## Hardware Test

Current Phase 2B.7 hardware configuration:

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

* Base → Node DATA_REQUEST.
* Node → Base SENSOR_DATA.
* Node 1 and Node 2 sequential polling.
* Temperature and humidity from real SHT41 sensors.
* Sequence synchronization.
* Response time below 200 ms.
* Node timeout handling.
* Continued operation when a node is offline.
* Automatic recovery when the node returns online.

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

## Phase 2B.6 Test Status

**PASS — LOCKED**

### Late Response Handling

The Base now verifies that a `SENSOR_DATA` response is only processed when the polling engine is actively waiting for a response.

The Base rejects responses when:

```text
State != WAIT_RESPONSE
```

The protection is implemented before Node ID, current-node, sequence, and sensor-data validation.

### Controlled Negative Test

Node 2 was temporarily configured with an artificial:

```text
300 ms
```

response delay.

The Base response timeout remained:

```text
200 ms
```

Observed behaviour:

```text
TX: DATA_REQUEST -> Node 2 | Sequence 654
TIMEOUT: Node 2 did not respond within 200 ms
RX: Ignored response | No active request
```

The delayed `SENSOR_DATA` response was therefore ignored.

### Regression Test

After removing the artificial delay:

* Node 1 returned normal SHT41 data.
* Node 2 returned normal SHT41 data.
* Sequential polling resumed normally.
* Response times remained well below 200 ms.
* No unexpected state behaviour was observed.

Therefore:

**WeatherBus V1.0 Phase 2B.6 is functionally validated and LOCKED.**

---

## Phase 2B.7 Test Status

**PASS — LOCKED**

### Node Recovery & Repeated Failure

Phase 2B.7 verifies that the Base remains operational when a Node becomes unavailable and subsequently returns online.

The test sequence was:

```text
1. Node 1 + Node 2 normal
2. Node 2 operates for several cycles
3. Node 2 power is disconnected
4. Base detects Node 2 timeout
5. Node 1 continues operating
6. Node 2 is powered on again
7. Node 2 responds normally
8. Sequential polling returns to normal
```

### Failure Detection

When Node 2 was unavailable:

```text
TX: DATA_REQUEST -> Node 2 | Sequence 444
TIMEOUT: Node 2 did not respond within 200 ms
```

The Base correctly detected the Node failure.

### Continued Operation

The Base immediately continued polling Node 1:

```text
TX: DATA_REQUEST -> Node 1 | Sequence 445

Node ID      : 1
Sequence     : 445
Response Time: 12 ms
```

Node 1 continued operating normally while Node 2 was unavailable.

### Node Recovery

After Node 2 was powered on again:

```text
TX: DATA_REQUEST -> Node 2 | Sequence 446

Node ID      : 2
Sequence     : 446
Response Time: 13 ms
```

Node 2 successfully returned to normal operation without requiring a Base restart.

### Post-Recovery Regression

Sequential polling continued:

```text
Node 1 → Sequence 447
Node 2 → Sequence 448
Node 1 → Sequence 449
Node 2 → Sequence 450
```

Verified:

* Node 2 timeout detection.
* Node 1 continued operating during Node 2 failure.
* Node 2 successfully recovered.
* Node 2 returned valid SHT41 data after recovery.
* Sequence numbers remained synchronized.
* No sequence mismatch occurred.
* No unexpected timeout occurred after recovery.
* Sequential polling continued correctly.
* Base did not require a restart.
* Polling engine did not become stuck after Node failure.

Therefore:

**WeatherBus V1.0 Phase 2B.7 is functionally validated and LOCKED.**

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
Current Node
  ↓
Sequence
  ↓
Temperature
  ↓
Humidity
  ↓
Valid SENSOR_DATA
```

The Base also verifies that a response is only processed while the polling engine is in:

```text
WAIT_RESPONSE
```

Responses received outside an active request are ignored.

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
Phase 2B.6    PASS 🔒
Phase 2B.7    PASS 🔒
```

The current implementation has been tested using:

* 1 × Base
* 2 × Nodes
* 2 × SHT41 sensors

The current hardware test is limited to two Nodes.

The software architecture uses logical Node IDs and can be extended when additional hardware becomes available.

---

## Next Phase

**PHASE 2B.8 - Communication Reliability & Edge Cases**

The next phase will first be defined after reviewing the complete Phase 2B protection chain.

Potential areas for investigation include:

* ESP-NOW transmission failure handling.
* Unexpected or corrupted communication events.
* Repeated timeout behaviour.
* Boundary conditions around the 200 ms response timeout.
* Sequence number wrap-around.
* Additional communication edge cases.

No implementation is defined for Phase 2B.8 until the requirements are reviewed and confirmed.

---

## Phase 2B Objective

Phase 2B progressively strengthens the reliability of WeatherBus communication.

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
      ↓
Response State Validation
      ↓
Node Recovery
```

Each layer is independently tested before moving to the next phase.

**Phase 2B.7 is functionally validated and LOCKED.**
