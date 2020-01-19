#pragma once

namespace Clock {
    struct Micros {
        static inline int32_t now() {
            return micros();
        }
    };
    struct Millis {
        static inline int32_t now() {
            return millis();
        }
    };
}

template<typename clock=Clock::Millis, typename value_t=int>
class Debouncer {
    typedef decltype( clock::now() ) time_t;
    typedef void (*debounce_callback)(value_t value, void* param);
    typedef void (*debounce_callback_noarg)(value_t value);

private:
    debounce_callback callback;
    void* callback_param;

    value_t stable_value;
    value_t last_value;

    time_t  debounce_before;  // Amount of time the value must be stable before triggering an event
    //time_t  debounce_after;   // Amount of time after a transition before the next event can be triggered
    bool    debounce_pending; // Indicates there is a pending event that will need to be pooled later
    time_t  debounce_at;      // if there is a pending event, when it should be triggered

    inline void bouncy_until(time_t until) {
        if (debounce_pending ? time_cmp(until, debounce_at) > 0 : true) {
            debounce_at = until;
            debounce_pending = true;
        }
    }

    time_t wait_time(time_t now) {
        if (debounce_pending) {
            return (debounce_at-now);
        } else {
            return -1;
        }
    }

    time_t flush(time_t now) {
        // Nothing to poll, or has to wait a bit longer
        if (!debounce_pending || time_cmp(debounce_at, now) > 0) {
            return wait_time(now);
        }

        // --- The event will finally be triggered! ---

        // clear the pending flag
        debounce_pending = false;

        // Haha, don't actually trigger an event if the stable value hasn't changed
        if (last_value != stable_value) {
            stable_value = last_value;
            if (callback) {
                callback(stable_value, callback_param);
            }
        }

        return wait_time(now);
    }

    time_t set_value(value_t value, bool flush, time_t now) {
        if (flush) this->flush(now); //Flush pending events

        if (value == last_value) { //Ignore no-ops
            return;
        }

        last_value = value;
        bouncy_until(now+debounce_before);

        if (flush && debounce_before == 0) {
            this->flush();
        }

        return wait_time(now);
    }

    inline int time_cmp(time_t a, time_t b) {
        // We must keep an eye open for wrap-around time.
        // e.g., an 32-bit microsecond-resolution timer will wrap every 70 minutes

        // It's important that the wrap time is (considerably) larger than the debounce time and the pooling interval for
        // this time comparisson to work reliably.

        // THIS WILL ONLY WORKS FOR SIGNED time_t
        time_t delta = a-b;
        return (delta>0) ? 1 : (delta<0) ? -1 : 0;
    }

    static hack_callback_noarg(value_t value, void* param) {
        ((debounce_callback_noarg)param)(value);
    }

public:
  /**
    * Start things up
    */
    void begin(value_t initial_value=value_t()) {
        this->debounce_pending = false;
        this->stable_value = initial_value;
        this->last_value = initial_value;
    }


    void end() {
        //HAHA! Nothing to do here!
    }


    /**
     * Attaches a Debounced state change listener
     * @param func The callback function to be called whenever the (debounced) value changes.
     * @param param Argument sent to the callback with whatever context you may find useful
     */
    template<typename T> void set_callback(void (*func)(value_t, T*), T* param=nullptr) {
        callback = (debounce_callback)func;
        callback_param = param;
    }

        //Attaches a Debounced state change listener
    //If you have a dettachable device, add pull down resistors to both data lines, otherwise random noise will produce lots of bogus State Change notifications (and a few Data Received notifications)
    void set_callback(debounce_callback_noarg func) {
        callback = hack_callback_noarg;
        callback_param = func;
    }

    /**
     * Set debounce interval.
     * This kind of debounce adds a small delay before the callback is triggered, and will ignore very short button presses as noise
     * @param debounce Amount of time the value must be stable before triggering an event
     */
    void set_debounce_stable(time_t debounce) {
        this->debounce_before = debounce;
        //this->debounce_after = 0;
    }

    /**
     * Set debounce interval
     * This kind of debounce triggers short button presses immediately, ignoring subsequent transitions as noise
     * @param debounce Amount of time after a transition before the next event can be triggered
     *
     * FIXME: Unsupported for now
    void set_debounce_fast(time_t debounce) {
        this->debounce_before = 0;
        this->debounce_after = debounce;
    }*/

    /* *
     * Set debounce interval
     * @param debounce_before Amount of time the value must be stable before triggering an event
     * @param debounce_after  Amount of time after a transition before the next event can be triggered
     * /
     *
     * FIXME: Unsupported for now
    void set_debounce(time_t debounce_before, time_debounce_after) {
        this->debounce_before = debounce_before;
        this->debounce_after = debounce_after;
    }*/

    /**
     * Return amount of time you should wait before pooling this debouncer again (unless a state change happens, of course)
     *
     * You are free to ignore the returned value and pool inside your main loop.
     *
     * @return The amout of time to wait before boncy-ness ends, -1 if nothing is bouncing
     */
    time_t wait_time() {
        return wait_time(clock::now());
    }

    /**
     * Sets the (bouncy) value of the input
     *
     * This might trigger the event callback.
     *
     * @param value The new value of the (bouncy) input
     * @return The amout of time you should wait before calling flush()
     */
    time_t set_value(value_t value, bool flush=true) {
        return set_value(value, flush, clock::now());
    }

    /**
     * Flush any delayed event.
     *
     * This might trigger the event callback.
     *
     * @return The amout of time to wait before boncy-ness ends, -1 if nothing is bouncing
     */
    time_t flush() {
        return flush(clock::now());
    }

};
