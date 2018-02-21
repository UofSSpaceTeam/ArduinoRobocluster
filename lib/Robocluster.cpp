#include <iostream>
#include "Robocluster.h" //include header file
#include <map>
#include <list>
#include <ArduinoJson.h>


char *sender, *type, *data;

std::map<String, callback> g_callback_lut;
std::list<Task> g_tasks;


Task::Task(void (*run_callback)(void)), unsigned long int call_time_in, unsigned long int interval_in){
    run = run_callback;
    if (call_time_in){
        run_callback();
    }
    else{
        call_time = call_time_in + millis(); //take into account time after program start
    }
    interval = interval_in;  
}

Task::Task(void (*run_callback)(void)){
    run_callback(); //just run it, assume non repeating
    //run = run_callback;
    //call_time = millis();
    //interval = 0;
}


/* LIBRARY FUNCTIONS */
/* Event struct */
struct Event{
    char *event; // The event name/identifier. TOPIC 
    char& data;  // Could also be a JsonObject??
    // JsonObject& data; 
    /* maybe the port it was received on? */
};

void set_device_name(char *name){
    g_device_name = name;
}

//add task back onto end of event loop
void add_task(Task t){
    unsigned char index = 0;
    bool inserted = false;

    // how to iterate through a list, can't normally find index of a list 
    for (std::list<Task>::const_iterator iterator = g_tasks.begin(); iterator != g_tasks.end(); ++iterator){ 
        Task& iter = *iterator; //temporary variable
        if (t.call_time > iter.call_time){
            g_tasks.insert (index, t);
            inserted = true;
            break;
        }
        index++; //increment with each loop
    }
    if (inserted == false){ //if the task wasn't inserted after the loop ended it must have the largest call time
        g_tasks.push_back(t);
    }
}

//populate map
void on_event(char *event, callback cb){ //pass in topic for event
    g_callback_lut["event"] = &cb; //adding to the map
}

// Run any events in the event loop.
void run_event_loop(){
    while(1){
        if (millis() > g_tasks.front().call_time ) {
            Task t = g_tasks.front();
            g_tasks.pop_front(); //delete front
            t.run();
            if (t.interval > 0){
                t.call_time = t.interval + millis(); //call again in interval seconds
                add_task(t);
            }
        }
    }
}


void publish(String event, JsonObject& data){
    string slash = '/' //need to use for topic field
    StaticJsonBuffer<200> jsonBuffer_publish;
    JsonObject& root = jsonBuffer_publish.createObject();

    root["source"] = g_device_name;
    root["type"] = "publish";
    JsonArray& data_nested = root.createNestedObject("data");
    data_nested["topic"] = g_device_name + slash + event; 
    data_nested["data"] = data;

    root.printTo(Serial); //pass into serial
}

void heartbeat(){
    StaticJsonBuffer<200> jsonBuffer_heartbeat;
    JsonObject& root = jsonBuffer_heartbeat.createObject();

    root["source"] = g_device_name;
    root["type"] = "heartbeat";
    JsonArray& data_nested = root.createNestedObject("data");
    data_nested["source"] = g_device_name; 
    data_nested["listen"] = "/dev/ttyACM0"; 
    //I don't actually know what the port field should be so this is a wild guess

    root.printTo(Serial); //pass into serial
}

/* SERIAL PORT 
Note: This handler isn't coded in the best way, too much processing inside of it
*/
void serialEvent(){
    noInterrupts();
    uint8_t stack = 0; //how to check if packet is complete, shouldn't need too many bits
    char source, type, data, published_topic, published_data, heartbeat_source, heartbeat_listen; 
    StaticJsonBuffer<200> jsonBuffer;

    if(Serial.available()){
        for (int i = 0; i < 200; i++){ //read json data
            jsonBuffer[i] = Serial.read();
            if (jsonBuffer[i] == '{'){
                stack++;
            }
            else if (jsonBuffer[i] == '}'){
                stack--;
            }
            else if (stack == 0){
                break;
            }
        }
    }
    if (stack == 0){ //process data, make sure it's valid
        JsonObject& root = jsonBuffer.parseObject(json);
        if (!root.success()) {
            break; //should probably also send a message saying it failed
        }   
        else {
            source = root["source"];
            type = root["type"]; 
            data = root["data"];

            if (data == "publish"){ //call the callback function
                JsonObject& publish = data.parseObject(json);
                if (!publish.success()){
                    break; //should probably also send a message saying it failed
                }
                else{
                    published_topic = publish["topic"]; //call g_callback_lut here
                    published_data = publish["data"];

                    Event packet_data; //make structure to put into callback
                    packet_data.event = published_topic;
                    packet_data.data = published_data;

                    if (g_callback_lut.find(packet_data.event) == g_callback_lut.end()){ //look for event key
                        break; //no key exists for said event data
                    }
                    else {
                        g_callback_lut[packet_data.event](data); //key is found, run
                    }
                }
            }
            else if (data == "heartbeat"){
                JsonObject& heartbeat = heartbeat.parseObject(json);
                if (!heartbeat.success()){
                    break;
                }
                else{
                    heartbeat_source = heartbeat["source"];
                    heartbeat_listen = heartbeat["listen"];
                    //need to do something with heartbeat data
                }
            }
        }     
    }
    interrupts(); //re-enable interrupts
}

