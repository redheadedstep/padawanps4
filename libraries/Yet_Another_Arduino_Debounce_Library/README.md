# Yet Another Arduino Debounce Library

Everyone someday implemented debouncing logic within the main loop.

But, as programs become more complicated and lots of tasks have to be executed, this approach doesn't really scale.

This library is my take on the debouncing problem

## How to use

This library is based on state change notifications:

1. Whenever the value changes, you notify the library via `debouncer.changed(newValue)`

2. Whenever the _debounced_ value changes, you are notified on your callback.

Since the library needs to wait a little to ensure the value is stable, you need to wake it up later with `debouncer.flush()`.

All functions return the amount of time you need to wait before waking it up again (Or -1, if you don't need to wake it), so that you can schedule it efficiently.

If scheduling isn't your thing, you can also poll from the main loop.


For actual code, see the examples:
- [**Polling**](examples/Polling/Polling.ino): uses loop() to poll the value of each pin and trigger the callbacks
- [**Asyncronous**](examples/Async/Async.ino): uses [Pin Change interrupts](https://github.com/paulo-raca/YetAnotherArduinoPcIntLibrary) to listen to digital pins and a [Scheduler](https://github.com/PRosenb/DeepSleepScheduler) to manage the wait intervals.


## Concurrency

The library doesn't support concurrency, it's up to you to ensure synchronization between your main loop and interruptions.

See the [Asyncronous](examples/Async/Async.ino) example.
