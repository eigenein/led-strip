#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <ESP8266TrueRandom.h>

// Constants.
// ----------------------------------------------------------------------------

const int PORT = 5555;

const int PIN_RED = D1;
const int PIN_GREEN = D2;
const int PIN_BLUE = D3;
const int PIN_WPS = D5;

// Setting up.
// ----------------------------------------------------------------------------

WiFiUDP udp;
WiFiEventHandler wifiConnectedHandler, wifiGotIpHandler, wifiDisconnected;
String uuid;

void startWPS() {
    // WPS button interrupt handler.
    Serial.println("Starting WPS.");
    WiFi.beginWPSConfig();
}

void setupPins() {
    // Built-in LED displays Wi-Fi connection status.
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_BLUE, OUTPUT);

    // WPS button.
    pinMode(PIN_WPS, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_WPS), startWPS, FALLING);
}

void setupSerial() {
    Serial.begin(9600);
    Serial.println();
    Serial.println("Serial is ready.");
}

void setupStrip() {
    // TODO: read state from the FS.
    analogWrite(PIN_RED, 128);
    analogWrite(PIN_GREEN, 128);
    analogWrite(PIN_BLUE, 128);
}

void setupFS() {
    Serial.print("Setting up file system: ");
    Serial.println(SPIFFS.begin() ? "ok." : "failed.");
}

void setupUUID() {
    File uuidFile = SPIFFS.open("/uuid.txt", "r");
    if (uuidFile) {
        Serial.print("Reading UUID: ");
        uuid = uuidFile.readStringUntil('\n');
    } else {
        Serial.print("Generating UUID: ");
        byte uuidBuffer[16];
        ESP8266TrueRandom.uuid(uuidBuffer);
        uuid = ESP8266TrueRandom.uuidToString(uuidBuffer);
        uuidFile = SPIFFS.open("/uuid.txt", "w");
        uuidFile.println(uuid);
    }
    uuidFile.close();
    Serial.println(uuid);
}

void setupWiFi() {
    wifiConnectedHandler = WiFi.onStationModeConnected([](const WiFiEventStationModeConnected& event) {
        Serial.println("Connected to Wi-Fi.");
    });
    wifiGotIpHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event) {
        Serial.print("Got IP: ");
        Serial.println(WiFi.localIP());
        digitalWrite(LED_BUILTIN, LOW);
    });
    wifiDisconnected = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event) {
        Serial.println("Disconnected from Wi-Fi.");
        digitalWrite(LED_BUILTIN, HIGH);
    });

    Serial.println("Connecting to Wi-Fi.");
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
    setupStrip();
    setupFS();
    setupUUID();
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

void sendResponse(JsonObject& message) {
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject& response = jsonBuffer.createObject();
    response["messageId"] = message.get<int>("messageId");
    response["millis"] = millis();
    response["deviceType"] = "RGB";
    response["uuid"] = uuid;
    sendPacket(response, udp.remoteIP(), udp.remotePort());
}

void handlePing(JsonObject& message) {
    sendResponse(message);
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

    if (type == "ping") {
        handlePing(message);
    } else if (type == "setColor") {
        handleSetColor(message);
    }
}

void loop() {
    handlePacket();
    yield();
}
