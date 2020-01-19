/**
 * This example ilustrates how to use the debouncer in a completely asynchronous way, as it was intended to be used:
 * 
 * - It uses Pin Change Interrupts (via YetAnotherPcInt library) to capture changes on digital pins 
 * - It uses a scheduler (via DeepSleepScheduler) to continue it's processing after the predefined delay has elapsed
 */

#define DEEP_SLEEP_DELAY 10
#define AWAKE_INDICATION_PIN 13

#include <DeepSleepScheduler.h>
#include <YetAnotherPcInt.h>
#include <YetAnotherDebouncer.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))



class AsyncDebouncer: public Runnable {
private:
    virtual void run() {
        //The debouncer requires external synchronization so that we don't 
        //call debouncer.flush() and debouncer.set_value() at the same time,
        noInterrupts(); 
        int delay = debouncer.flush();
        interrupts();
        if (delay >= 0) {
            scheduler.scheduleDelayed(this, delay);
        }
        ::delay(100);
    }
  
    static void changed(bool value, AsyncDebouncer* _this) {
        //This is called from debouncer.flush(), but no synchronization is required here
        interrupts();
        _this->changed(value);
        noInterrupts();
    }
  
    static void pcint(AsyncDebouncer* _this, bool value) {
        //We're in an ISR , therefore we use 'flush=false' to indicate that the callback cannot be triggered right now
        int delay = _this->debouncer.set_value(value, false);
        scheduler.removeCallbacks(_this);
        if (delay >= 0) {
            scheduler.scheduleDelayed(_this, delay);
        }
    }

protected:
    int pin;
    Debouncer<Clock::Millis> debouncer;

public:
    void begin(int pin, int debounce=0) {
        this->pin = pin;
        pinMode(pin, INPUT_PULLUP);
    
        debouncer.begin(digitalRead(pin));
        debouncer.set_debounce_stable(debounce);
        debouncer.set_callback(AsyncDebouncer::changed, this);
          
        PcInt::attachInterrupt(pin, AsyncDebouncer::pcint, this, CHANGE, true);    
    }
  
    void end() {
        PcInt::detachInterrupt(pin);
        debouncer.end();
    }  
  
    virtual void changed(bool value) = 0;
};



class SerialAsyncDebouncer: public AsyncDebouncer {
    virtual void changed(bool value) {
        Serial.print("Pin #");
        Serial.print(this->pin);
        Serial.print(" changed to ");
        Serial.println(value? "OFF" : "ON");
    }
};



int input_pins[] = {A8, A9, A10, A11, A12, A13, A14, A15};
SerialAsyncDebouncer debouncers[ARRAY_SIZE(input_pins)];



void setup() {
    Serial.begin(115200);
  
    for (int i=0; i<ARRAY_SIZE(input_pins); i++) {
        int debounce = i*i*25;
        debouncers[i].begin(input_pins[i], debounce);

        Serial.print("Pin #");
        Serial.print(input_pins[i]);
        Serial.print(" initialized with ");
        Serial.print(debounce);
        Serial.println("ms of debounce"); 
    }
}



void loop() {
    scheduler.execute();
}


