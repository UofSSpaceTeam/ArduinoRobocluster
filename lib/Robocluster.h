#ifndef ROBOCLUSTER
#define ROBOCLUSTER

#include "Robocluster.cpp"
#include <map>
#include <list>
#include <ArduinoJson.h>

/*
 * =============== Data Types ===============
 */

/*
 * Wraps a funcion and keeps track of meta data associated
 * with that function. Tasks can be scheduled to be run
 * in the future, or be triggered as a callback for an event.
 */
class Task {
public:

    Task(void (*run_callback)(void),
            unsigned long int call_time_in, unsigned long int interval_in);

    /* Constructor assigning 0 to call_time and interval. */
    Task(void (*run_callback)(void));

    /* Pointer to a function to run. Function should return void,
     * and take no arguments.
     */
    void (*run)(void);

    /* The time in miliseconds that this task should be called at.
     * This is not a deadline, and the actual time this is called
     * could be after call_time. A call_time of 0 indicates a task
     * that should be run as soon as possible.
     */
    unsigned long int call_time;

    /* The time interval in milliseconds that
     * this task will be repeatedly called.
     * An interval of 0 indicates a task that does not repeat.
     */
    unsigned long int interval;

};

/* How to format the data. JSON for now,
 * maybe we'll add VESC later.
 */
typedef enum {JSON, VESC} Encoding;

// /* A transport protocol such as Serial, I2C, SPI, etc.
//  * Abstract class, so you have to create subclasses for
//  * Serial, I2C, implementations.
//  * I'm also not sure how we want to go about instanciating ports.
//  */
// class Port {
// public:
//     virtual byte *read();
//     virtual void write(byte *data);
//     bool enabled; // public to make things simple, might add getters/setters in future
//     Encoding encoding = JSON; // JSON by default
// };

// /* This one will need an actuall implementation. */
// class SerialPort: public Port{
// public:
//     byte *read();
//     void write(byte *data);
// };

/* Event struct */
struct Event{
    char *event; // The event name/identifier.
    JsonObject& data;
    /* maybe the port it was received on? */
};

/* A callback is a function that takes in an event,
 * and returns nothing. Not 100% sure if e should be a pointer or not.
 */
typedef void (*callback)(struct Event *e);


/*
 * ========== Global Variables ============
 */

/* The global device name to be used to identify it
 * on the robocluster network, and to be prepended on
 * any events it publishes.
 */
char *g_device_name;

/* Callback look up table (lut).
 * Maps the event string to a callback function.
 */
std::map<String, callback> g_callback_lut;

/* A list of tasks to run in the event loop.
 * This should be sorted by Task.call_time, so that the first items
 * are the ones that should be called the soonest.
 */
std::list<Task> g_tasks;

/*
 * ============== Library Functions =============
 */

/* Set the global device name to be used to identify it
 * on the robocluster network, and to be prepended on
 * any events it publishes.
 */
void set_device_name(char *name);

/* Add a task to the event loop (add to g_tasks)*/
void add_task(Task t);

/* run Task t imediately when an event occurs
 * Events are identified with a URI structure such as this:
 * "device-1/event1", "device-2/gps/position"
 */
void on_event(char *event, callback cb);


/* Run any events in the event loop.
 * Here is some psuedo python of how I imagined this would work:
 * while True:
 *    if millis() > g_tasks.first().call_time:
 *       # The next task is due to be run.
 *       t = g_tasks.pop();
 *       t.run();
 *       if t.interval > 0:
 *          # add back into the loop if it is a repeating task
 *          add_task(t)
 */
void run_event_loop();

/* Send "data" under "event" on the desired port.
 * This will have to wrap the event and data into
 * an Event like JSON object before sending over the wire.
 */
void publish(String event, JsonObject& data, Port port);

#endif
