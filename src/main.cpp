#include <Arduino.h>
#include <ArduinoJson.h>

#include "../lib/Robocluster.h"

StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();

void send_message() {
    root.printTo(Serial);
}

void setup() {
    pinMode(13, OUTPUT);
    Serial.begin(115200);
    root["event"] = "test";
    JsonObject& nested = root.createNestedObject("data");
    nested["key1"] = 42;
    root["data"] = nested;
}

void loop() {
    send_message();
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    delay(500);
    // while(!Serial.available()){}
    // Serial.read();
}
