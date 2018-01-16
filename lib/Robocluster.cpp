#include <iostream>
#include "Robocluster.h" //include header file
#include <map>
#include <list>
#include <ArduinoJson.h>
using namespace std;


// note that each int is 4 bytes, usually want 8 bytes

/* This is a function pointer, *run is name of pointer */
class Task{
    private:
        

    public:
        unsigned long int call_time, interval;
        void (*run)(void) //pointer to function to run, function should return void

        //constructor with all arguments
        Task(void (*run)(void),
            unsigned long int call_time, unsigned long int interval){
            call_time = call_time;
            interval = interval;
        }

        //constructor with no arguments
        Task(void (*run)(void)){
            call_time = 0; //assume it's a 1 time call
            interval = 0;
        }
}



