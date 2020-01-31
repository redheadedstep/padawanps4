# Padawan PS4
![GitHub last commit](https://img.shields.io/github/last-commit/redheadedstep/padawanps4)
![GitHub](https://img.shields.io/github/license/redheadedstep/padawanps4)

Padawan PS4 is a set of Arduino Sketches that work to control different droids.  Right now, it just has sketches for connecting to the PS4 Controller and controller D-O.  D-O is 3D printed from plans from Mr Baddeley.

## Setup
- Load the sketch onto your arduino
- Plug the Bluetooth USB into the USB Host Shield.  
- Pair your PS4 controller to the bluetooth by holding down `Share + PS Button`.  The light on the controller should blink rapidly as it is pairing, then go solid blue.

## PS4BT Sketch
This sketch is pulled from the USB Host Shield project here: https://github.com/felis/USB_Host_Shield_2.0.  It is a basic sketch that lets you verify that your controller is connected and all the joysticks and buttons are working.

## Wiring Diagram
Check in the D-O folder for a wiring diagram.  I am using a Matek Power Distribution Board to keep the wires cleaner.
![D-O wiring diagram](https://raw.githubusercontent.com/redheadedstep/padawanps4/master/D-O/WiringDiagram.jpg)

## Hardware

You will need the following hardware in your D-O:

### Servos
- (2) FS5103R or FS5106R - Continuous Rotation Servos for the wheels
- (2) MG90S or SG90S - 180° Servos for the neck
- (2) MG99R - 90° Servos for the nodbar

### Boards
- Arduino Mega or Uno (https://www.amazon.com/gp/product/B01H4ZLZLQ/)
- USB Host Shield (https://www.amazon.com/gp/product/B07J2KKGZ4/)
- MP3 Shield (https://www.amazon.com/gp/product/B07Y21HKKL/)
- Adafruit PWM Board (https://www.amazon.com/gp/product/B07D6SXDJT/)
- USB Bluetooth Adapter
  - Kinivo BTD-400 USB Bluetooth Adapter for PC (https://www.amazon.com/gp/product/B007Q45EF4/)
  - Azio USB Micro Bluetooth Adapter V4.0 EDR and aptX (https://www.amazon.com/gp/product/B00JAZGSXY/)
  - *Note: check that the adapter is compatible.  BestBuy sells an Insignia one that does NOT work*
- PS4 Dualshock controller

## PadawanPS4 Sketch
This sketch is used to control D-O.

### Install Libraries
Before you start, you need to install some libraries.  Open your Arduino IDE and click on `Tools -> Manage Libraries`.  You will want to install the following libraries:

- ServoEasing by Armin Joachimsmeyer
- Adafruit PWM Servo Driver Library by Adafruit 
- Yet Another Arduino Debounce Library by Paulo Costa
- USB Host Shield Library 2.0 by Oleg Mazurov (Circuits@Home)

Once you have installed the library, you should be able to run the sketch.

#### Changes to ServoEasing library
You may need to make some changes to the ServoEasing library depending on your servos.  

On line 35, you will want to enable the servo expander (Adafruit PWM):

```
/*
 * For use with e.g. the Adafruit PCA9685 16-Channel Servo Driver aOffUnits.
 */
#define USE_PCA9685_SERVO_EXPANDER
```

On lines 207-215, you will want to change the values to match the FS5103R/FS5106R:

```
/*
 * Definitions here are only for convenience. You may freely modify them.
 */
#define MICROSECONDS_FOR_ROTATING_SERVO_CLOCKWISE_MAX (MICROSECONDS_FOR_ROTATING_SERVO_STOP - 1000)
#define MICROSECONDS_FOR_ROTATING_SERVO_CLOCKWISE_HALF (MICROSECONDS_FOR_ROTATING_SERVO_STOP - 500)
#define MICROSECONDS_FOR_ROTATING_SERVO_CLOCKWISE_QUARTER (MICROSECONDS_FOR_ROTATING_SERVO_STOP - 250)
#define MICROSECONDS_FOR_ROTATING_SERVO_COUNTER_CLOCKWISE_MAX (MICROSECONDS_FOR_ROTATING_SERVO_STOP + 1000)
#define MICROSECONDS_FOR_ROTATING_SERVO_COUNTER_CLOCKWISE_HALF (MICROSECONDS_FOR_ROTATING_SERVO_STOP + 500)
#define MICROSECONDS_FOR_ROTATING_SERVO_COUNTER_CLOCKWISE_QUARTER (MICROSECONDS_FOR_ROTATING_SERVO_STOP + 250)
```

#### Changes to YetAnotherDebouncer Library
You will need to make one small change to this library.  It defaults to only sending a boolean value (true/false) because it is made for buttons, but we want it to send an integer value (joystick position).  Simply change `bool` to `int` on line 16:

```
// Old code
template<typename clock=Clock::Millis, typename value_t=bool>
```

```
// New code
template<typename clock=Clock::Millis, typename value_t=int>
```

### Configuration
The sketch is setup assuming that you are using the same servos as specified in the D-O instructions.  These servos are:

- (2) FS5103R or FS5106R - Continuous Rotation Servos for the wheels
- (2) MG90S or SG90S - 180° Servos for the neck
- (2) MG99R - 90° Servos for the nodbar

Each servo is named in the code:

- FS5103R or FS5106R - Named as `RIGHT_THROTTLE` or `LEFT_THROTTLE`
- MG90S or SG90S - Named as `NECK_TURN` (bottom servo) or `NECK_TILT` (top servo)
- MG99R - Named as `HEAD_TILT` (mainbar servo) or `HEAD_NOD` (nodbar servo)

Each servo has a pin associated with it that you can customize.  The pin corresponds to the location on the PWM board, starting with Pin 0.  For instance, to change which pin the right wheel servo is plugged into, you would change this line:

```
/*
 * Sets the PWM pin number for the right wheel
 */
const int RIGHT_THROTTLE_PWM_PIN = 1;
```

### Test Mode
While testing, you can enable `TEST_MODE`.  This will bypass the controller and send signals directly to the servos.  Each servo will run the maximum speed in one direction for 1.5 seconds, then reverse and run the maximum speed in the other direction for 1.5 seconds.  Logs are output to the serial console so you can know which servo is being tested.  

You can enable `TEST_MODE` by changing this constant in the file:

```
/*
 * Sets the sketch into test mode for testing the servos and PWM
 */
const boolean TEST_MODE = false;
const int TEST_MODE_DELAY = 1500;
```

Additionally, while you are in `TEST_MODE`, if you connect the controller, the automatic testing will stop and the controller testing will start.  While in controller test mode, any button or joystick you press on the controller will be output to the serial console.  The controller *will not* control the servos.

## Controls

The sketch is setup to control D-O using the following controls:

- **Left Joystick Up** - Both wheels move forward
- **Left Joystick Down** - Both wheels move backward
- **Left Joystick Left** - Left wheel moves backward, right wheel moves forward (like a tank turning)
- **Left Joystick Right** - Right wheel moves backward, left wheel moves forward (like a tank turning)

- **Right Joystick Up** - Mainbar rotates forward (moving the head box forward)
- **Right Joystick Down** - Mainbar rotates backward (moving the head box back)
- **Right Joystick Left** - Head rotates left (moving the head left, like nodding the word "no")
- **Right Joystick Right** - Head rotates right (moving the head right, like nodding the word "no")

- **Tilt Controller Forward** - Nodbar rotates up (tilting the head down, like nodding "yes")
- **Tilt Controller Backward** - Nodbar rotates down (tilting the head up, like nodding "yes")
- **Tilt Controller Left** - Head tilts left (cocks the head left, like in the movie)
- **Tilt Controller Right** - Head tilts right (cocks the head right, like in the movie)

- **Press Button (Circle)** - Activate a sound on PIN A0
- **Press Button (Triangle)** - Activate a sound on PIN A1
- **Press Button (Square)** - Activate a sound on PIN A5
- **Press Button (Cross)** - Activate a sound on PIN A4

## Errors

### ERROR: `no matching function for call to 'Debouncer<Clock::Millis>::set_callback(void (&)(int))'`

### Solution:

You need to edit the `Yet_Another_Arduino_Debounce_Library/src/YetAnotherDebouncer.h` file.  Simply change `bool` to `int` on line 16:

```
// Old code
template<typename clock=Clock::Millis, typename value_t=bool>
```

```
// New code
template<typename clock=Clock::Millis, typename value_t=int>
```

### ERROR: Forward and Backward doesn't work on my D-O, it moves so slowly

### Solution:

The ServoEasing library defaults to different servos that have a smaller range than the FS5103R/FS5106R.  You need to increase this range.  The FS5103R/FS5106R range from 500-2500µs (or +1000µs from center and -1000µs from center), so change lines 207-215 in `Arduino/libraries/ServoEasing/src/ServoEasing.h` to the values to match the FS5103R/FS5106R:

```
/*
 * Definitions here are only for convenience. You may freely modify them.
 */
#define MICROSECONDS_FOR_ROTATING_SERVO_CLOCKWISE_MAX (MICROSECONDS_FOR_ROTATING_SERVO_STOP - 1000)
#define MICROSECONDS_FOR_ROTATING_SERVO_CLOCKWISE_HALF (MICROSECONDS_FOR_ROTATING_SERVO_STOP - 500)
#define MICROSECONDS_FOR_ROTATING_SERVO_CLOCKWISE_QUARTER (MICROSECONDS_FOR_ROTATING_SERVO_STOP - 250)
#define MICROSECONDS_FOR_ROTATING_SERVO_COUNTER_CLOCKWISE_MAX (MICROSECONDS_FOR_ROTATING_SERVO_STOP + 1000)
#define MICROSECONDS_FOR_ROTATING_SERVO_COUNTER_CLOCKWISE_HALF (MICROSECONDS_FOR_ROTATING_SERVO_STOP + 500)
#define MICROSECONDS_FOR_ROTATING_SERVO_COUNTER_CLOCKWISE_QUARTER (MICROSECONDS_FOR_ROTATING_SERVO_STOP + 250)
```

### ERROR: When I move the controller, there is a delay before D-O moves

### Solution:

This is caused from the debouncing library.  It only sends a signal to the servo once a value (like a joystick movement) has registered as the same value for longer than a threshold time.  You can set this threshold time in `PadawanPS4.ino` on line 229:

```
const int debounce = 50; // time in milliseconds to wait until we send a signal to a servo
```

## Help
If you need any help, send an email to redheadedstep@me.com or create an issue on the repository.  

## Credits
Credit goes to Kristian Lauszus (kristianl@tkjelectronics.com) for creating the PS4 library for the USB Host Shield, Mr Baddeley for the 3-D files for D-O, authors of the various libraries, and you for building a droid!
