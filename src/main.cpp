#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

// Constants.
// ----------------------------------------------------------------------------

const int PORT = 5555;

const int PIN_BLUE = D1;
const int PIN_RED = D2;
const int PIN_GREEN = D3;
const int PIN_WIFI = D4;
const int PIN_WPS = D5;

// State.
// ----------------------------------------------------------------------------
bool turnedOn;
float red, green, blue;

// Setting up.
// ----------------------------------------------------------------------------

WiFiUDP udp;
WiFiEventHandler wifiConnectedHandler, wifiGotIpHandler, wifiDisconnected;

void startWPS() {
    // Cannot be used in an interrupt handler because of some bug in WiFi class.
    Serial.println("Disconnecting from Wi-Fi.");
    WiFi.disconnect();
    Serial.println("Starting WPS.");
    if (!WiFi.beginWPSConfig()) {
        Serial.println("Failed to start WPS.");
    }
}

void setupPins() {
    pinMode(PIN_WIFI, OUTPUT);
    digitalWrite(PIN_WIFI, HIGH);

    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_BLUE, OUTPUT);

    // WPS button.
    pinMode(PIN_WPS, INPUT_PULLUP);
}

void setupSerial() {
    Serial.begin(9600);
    Serial.println();
    Serial.println("Serial is ready.");
}

void setupState() {
    // TODO: read state from the FS.
    turnedOn = true;
    red = green = blue = 1.0;
    analogWrite(PIN_RED, 1023);
    analogWrite(PIN_GREEN, 1023);
    analogWrite(PIN_BLUE, 1023);
}

void setupFS() {
    Serial.print("Setting up file system: ");
    Serial.println(SPIFFS.begin() ? "ok." : "failed.");
}

void setupWiFi() {
    wifiConnectedHandler = WiFi.onStationModeConnected([](const WiFiEventStationModeConnected& event) {
        Serial.println("Connected to Wi-Fi.");
    });
    wifiGotIpHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event) {
        Serial.print("Got IP: ");
        Serial.println(WiFi.localIP());
        digitalWrite(PIN_WIFI, LOW);
    });
    wifiDisconnected = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event) {
        Serial.println("Disconnected from Wi-Fi.");
        digitalWrite(PIN_WIFI, HIGH);
    });

    Serial.println("Connecting to Wi-Fi.");
    WiFi.mode(WIFI_STA);
    WiFi.begin();
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
    Serial.print("Starting UDP: ");
    Serial.println(udp.begin(PORT) ? "ok." : "failed.");
}

void setup() {
    setupPins();
    setupSerial();
    setupState();
    setupFS();
    setupWiFi();
    setupMDNS();
    setupUDP();
}

// Running.
// ----------------------------------------------------------------------------

void sendPacket(JsonObject& root, IPAddress ip, uint16_t port) {
    udp.beginPacket(ip, port);
    root.printTo(udp);
    udp.endPacket();
}

void sendResponse(JsonObject& message) {
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject& response = jsonBuffer.createObject();
    response["mid"] = message.get<int>("messageId");
    response["ms"] = millis();
    response["t"] = "MULTICOLOR_LIGHTING";
    response["id"] = WiFi.hostname();
    response["name"] = WiFi.hostname();  // TODO: make customizable.
    response["r"] = red;
    response["g"] = green;
    response["b"] = blue;
    response["on"] = turnedOn;
    sendPacket(response, udp.remoteIP(), udp.remotePort());
}

void handleSetColor(JsonObject& message) {
    turnedOn = true;

    red = message.get<float>("r");
    green = message.get<float>("g");
    blue =  message.get<float>("b");

    analogWrite(PIN_RED, PWMRANGE * red);
    analogWrite(PIN_GREEN, PWMRANGE * green);
    analogWrite(PIN_BLUE, PWMRANGE * blue);
}

void handleTurnOn() {
    turnedOn = true;

    analogWrite(PIN_RED, PWMRANGE * red);
    analogWrite(PIN_GREEN, PWMRANGE * green);
    analogWrite(PIN_BLUE, PWMRANGE * blue);
}

void handleTurnOff() {
    turnedOn = false;

    analogWrite(PIN_RED, 0);
    analogWrite(PIN_GREEN, 0);
    analogWrite(PIN_BLUE, 0);
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

    JsonVariant type = message["t"];
    if (!type.success()) {
        Serial.println("Received invalid message type.");
        return;
    }

    if (type == "PING") {
        // Do nothing.
    } else if (type == "SET_COLOR") {
        handleSetColor(message);
    } else if (type == "TURN_ON") {
        handleTurnOn();
    } else if (type == "TURN_OFF") {
        handleTurnOff();
    } else {
        return;
    }

    sendResponse(message);
}

void handleWPS() {
    if (digitalRead(PIN_WPS) == LOW) {
        delay(100);
        startWPS();
    }
}

void loop() {
    handlePacket();
    handleWPS();
}
