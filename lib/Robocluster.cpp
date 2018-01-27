#include <iostream>
#include "Robocluster.h" //include header file
#include <map>
#include <list>
#include <ArduinoJson.h>


char *g_device_name, sender, type, data;
void run_event_loop();
void add_task(Task t);
void set_device_name(char *name);

/* CLASSES */
class Task{ //how task is added 
    public:
        unsigned long int call_time, interval;
        void (*run)(void) //pointer to function to run, function should return void

        //constructor with all arguments
        Task(void (*run_callback)(void), unsigned long int call_time, 
            unsigned long int interval);

        //constructor with no arguments
        Task(void (*run_callback)(void));
}

Task::Task(void (*run_callback)(void)), unsigned long int call_time_in, unsigned long int interval_in){
    run = run_callback;
    call_time = call_time + millis(); //take into account time after program start
    interval = interval;  
}

Task::Task(void (*run_callback)(void)){
    run = run_callback;
    call_time = millis();
    interval = 0;    
}


/* LIBRARY FUNCTIONS */
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

//add task back onto end of event loop
void add_task(Task t){
    unsigned char index = 0;
    bool inserted = false;

    for (std::list<Task>::const_iterator iterator = g_tasks.begin(); iterator != g_tasks.end(); ++iterator){
        Task& iter = *iterator;
        if (t.call_time > iter.call_time){
            g_tasks.insert (index, t);
            inserted = true;
            break;
        }
        index ++; //increment with each loop
    }
    if (inserted == false){ //if the task wasn't inserted after the loop ended it must have the largest call time
        g_tasks.push_back(t);
    }
}

void set_device_name(char *name){
    g_device_name = name;
}


/* SERIAL PORT */
void serialEvent(){
    noInterrupts();
    unsigned char stack = 0;
    bool data_Json = false; 
    StaticJsonBuffer<200> jsonBuffer;
    if(Serial.available()){
        for (int i = 0; i < 200; i++){
            jsonBuffer[i] = Serial.read();
            if (stack == 2){
                data_Json = true; //if there's a nested json packet in data
            }
            else if (jsonBuffer[i] == '{'){
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
    if (stack == 0){ //make sure the data is valid
        JsonObject& root = jsonBuffer.parseObject(json);
        source = root["source"];
        type = root["type"]; 
        data1 = root["data"];
        if (data_Json == true){
            JsonObject& data = data.parseObject(json);
            topic_data = data["topic"];
            data2 = data["data"];
        }
    }
    interrupts(); //re-enable interrupts
}

