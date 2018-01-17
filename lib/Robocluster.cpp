#include <iostream>
#include "Robocluster.h" //include header file
#include <map>
#include <list>
#include <ArduinoJson.h>

class Task{
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
    call_time = call_time;
    interval = interval;  
}

Task::Task(void (*run_callback)(void)){
    run = run_callback;
    call_time = 0;
    interval = 0;    
}

class Port{
    private:

    public:

}


