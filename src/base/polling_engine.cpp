#include "polling_engine.h"
#include <WiFi.h>
#include <esp_now.h>

#include "../common/weatherbus_protocol.h"

using namespace WeatherBus;

PollingEngine::PollingEngine(NodeManager& manager)
    : nodeManager(manager) {
    state = State::IDLE;
    currentNodeIndex = 0;
    sequenceNumber = 0;
    stateTimestamp = 0;
    requestTimestamp = 0;
}

void PollingEngine::begin() {
    state = State::IDLE;
    stateTimestamp = millis();
    Serial.println("Polling Engine ready");
}

void PollingEngine::update() {
    uint32_t now = millis();
    switch (state) {
    case State::IDLE:
        if (now - stateTimestamp >= POLL_INTERVAL_MS) {
            state = State::SEND_REQUEST;
        }
        break;
    case State::SEND_REQUEST:
        sendRequest();
        break;
    case State::WAIT_RESPONSE:
        if (now - requestTimestamp >= RESPONSE_TIMEOUT_MS) {
            handleTimeout();
        }
        break;
    }
}

void PollingEngine::sendRequest() {
    NodeInfo* node = nodeManager.getNode(currentNodeIndex);
    if (node == nullptr) {
        moveToNextNode();
        return;
    }
    Header request{};
    request.protocolVersion = PROTOCOL_VERSION;
    request.packetType = static_cast<uint8_t>(PacketType::DATA_REQUEST);
    request.nodeId = node->nodeId;
    request.flags = 0;
    request.sequence = ++sequenceNumber;
    request.payloadLength = 0;

    // CRC disabled in Phase 2A
    request.crc16 = 0;
    request.reserved = 0;

    esp_err_t result = esp_now_send(node->mac, reinterpret_cast<uint8_t*>(&request), sizeof(request));
    if (result != ESP_OK) {
        Serial.printf("TX ERROR: Node %u | ESP-NOW error %d\n", node->nodeId, result);
        handleTimeout();
        return;
    }

    requestTimestamp = millis();
    state = State::WAIT_RESPONSE;
    Serial.printf("TX: DATA_REQUEST -> Node %u | Sequence %u\n", node->nodeId, request.sequence);
}

void PollingEngine::onSensorData(
    uint8_t nodeId,
    uint16_t sequence,
    float temperature,
    float humidity) {

    NodeInfo* node = nodeManager.getNodeById(nodeId);

    if (node == nullptr) {
        Serial.printf("RX: Unknown Node %u\n", nodeId);
        return;
    }

    // -------------------------------------------------
    // Only accept response from current node
    // -------------------------------------------------
    if (node->nodeId != nodeManager.getNode(currentNodeIndex)->nodeId) {
        Serial.printf("RX: Unexpected Node %u\n", nodeId);
        return;
    }

    nodeManager.markOnline(nodeId);
    uint32_t responseTime = millis() - requestTimestamp;
    Serial.println();
    Serial.println("========== WEATHERBUS RX ==========");
    Serial.printf("Node ID      : %u\n", nodeId);
    Serial.printf("Sequence     : %u\n", sequence);
    Serial.printf("Temperature  : %.2f C\n", temperature);
    Serial.printf("Humidity     : %.2f %%\n", humidity);
    Serial.printf("Response Time: %lu ms\n", responseTime);
    Serial.println("===================================");
    moveToNextNode();
}

void PollingEngine::handleTimeout() {
    NodeInfo* node = nodeManager.getNode(currentNodeIndex);
    if (node != nullptr) {
        nodeManager.markOffline(node->nodeId);

        Serial.printf("TIMEOUT: Node %u did not respond within %lu ms\n", node->nodeId, RESPONSE_TIMEOUT_MS);
    }
    moveToNextNode();
}

void PollingEngine::moveToNextNode() {
    currentNodeIndex++;
    if (currentNodeIndex >= nodeManager.getNodeCount()) {
        currentNodeIndex = 0;
    }

    stateTimestamp = millis();
    state = State::IDLE;
}