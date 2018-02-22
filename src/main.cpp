#include <Arduino.h>
#include <ArduinoJson.h>

#include "../lib/Robocluster.h"

StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();

void send_message() {
    root.printTo(Serial);
}

void setup() {
    cli();//stop interrupts

    TCCR1A = 0;// set entire TCCR1A register to 0
    TCCR1B = 0;// same for TCCR1B
    TCNT1  = 0;//initialize counter value to 0
    // set compare match register for 1hz increments
    OCR1A = 3124;// = (16*10^6) / (5*1024) - 1 (must be <65536)
    // turn on CTC mode
    TCCR1B |= (1 << WGM12);
    // Set CS10 and CS12 bits for 1024 prescaler
    TCCR1B |= (1 << CS12) | (1 << CS10);  
    // enable timer compare interrupt
    TIMSK1 |= (1 << OCIE1A);

    sei();//allow interrupts

    pinMode(13, OUTPUT);
    Serial.begin(115200);
    root["event"] = "test";
    JsonObject& nested = root.createNestedObject("data");
    nested["key1"] = 42;
    root["data"] = nested;
}

ISR(TIMER1_COMPA_vect){//timer1 interrupt 5Hz/200ms
  heartbeat();
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
