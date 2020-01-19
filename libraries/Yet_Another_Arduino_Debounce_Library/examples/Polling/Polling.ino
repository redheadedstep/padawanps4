/**
 * This example ilustrates how to use the debouncer in a pooled way:
 * - You call debouncer.set_value() often as you can, and it triggers the callbacks once the value is stable
 */
#include <YetAnotherDebouncer.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))



void debounce_callback(bool value, int* i) {
    Serial.print("Button #");
    Serial.print(*i);
    Serial.print(" changed to ");
    Serial.println(value? "OFF" : "ON");
}


int input_pins[] = {A8, A9, A10, A11, A12, A13, A14, A15};
Debouncer<Clock::Millis> debouncers[ARRAY_SIZE(input_pins)];



void setup() {
    Serial.begin(115200);
    for (int i=0; i<ARRAY_SIZE(input_pins); i++) {
        int debounce = i*i*25;
        
        pinMode(input_pins[i], INPUT_PULLUP);
        debouncers[i].begin();
        debouncers[i].set_debounce_stable(debounce);
        debouncers[i].set_callback(debounce_callback, &input_pins[i]);

        Serial.print("Pin #");
        Serial.print(input_pins[i]);
        Serial.print(" initialized with ");
        Serial.print(debounce);
        Serial.println("ms of debounce"); 
    }
}



void loop() {
    // Refresh the value, and once the value has been stable long enough it will automatically triggers the callback
    for (int i=0; i<ARRAY_SIZE(input_pins); i++) {
        debouncers[i].set_value(digitalRead(input_pins[i]));
    }
}

