#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

#include "settings.h"

// Constants.
// ----------------------------------------------------------------------------

const int PORT = 5555;

const int PIN_RED = D1;
const int PIN_GREEN = D2;
const int PIN_BLUE = D3;

// Setting up.
// ----------------------------------------------------------------------------

WiFiUDP udp;
WiFiEventHandler wifiConnectedHandler, wifiGotIpHandler, wifiDisconnected;

void setupPins() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_BLUE, OUTPUT);
}

void setupSerial() {
    Serial.begin(9600);
    Serial.println();
    Serial.println("Serial is ready.");
}

void setupWiFi() {
    wifiConnectedHandler = WiFi.onStationModeConnected([](const WiFiEventStationModeConnected& event) {
        Serial.println("Connected to Wi-Fi.");
    });
    wifiGotIpHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event) {
        Serial.print("Got IP: ");
        Serial.println(WiFi.localIP());
    });
    wifiDisconnected = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event) {
        Serial.println("Disconnected from Wi-Fi.");
    });
    WiFi.begin(SSID, KEY);
}

void setupMDNS() {
    Serial.print("Setting up mDNS. Hostname: ");
    Serial.println(WiFi.hostname());
    if (MDNS.begin(WiFi.hostname().c_str())) {
        Serial.println("Registering mDNS service.");
        MDNS.addService("smart-home", "udp", PORT);
    } else {
        Serial.println("mDNS responder error.");
    }
}

void setupUDP() {
    if (udp.begin(PORT)) {
        Serial.println("UDP started.");
    } else {
        Serial.println("UDP start error.");
    }
}

void setup() {
    setupPins();
    setupSerial();
    setupWiFi();
    setupMDNS();
    setupUDP();
}

// Running.
// ----------------------------------------------------------------------------

void sendPacket(JsonObject& root, IPAddress ip, uint16_t port) {
    udp.beginPacket(ip, port);
    root.printTo(udp);
    udp.println();
    udp.endPacket();
}

void handlePing(JsonObject& message) {
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject& response = jsonBuffer.createObject();
    response["messageId"] = message.get<int>("messageId");
    response["millis"] = millis();
    response["deviceType"] = "LED";
    response["uuid"] = UUID;
    sendPacket(response, udp.remoteIP(), udp.remotePort());
}

void handleSetColor(JsonObject& message) {
    analogWrite(PIN_RED, message.get<int>("red"));
    analogWrite(PIN_GREEN, message.get<int>("green"));
    analogWrite(PIN_BLUE, message.get<int>("blue"));
}

void handlePacket() {
    if (udp.parsePacket() == 0) {
        return;
    }

    StaticJsonBuffer<512> jsonBuffer;
    JsonObject& message = jsonBuffer.parse(udp);
    if (!message.success()) {
        Serial.println("Received invalid JSON message.");
        return;
    }

    JsonVariant type = message["type"];
    if (!type.success()) {
        Serial.println("Received invalid message type.");
        return;
    }

    if (type == F("ping")) {
        handlePing(message);
    } else if (type == F("setColor")) {
        handleSetColor(message);
    }
}

void loop() {
    handlePacket();
}
