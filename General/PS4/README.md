# PS4 Sample Sketch
Load this sketch to test your PS4 Controller with the USB Host Shield.  It will not control any servos, but it will allow you 
to connect the PS4 controller to your Arduino

## Installation
Load the sketch onto your Arduino and open the Serial Monitor.  You should see the controller pair and will see output whenever
you press a button or joystick.

![PS4 Serial](https://raw.githubusercontent.com/redheadedstep/padawanps4/master/General/PS4/PS4_Serial.jpg)

## Pairing
To pair the PS4 controller, hold down the *Share* + *PS* Buttons.  The light will blink white rapidly, then turn blue when it 
is paired.

If your bluetooth dongle has an led, it should be blinking steadily (once every second) to indicate it is in pairing mode.

## Bluetooth Dongles

I've tested these dongles and they work great with the PS4 controller:

- Kinivo BTD-400 USB Bluetooth Adapter for PC (https://www.amazon.com/gp/product/B007Q45EF4/)
- Azio USB Micro Bluetooth Adapter V4.0 EDR and aptX (https://www.amazon.com/gp/product/B00JAZGSXY/)

*Note: check that the adapter is compatible.  BestBuy sells an Insignia one that does NOT work*
