/**
 Padawan PS4 sketch
 This sketch will control D-O with a PS4 Dualshock conroller.  The controller is mapped as follows:
  - Left Joystick - Forward moves both forward, Backward moves both backward.  Turning left or right will counterspin the wheels (like a tank)
  - Right Joystick Up/Down - Moves the mainbar and headbox forward and backward
  - Right Joystick Left/Rigth - Moves the neck left and right
  - Tilt controller Up/Down - Moves the nodbar to make the head nod up and down
  - Roll controller Left/Right - Cocks the head left and right by moving the neck servo

 Author: Mark Hobson
 Email: redheadedstep@me.com
 */
const boolean _DEBUG = false;

#include <PS4BT.h>
#include <Adafruit_PWMServoDriver.h>
#include <ServoEasing.h>
#include <YetAnotherDebouncer.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

/*
 * Initialize the PWM controller, USB and Bluetooth dongle
 */
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
USB Usb;
BTD Btd(&Usb);

/*
 * Initialize the MP3 player and serial pins (10, 11)
 * Connection and Diagram can be found here: <https://www.dfrobot.com/wiki/index.php/DFPlayer_Mini_SKU:DFR0299#Connection_Diagram>
 */
SoftwareSerial mp3Serial(10, 11); // RX, TX
DFRobotDFPlayerMini mp3Player;

/* 
 * You can create the instance of the PS4BT class in two ways
 * This will start an inquiry and then pair with the PS4 controller - you only have to do this once 
 * You will need to hold down the PS and Share button at the same time, the PS4 controller will then start to blink rapidly indicating that it is in pairing mode
 * > PS4BT PS4(&Btd, PAIR); 
 *
 *  - or -
 * If you are already paired, you don't need to pass the PAIR parameter 
 * > PS4BT PS4(&Btd); 
 */
PS4BT PS4(&Btd, PAIR);
//PS4BT PS4(&Btd);

/*
 * The following will map MP3 sound files so they can be used in the code easier.  You will want to 
 * map the song index to the constant.  For some instance, all my songs are odd (i.e. no sound on even tracks...could be the board or SD Card)
 */
const int MP3_SAD = 1;
const int MP3_NO_THANK_YOU_1 = 3;
const int MP3_SORRY_SHE_IS_GONE = 5;
const int MP3_I_AM_DO = 7;
const int MP3_HELLO_1 = 9;
const int MP3_I_MISS_HER = 11;
const int MP3_FRIENDS_AHEAD = 13;
const int MP3_SQUEAKY_WHEEL = 15;
const int MP3_SQUEAKY_WHEEL_ELIMINATED = 17;
const int MP3_NO_THANK_YOU_2 = 19;
const int MP3_HELLO_2 = 21;
const int MP3_HELLO_3 = 23;
const int MP3_WHAT_IS_THAT = 25;
const int MP3_BATTERY_CHARGED = 27;
/*
 * Set the index of the mp3 to play when we power on
 */
const int MP3_POWER_ON = MP3_BATTERY_CHARGED;

/*
 * Set the MP3 Volume
 */
const int MP3_VOLUME = 20;
/*
 * Set how long to wait to load the MP3 files before causing an error
 * Default timeout is 500ms
 */
const int MP3_TIMEOUT = 1000;
/*
 * Stores the current set of sounds.  To map all the sounds, we use the Circle, Triangle, Square, and X buttons
 * to play 4 different sounds.  We then use the Up, Down, Left, Right to toggle between 4 different categories of 
 * sounds.  You can define the categories below.
 */
int MP3_SOUNDS[4][4] = {
  { // UP
    MP3_I_AM_DO, // TRIANGLE
    MP3_HELLO_1, // CIRCLE
    MP3_WHAT_IS_THAT, // CROSS
    MP3_FRIENDS_AHEAD //SQUARE
  },
  { // RIGHT
    MP3_SAD, // TRIANGLE
    MP3_I_MISS_HER, // CIRCLE
    MP3_SORRY_SHE_IS_GONE, // CROSS
    MP3_FRIENDS_AHEAD //SQUARE
  },
  { // DOWN
    MP3_NO_THANK_YOU_1, // TRIANGLE
    MP3_HELLO_3, // CIRCLE
    MP3_SQUEAKY_WHEEL_ELIMINATED, // CROSS
    MP3_SQUEAKY_WHEEL //SQUARE
  },
  { // LEFT
    MP3_I_AM_DO, // TRIANGLE
    MP3_HELLO_1, // CIRCLE
    MP3_BATTERY_CHARGED, // CROSS
    MP3_HELLO_2 //SQUARE
  }
};

int LAST_SOUND_BUTTON = UP;

/*
 * The following settings affect the throttle.  These will be FS5103R or FS5106R servos.  Both models
 * work the same.  You have to send a pulse to the servo using the PWM.  The pulse has a MAX (clockwise)
 * and MIN (counter-clockwise).  Halfway between the MIN and MAX should be where the servo stops moving.
 * The MIN and MAX are measured in microseconds.
 * 
 * Set the Throttle Servo (FS5103R or FS5106R) pulse signals.  You can get these from the 
 * datasheet.  They normally are in the range of 500 µs - 2500 µs
 */
const int THROTTLE_SERVO_MIN = 500; // Minimum pulse in µs
const int THROTTLE_SERVO_MAX = 2500; // Maximum pulse in µs
const int THROTTLE_SERVO_STOP = ((THROTTLE_SERVO_MIN + THROTTLE_SERVO_MAX) / 2);

const int THROTTLE_ROTATION_MIN = -100; // Minimum pulse in µs
const int THROTTLE_ROTATION_MAX = 100; // Maximum pulse in µs
const int THROTTLE_ROTATION_STOP = ((THROTTLE_ROTATION_MIN + THROTTLE_ROTATION_MAX) / 2);
/*
 * Set the dead zone of the hats on the controller (middle area that will not send a signal)
 * CENTER: PS4 joysticks range from 0-255 with 127 being in the middle
 * DEAD_ZONE: Dead zone in the middle of the joystick (10 is a good number, it means there is 
 *            a dead zone from 117 - 137, or about a 10% area around the center of the joystick)
 */
const int THROTTLE_CENTER = 127;
const int THROTTLE_DEAD_ZONE = 10;
/*
 * Sets the PWM pin number for the left wheel
 */
const int LEFT_THROTTLE_PWM_PIN = 0;
/*
 * Sets the PWM pin number for the right wheel
 */
const int RIGHT_THROTTLE_PWM_PIN = 1;

/*
 * The following settings affect the neck side to side movement.  These will be SG90S or MG90S servos.  oth models
 * work the same.  You have to send a pulse to the servo using the PWM.  The pulse has a MAX (clockwise)
 * and MIN (counter-clockwise).  Halfway between the MIN and MAX should be where the servo stops moving.
 * The MIN and MAX are measured in microseconds.
 * 
 * Set the Nod Servo (MG90S or SG90S) pulse signals.  You can get these from the 
 * datasheet.  They normally are in the range of 1000 µs - 2000 µs
 */
const int NECK_TURN_SERVO_MIN = 500; // Minimum pulse in µs
const int NECK_TURN_SERVO_MAX = 2500; // Maximum pulse in µs
const int NECK_TURN_SERVO_STOP = ((NECK_TURN_SERVO_MIN + NECK_TURN_SERVO_MAX) / 2);

const int NECK_TURN_ROTATION_MIN = 0; // Minimum rotation in degrees
const int NECK_TURN_ROTATION_MAX = 180; // Maximum rotation in degrees
const int NECK_TURN_ROTATION_STOP = ((NECK_TURN_ROTATION_MIN + NECK_TURN_ROTATION_MAX) / 2);
/*
 * Set the dead zone of the hats on the controller (middle area that will not send a signal)
 * CENTER: PS4 joysticks range from 0-255 with 127 being in the middle
 * DEAD_ZONE: Dead zone in the middle of the joystick (10 is a good number, it means there is 
 *            a dead zone from 117 - 137, or about a 10% area around the center of the joystick)
 */
const int NECK_TURN_CENTER = 127;
const int NECK_TURN_DEAD_ZONE = 10;
/*
 * Sets the PWM pin number for the neck nod
 */
const int NECK_TURN_PWM_PIN = 5;


/*
 * The following settings affect the neck tilt movement.  These will be SG90S or MG90S servos.  oth models
 * work the same.  You have to send a pulse to the servo using the PWM.  The pulse has a MAX (clockwise)
 * and MIN (counter-clockwise).  Halfway between the MIN and MAX should be where the servo stops moving.
 * The MIN and MAX are measured in microseconds.
 * 
 * Set the Nod Servo (MG90S or SG90S) pulse signals.  You can get these from the 
 * datasheet.  They normally are in the range of 500 µs - 2500 µs, but we don't want to hit
 * the neck, so set it 200-300µs larger
 */
const int NECK_TILT_SERVO_MIN = 800; // Minimum pulse in µs
const int NECK_TILT_SERVO_MAX = 2200; // Maximum pulse in µs
const int NECK_TILT_SERVO_STOP = ((NECK_TILT_SERVO_MIN + NECK_TILT_SERVO_MAX) / 2);

const int NECK_TILT_ROTATION_MIN = 0; // Minimum rotation in degrees
const int NECK_TILT_ROTATION_MAX = 180; // Maximum rotation in degrees
const int NECK_TILT_ROTATION_STOP = ((NECK_TILT_ROTATION_MIN + NECK_TILT_ROTATION_MAX) / 2);
/*
 * Set the dead zone of the hats on the controller (middle area that will not send a signal)
 * CENTER: PS4 joysticks range from 0-255 with 127 being in the middle
 * DEAD_ZONE: Dead zone in the middle of the joystick (10 is a good number, it means there is 
 *            a dead zone from 117 - 137, or about a 10% area around the center of the joystick)
 */
const int NECK_TILT_CENTER = 127;
const int NECK_TILT_DEAD_ZONE = 10;
const int ROLL_DEAD_ZONE_LEFT = 90;
const int ROLL_DEAD_ZONE_RIGHT = 270;

/*
 * Sets the PWM pin number for the neck tilt
 */
const int NECK_TILT_PWM_PIN = 4;


/*
 * The following settings affect the headbox tilt movement (large vertial bar from wheel to headbox).  
 * These will be SG99R or MG99R servos.  Both models work the same.  You have to send a pulse to the servo 
 * using the PWM.  The pulse has a MAX (clockwise) and MIN (counter-clockwise).  Halfway between the MIN 
 * and MAX should be where the servo stops moving.  The MIN and MAX are measured in microseconds.
 * 
 * Set the Nod Servo (SG99R or MG99R) pulse signals.  You can get these from the 
 * datasheet.  They normally are in the range of 1000 µs - 2000 µs
 */
const int HEAD_TILT_SERVO_MIN = 1000; // Minimum pulse in µs
const int HEAD_TILT_SERVO_MAX = 2000; // Maximum pulse in µs
const int HEAD_TILT_SERVO_STOP = ((HEAD_TILT_SERVO_MIN + HEAD_TILT_SERVO_MAX) / 2);

const int HEAD_TILT_ROTATION_MIN = 0; // Minimum rotation in degrees
const int HEAD_TILT_ROTATION_MAX = 90; // Maximum rotation in degrees
const int HEAD_TILT_ROTATION_STOP = ((HEAD_TILT_ROTATION_MIN + HEAD_TILT_ROTATION_MAX) / 2);
/*
 * Set the dead zone of the hats on the controller (middle area that will not send a signal)
 * CENTER: PS4 joysticks range from 0-255 with 127 being in the middle
 * DEAD_ZONE: Dead zone in the middle of the joystick (10 is a good number, it means there is 
 *            a dead zone from 117 - 137, or about a 10% area around the center of the joystick)
 */
const int HEAD_TILT_CENTER = 127;
const int HEAD_TILT_DEAD_ZONE = 3;
/*
 * Sets the PWM pin number for the neck tilt
 */
const int HEAD_TILT_PWM_PIN = 3;


/*
 * The following settings affect the headbox nod movement (small vertial bar from wheel to headbox).  
 * These will be SG99R or MG99R servos.  Both models work the same.  You have to send a pulse to the servo 
 * using the PWM.  The pulse has a MAX (clockwise) and MIN (counter-clockwise).  Halfway between the MIN 
 * and MAX should be where the servo stops moving.  The MIN and MAX are measured in microseconds.
 * 
 * Set the Nod Servo (SG99R or MG99R) pulse signals.  You can get these from the 
 * datasheet.  They normally are in the range of 1000 µs - 2000 µs
 */
const int HEAD_NOD_SERVO_MIN = 500; // Minimum pulse in µs
const int HEAD_NOD_SERVO_MAX = 2500; // Maximum pulse in µs
const int HEAD_NOD_SERVO_STOP = ((HEAD_NOD_SERVO_MIN + HEAD_NOD_SERVO_MAX) / 2);

const int HEAD_NOD_ROTATION_MIN = 0; // Minimum rotation in degrees
const int HEAD_NOD_ROTATION_MAX = 90; // Maximum rotation in degrees
const int HEAD_NOD_ROTATION_STOP = ((HEAD_NOD_ROTATION_MIN + HEAD_NOD_ROTATION_MAX) / 2);

/*
 * Set the dead zone of the hats on the controller (middle area that will not send a signal)
 * CENTER: PS4 joysticks range from 0-255 with 127 being in the middle
 * DEAD_ZONE: Dead zone in the middle of the joystick (10 is a good number, it means there is 
 *            a dead zone from 117 - 137, or about a 10% area around the center of the joystick)
 */
const int HEAD_NOD_CENTER = 127;
const int HEAD_NOD_DEAD_ZONE = 10;
const int PITCH_DEAD_ZONE_FORWARD = 90;
const int PITCH_DEAD_ZONE_BACKWARD = 270;
/*
 * Sets the PWM pin number for the neck tilt
 */
const int HEAD_NOD_PWM_PIN = 2;

/*
 * Sets the sketch into test mode for testing the servos and PWM
 */
const boolean TEST_MODE = false;
const int TEST_MODE_DELAY = 1500;

/* 
 * Timing constants 
 */
unsigned long NewTick;         // [ms] time start of current loop
unsigned long LastTick;        // [ms] time start of previous loop
float dt;                      // [ms] time between consecutive loops


int throttleLeft = THROTTLE_SERVO_STOP;
int throttleRight = THROTTLE_SERVO_STOP;
int neckTurn = NECK_TURN_SERVO_STOP;
int neckTilt = NECK_TILT_SERVO_STOP;
int headNod = HEAD_NOD_SERVO_STOP;
int headTilt = HEAD_TILT_SERVO_STOP;

/*
 * Sets the debounce threshold.  Debouncing allows you to use a joystick and the script will only send a signal to 
 * the PWM when the value is steady for 50ms (debounce value).  Debouncing keeps you from sending erratic data to 
 * a servo and makes for smoother movements.
 */
const int debounce = 50;
Debouncer<Clock::Millis> neckTurnValue;
Debouncer<Clock::Millis> neckTiltValue;
Debouncer<Clock::Millis> headTiltValue;
Debouncer<Clock::Millis> headNodValue;
Debouncer<Clock::Millis> leftWheelValue;
Debouncer<Clock::Millis> rightWheelValue;

/*
 * Setup all our servos to use easing
 */
ServoEasing HeadNodServo;
ServoEasing HeadTiltServo;
ServoEasing NeckTiltServo;
ServoEasing NeckTurnServo;
ServoEasing LeftWheelServo;
ServoEasing RightWheelServo;

/* used for debugging */
bool printAngle, printTouch;
uint8_t oldL2Value, oldR2Value;

void setup() {
  /* Start the serial port */
  Serial.begin(115200);  
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection

  /* Initialize the USB Host Shield */
  _DEBUG && Serial.print(F("Initializing USB..."));
  if (Usb.Init() == -1) {
    _DEBUG && Serial.println(F("USB did not start"));
    while (true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  _DEBUG && Serial.println(F("done"));

  /* Initialize the PWM Controller */
  _DEBUG && Serial.print(F("Initializing PWM..."));
  pwm.begin();
  pwm.setPWMFreq(50);  // Analog servos run at ~60 Hz updates 
  delay(10);
  _DEBUG && Serial.println(F("done"));  

  /* Initialize the Servos, assign their pins, and setup debouncing on them */
  _DEBUG && Serial.print(F("Initializing Servos...\n"));
  if (NeckTurnServo.attach(NECK_TURN_PWM_PIN) == INVALID_SERVO) {
      _DEBUG && Serial.print(F("Error attaching NeckTurnServo to pin "));
      _DEBUG && Serial.println(NECK_TURN_PWM_PIN);
  } else {
    _DEBUG && Serial.print(F("  Neck Turn Servo..."));
    NeckTurnServo.setEasingType(EASE_CUBIC_IN_OUT);
    NeckTurnServo.setSpeed(1000);
    NeckTurnServo.write(NECK_TURN_ROTATION_STOP);
    neckTurnValue.begin();
    neckTurnValue.set_debounce_stable(debounce);
    neckTurnValue.set_callback(doNeckTurn);
    _DEBUG && Serial.println(F("done"));
  }
  if (NeckTiltServo.attach(NECK_TILT_PWM_PIN) == INVALID_SERVO) {
      _DEBUG && Serial.print(F("Error attaching NeckTiltServo to pin "));
      _DEBUG && Serial.println(NECK_TILT_PWM_PIN);
  } else {
    _DEBUG && Serial.print(F("  Neck Tilt Servo..."));
    NeckTiltServo.setEasingType(EASE_CUBIC_IN_OUT);
    NeckTiltServo.setSpeed(1000);
    NeckTiltServo.write(NECK_TILT_ROTATION_STOP);
    neckTiltValue.begin();
    neckTiltValue.set_debounce_stable(debounce);
    neckTiltValue.set_callback(doNeckTilt);
    _DEBUG && Serial.println(F("done"));
  }
  if (HeadTiltServo.attach(HEAD_TILT_PWM_PIN) == INVALID_SERVO) {
      _DEBUG && Serial.print(F("Error attaching HeadTiltServo to pin "));
      _DEBUG && Serial.println(HEAD_TILT_PWM_PIN);
  } else {
    _DEBUG && Serial.print(F("  Head Tilt Servo..."));
    HeadTiltServo.setEasingType(EASE_CUBIC_IN_OUT);
    HeadTiltServo.setSpeed(1000);
    HeadTiltServo.write(HEAD_TILT_ROTATION_STOP);
    headTiltValue.begin();
    headTiltValue.set_debounce_stable(debounce);
    headTiltValue.set_callback(doHeadTilt);
    _DEBUG && Serial.println(F("done"));
  }
  if (HeadNodServo.attach(HEAD_NOD_PWM_PIN) == INVALID_SERVO) {
      _DEBUG && Serial.print(F("Error attaching HeadNodServo to pin "));
      _DEBUG && Serial.println(HEAD_NOD_PWM_PIN);
  } else {
    _DEBUG && Serial.print(F("  Head Nod Servo..."));
    HeadNodServo.setEasingType(EASE_CUBIC_IN_OUT);
    HeadNodServo.setSpeed(1000);
    HeadNodServo.write(HEAD_NOD_ROTATION_STOP);
    headNodValue.begin();
    headNodValue.set_debounce_stable(debounce);
    headNodValue.set_callback(doHeadNod);    
    _DEBUG && Serial.println(F("done"));
  }
  if (LeftWheelServo.attach(LEFT_THROTTLE_PWM_PIN, MICROSECONDS_FOR_ROTATING_SERVO_CLOCKWISE_MAX, MICROSECONDS_FOR_ROTATING_SERVO_COUNTER_CLOCKWISE_MAX, 100, -100) == INVALID_SERVO) {
      _DEBUG && Serial.print(F("Error attaching LeftWheelServo to pin "));
      _DEBUG && Serial.println(LEFT_THROTTLE_PWM_PIN);
  } else {
    _DEBUG && Serial.print(F("  Left Wheel Servo..."));
    LeftWheelServo.setEasingType(EASE_CUBIC_IN_OUT);
    LeftWheelServo.setSpeed(90);
    LeftWheelServo.write(THROTTLE_ROTATION_STOP);
    leftWheelValue.begin();
    leftWheelValue.set_debounce_stable(debounce);
    leftWheelValue.set_callback(doThrottleLeft);
    _DEBUG && Serial.println(F("done"));
  }
  if (RightWheelServo.attach(RIGHT_THROTTLE_PWM_PIN, MICROSECONDS_FOR_ROTATING_SERVO_CLOCKWISE_MAX, MICROSECONDS_FOR_ROTATING_SERVO_COUNTER_CLOCKWISE_MAX, 100, -100) == INVALID_SERVO) {
      _DEBUG && Serial.print(F("Error attaching RightWheelServo to pin "));
      _DEBUG && Serial.println(RIGHT_THROTTLE_PWM_PIN);
  } else {
    _DEBUG && Serial.print(F("  Right Wheel Servo..."));
    RightWheelServo.setEasingType(EASE_CUBIC_IN_OUT);
    RightWheelServo.setSpeed(90);
    RightWheelServo.write(THROTTLE_ROTATION_STOP);
    rightWheelValue.begin();
    rightWheelValue.set_debounce_stable(debounce);
    rightWheelValue.set_callback(doThrottleRight);
    _DEBUG && Serial.println(F("done"));
  }
  _DEBUG && Serial.print(F("done\n"));

//  Serial.print(F("Resetting Servos...\n"));
//  Serial.print(F("Resetting Neck Turn...\n"));
//  doNeckTurn(NECK_TURN_ROTATION_MIN);
//  delay(1000);
//  doNeckTurn(NECK_TURN_ROTATION_MAX);
//  delay(1000);
//  doNeckTurn(NECK_TURN_ROTATION_STOP);
//  Serial.print(F("Resetting Neck Tilt...\n"));
//  doNeckTilt(NECK_TILT_ROTATION_MIN);
//  delay(1000);
//  doNeckTilt(NECK_TILT_ROTATION_MAX);
//  delay(1000);
//  doNeckTilt(NECK_TILT_ROTATION_STOP);
//  Serial.print(F("Resetting Head Tilt...\n"));
//  doHeadTilt(HEAD_TILT_ROTATION_MIN);
//  delay(1000);
//  doHeadTilt(HEAD_TILT_ROTATION_MAX);
//  delay(1000);
//  doHeadTilt(HEAD_TILT_ROTATION_STOP);
//  Serial.print(F("Resetting Head Turn...\n"));
//  doHeadNod(HEAD_NOD_ROTATION_MIN);
//  delay(1000);
//  doHeadNod(HEAD_NOD_ROTATION_MAX);
//  delay(1000);
//  doHeadNod(HEAD_NOD_ROTATION_STOP);
//  Serial.print(F("Resetting Wheels...\n"));
//  doThrottleLeft(THROTTLE_ROTATION_STOP);
//  doThrottleRight(THROTTLE_ROTATION_STOP);
//  Serial.print(F("done\n"));

  /* Initialize the MP3 card and play a POWER_ON sound */
  _DEBUG && Serial.print(F("Initializing MP3..."));
  mp3Serial.begin(9600);
  mp3Player.setTimeOut(MP3_TIMEOUT);
  if (!mp3Player.begin(mp3Serial, true, false)) {  //Use softwareSerial to communicate with mp3.
    _DEBUG && Serial.println(F("Unable to start MP3:"));
    _DEBUG && Serial.println(F("  Error from MP3: "));
    mp3Detail(mp3Player.readType(), mp3Player.readCurrentFileNumber());
    _DEBUG && Serial.println(F("  Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  _DEBUG && Serial.println(F("done"));
  mp3Player.volume(MP3_VOLUME);  //Set volume value. From 0 to 30
  _DEBUG && Serial.print(F("Playing MP3_POWER_ON file..."));
  mp3Player.play(MP3_POWER_ON);
  _DEBUG && Serial.println(F("done"));

  /* Everything is setup, so we can continue */
  Serial.println(F("PS4 Wireless Receiver Library Started"));
}

void loop() {
  NewTick = millis();

  dt = NewTick - LastTick;

  if (dt > 50) {
    Usb.Task();
  
    if (TEST_MODE) {
      if (PS4.connected()) {
        DebugController();  
      } else {
        testServos();
      }
    } else {        
      if (PS4.connected()) {
        int joyY = map(PS4.getAnalogHat(LeftHatY), 0, 255, -100, 100);
        int joyX = map(PS4.getAnalogHat(LeftHatX), 0, 255, -100, 100);
        joyY = -joyY;
        
        int joyV = (100 - abs(joyY)) * (joyX/100) + joyX;
        int joyW = (100 - abs(joyX)) * (joyY/100) + joyY;
          
        leftWheelValue.set_value(((joyV + joyW) / 2));
        rightWheelValue.set_value(((joyV - joyW) / 2));
        neckTurnValue.set_value(map(map(PS4.getAnalogHat(RightHatX), 0, 255, 0, 25), 0, 25, 180, 0));
        headTiltValue.set_value(map(PS4.getAnalogHat(RightHatY), 0, 255, 0, 180));
        neckTiltValue.set_value(map(map(PS4.getAngle(Roll), 0, 360, 0, 36), 0, 36, 180, 0));
        headNodValue.set_value(map(PS4.getAngle(Pitch), 0, 360, 0, 180));

        if (PS4.getButtonClick(UP)) {
          LAST_SOUND_BUTTON = UP;
        } else if (PS4.getButtonClick(DOWN)) {
          LAST_SOUND_BUTTON = DOWN;
        } else if (PS4.getButtonClick(RIGHT)) {
          LAST_SOUND_BUTTON = RIGHT;
        } else if (PS4.getButtonClick(LEFT)) {
          LAST_SOUND_BUTTON = LEFT;
        }
    
        if (PS4.getButtonClick(CIRCLE)) {
          mp3Player.play(MP3_SOUNDS[LAST_SOUND_BUTTON][0]);
        }
        if (PS4.getButtonClick(TRIANGLE)) {
          mp3Player.play(MP3_SOUNDS[LAST_SOUND_BUTTON][1]);
        }
        if (PS4.getButtonClick(SQUARE)) {
          mp3Player.play(MP3_SOUNDS[LAST_SOUND_BUTTON][2]);
        }
        if (PS4.getButtonClick(CROSS)) {
          mp3Player.play(MP3_SOUNDS[LAST_SOUND_BUTTON][3]);
        }
      }
    }
  }
}

/*
 * This function is only used in TEST_MODE and will cycle through each servo and 
 * spin it forwards and backwards
 */
void testServos() {
  /*
   * Test the neck tilt
   */
  Serial.print(F("\nTesting Neck Tilt Minimum ("));
  Serial.print(NECK_TILT_SERVO_MIN);
  Serial.print(F(") on pin "));
  Serial.print(NECK_TILT_PWM_PIN);
  doNeckTilt(NECK_TILT_ROTATION_MIN);
  delay(TEST_MODE_DELAY);
  doNeckTilt(NECK_TILT_ROTATION_STOP);

  Serial.print(F("\nTesting Neck Tilt Maximum ("));
  Serial.print(NECK_TILT_SERVO_MAX);
  Serial.print(F(") on pin "));
  Serial.print(NECK_TILT_PWM_PIN);
  doNeckTilt(NECK_TILT_ROTATION_MAX);
  delay(TEST_MODE_DELAY);
  doNeckTilt(NECK_TILT_ROTATION_STOP);

  /*
   * Test the neck nod
   */
  Serial.print(F("\nTesting Neck Nod Minimum ("));
  Serial.print(NECK_TURN_SERVO_MIN);
  Serial.print(F(") on pin "));
  Serial.print(NECK_TURN_PWM_PIN);
  doNeckTurn(NECK_TURN_ROTATION_MIN);
  delay(TEST_MODE_DELAY);
  doNeckTurn(NECK_TURN_ROTATION_STOP);

  Serial.print(F("\nTesting Neck Nod Maximum ("));
  Serial.print(NECK_TURN_SERVO_MAX);
  Serial.print(F(") on pin "));
  Serial.print(NECK_TURN_PWM_PIN);
  doNeckTurn(NECK_TURN_ROTATION_MAX);
  delay(TEST_MODE_DELAY);
  doNeckTurn(NECK_TURN_ROTATION_STOP);


  /*
   * Test the head tilt
   */
  Serial.print(F("\nTesting Head Tilt Minimum ("));
  Serial.print(HEAD_TILT_SERVO_MIN);
  Serial.print(F(") on pin "));
  Serial.print(HEAD_TILT_PWM_PIN);
  doHeadTilt(HEAD_TILT_ROTATION_MIN);
  delay(TEST_MODE_DELAY);
  doHeadTilt(HEAD_TILT_ROTATION_STOP);

  Serial.print(F("\nTesting Head Tilt Maximum ("));
  Serial.print(HEAD_TILT_SERVO_MAX);
  Serial.print(F(") on pin "));
  Serial.print(HEAD_TILT_PWM_PIN);
  doHeadTilt(HEAD_TILT_ROTATION_MAX);
  delay(TEST_MODE_DELAY);
  doHeadTilt(HEAD_TILT_ROTATION_STOP);

  /*
   * Test the head nod
   */
  Serial.print(F("\nTesting Head Nod Minimum ("));
  Serial.print(HEAD_NOD_SERVO_MIN);
  Serial.print(F(") on pin "));
  Serial.print(HEAD_NOD_PWM_PIN);
  doHeadNod(HEAD_NOD_ROTATION_MIN);
  delay(TEST_MODE_DELAY);
  doHeadNod(HEAD_NOD_ROTATION_STOP);

  Serial.print(F("\nTesting Head Nod Maximum ("));
  Serial.print(HEAD_NOD_SERVO_MAX);
  Serial.print(F(") on pin "));
  Serial.print(HEAD_NOD_PWM_PIN);
  doHeadNod(HEAD_NOD_ROTATION_MAX);
  delay(TEST_MODE_DELAY);
  doHeadNod(HEAD_NOD_ROTATION_STOP);


  /*
   * Test the left throttle
   */
  Serial.print(F("\nTesting Throttle Left Minimum ("));
  Serial.print(THROTTLE_SERVO_MIN);
  Serial.print(F(") on pin "));
  Serial.print(LEFT_THROTTLE_PWM_PIN);
  doThrottleLeft(THROTTLE_ROTATION_MIN);
  delay(TEST_MODE_DELAY);
  doThrottleLeft(THROTTLE_ROTATION_STOP);


  Serial.print(F("\nTesting Throttle Left Maximum ("));
  Serial.print(THROTTLE_SERVO_MAX);
  Serial.print(F(") on pin "));
  Serial.print(LEFT_THROTTLE_PWM_PIN);
  doThrottleLeft(THROTTLE_ROTATION_MAX);
  delay(TEST_MODE_DELAY);
  doThrottleLeft(THROTTLE_ROTATION_STOP);


  /*
   * Test the right throttle
   */
  Serial.print(F("\nTesting Throttle Right Minimum ("));
  Serial.print(THROTTLE_SERVO_MIN);
  Serial.print(F(") on pin "));
  Serial.print(RIGHT_THROTTLE_PWM_PIN);
  doThrottleRight(THROTTLE_ROTATION_MIN);
  delay(TEST_MODE_DELAY);
  doThrottleRight(THROTTLE_ROTATION_STOP);

  Serial.print(F("\nTesting Throttle Right Maximum ("));
  Serial.print(THROTTLE_SERVO_MAX);
  Serial.print(F(") on pin "));
  Serial.print(RIGHT_THROTTLE_PWM_PIN);
  doThrottleRight(THROTTLE_ROTATION_MAX);
  delay(TEST_MODE_DELAY);
  doThrottleRight(THROTTLE_ROTATION_STOP);
}

/**
 * Turns the neck left and right
 */
void doNeckTurn(int value) {
  _DEBUG && Serial.print(F("Neck Turn: "));
  _DEBUG && Serial.println(value);
  NeckTurnServo.startEaseToD(value, 1000, true);
}

/**
 * Tilts the neck left and right
 */
void doNeckTilt(int value) {
  _DEBUG && Serial.print(F("Neck Tilt: "));
  _DEBUG && Serial.println(value);
  NeckTiltServo.startEaseToD(value, 1000, true);
}

/**
 * Tilts the headbox using the mainbar
 */
void doHeadTilt(int value) {
  _DEBUG && Serial.print(F("Head Tilt: "));
  _DEBUG && Serial.println(value);
  HeadTiltServo.startEaseToD(value, 1000, true);
}

/**
 * Nods the headbox using the nodbar
 */
void doHeadNod(int value) {
  _DEBUG && Serial.print(F("Head Nod: "));
  _DEBUG && Serial.println(value);
  HeadNodServo.startEaseToD(value, 1000, true);
}

/**
 * Turns the left wheel
 */
void doThrottleLeft(int value) {
  _DEBUG && Serial.print(F("Throttle Left: "));
  _DEBUG && Serial.println(value);
  LeftWheelServo.startEaseToD(value, 500, true);
}

/**
 * Turns the right wheel
 */
void doThrottleRight(int value) {
  _DEBUG && Serial.print(F("Throttle Left: "));
  _DEBUG && Serial.println(value);
  RightWheelServo.startEaseToD(value, 500, true);
}

/*
 * This function is used only in TEST_MODE and lets you see what you are pressing on 
 * the PS4 controller
 */
void DebugController() {
  if (PS4.connected()) {
    if (PS4.getAnalogHat(LeftHatX) > 137 || PS4.getAnalogHat(LeftHatX) < 117 || PS4.getAnalogHat(LeftHatY) > 137 || PS4.getAnalogHat(LeftHatY) < 117 || PS4.getAnalogHat(RightHatX) > 137 || PS4.getAnalogHat(RightHatX) < 117 || PS4.getAnalogHat(RightHatY) > 137 || PS4.getAnalogHat(RightHatY) < 117) {
      Serial.print(F("\r\nLeftHatX: "));
      Serial.print(PS4.getAnalogHat(LeftHatX));
      Serial.print(F("\tLeftHatY: "));
      Serial.print(PS4.getAnalogHat(LeftHatY));
      Serial.print(F("\tRightHatX: "));
      Serial.print(PS4.getAnalogHat(RightHatX));
      Serial.print(F("\tRightHatY: "));
      Serial.print(PS4.getAnalogHat(RightHatY));
    }

    if (PS4.getAnalogButton(L2) || PS4.getAnalogButton(R2)) { // These are the only analog buttons on the PS4 controller
      Serial.print(F("\r\nL2: "));
      Serial.print(PS4.getAnalogButton(L2));
      Serial.print(F("\tR2: "));
      Serial.print(PS4.getAnalogButton(R2));
    }
    if (PS4.getAnalogButton(L2) != oldL2Value || PS4.getAnalogButton(R2) != oldR2Value) // Only write value if it's different
      PS4.setRumbleOn(PS4.getAnalogButton(L2), PS4.getAnalogButton(R2));
    oldL2Value = PS4.getAnalogButton(L2);
    oldR2Value = PS4.getAnalogButton(R2);

    if (PS4.getButtonClick(PS)) {
      Serial.print(F("\r\nPS"));
      PS4.disconnect();
    }
    else {
      if (PS4.getButtonClick(TRIANGLE)) {
        Serial.print(F("\r\nTriangle"));
        PS4.setRumbleOn(RumbleLow);
      }
      if (PS4.getButtonClick(CIRCLE)) {
        Serial.print(F("\r\nCircle"));
        PS4.setRumbleOn(RumbleHigh);
      }
      if (PS4.getButtonClick(CROSS)) {
        Serial.print(F("\r\nCross"));
        PS4.setLedFlash(10, 10); // Set it to blink rapidly
      }
      if (PS4.getButtonClick(SQUARE)) {
        Serial.print(F("\r\nSquare"));
        PS4.setLedFlash(0, 0); // Turn off blinking
      }

      if (PS4.getButtonClick(UP)) {
        Serial.print(F("\r\nUp"));
        PS4.setLed(Red);
      } if (PS4.getButtonClick(RIGHT)) {
        Serial.print(F("\r\nRight"));
        PS4.setLed(Blue);
      } if (PS4.getButtonClick(DOWN)) {
        Serial.print(F("\r\nDown"));
        PS4.setLed(Yellow);
      } if (PS4.getButtonClick(LEFT)) {
        Serial.print(F("\r\nLeft"));
        PS4.setLed(Green);
      }

      if (PS4.getButtonClick(L1))
        Serial.print(F("\r\nL1"));
      if (PS4.getButtonClick(L3))
        Serial.print(F("\r\nL3"));
      if (PS4.getButtonClick(R1))
        Serial.print(F("\r\nR1"));
      if (PS4.getButtonClick(R3))
        Serial.print(F("\r\nR3"));

      if (PS4.getButtonClick(SHARE))
        Serial.print(F("\r\nShare"));
      if (PS4.getButtonClick(OPTIONS)) {
        Serial.print(F("\r\nOptions"));
        printAngle = !printAngle;
      }
      if (PS4.getButtonClick(TOUCHPAD)) {
        Serial.print(F("\r\nTouchpad"));
        printTouch = !printTouch;
      }

      if (printAngle) { // Print angle calculated using the accelerometer only
        Serial.print(F("\r\nPitch: "));
        Serial.print(PS4.getAngle(Pitch));
        Serial.print(F("\tRoll: "));
        Serial.print(PS4.getAngle(Roll));
      }

      if (printTouch) { // Print the x, y coordinates of the touchpad
        if (PS4.isTouching(0) || PS4.isTouching(1)) // Print newline and carriage return if any of the fingers are touching the touchpad
          Serial.print(F("\r\n"));
        for (uint8_t i = 0; i < 2; i++) { // The touchpad track two fingers
          if (PS4.isTouching(i)) { // Print the position of the finger if it is touching the touchpad
            Serial.print(F("X")); Serial.print(i + 1); Serial.print(F(": "));
            Serial.print(PS4.getX(i));
            Serial.print(F("\tY")); Serial.print(i + 1); Serial.print(F(": "));
            Serial.print(PS4.getY(i));
            Serial.print(F("\t"));
          }
        }
      }
    }
  } else {
    Serial.println(F("Controller is not connected"));
  }
}

/*
 * Outputs codes from the MP3 card into human readable text
 */
void mp3Detail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F(" Play Finished: "));
      break;
    case DFPlayerFeedBack:
      Serial.println(F(" Feedback"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  } 
}

/*
 * Basic way to turn a servo, send it a raw pulse
 * We don't use this anymore because we use ServoEasing
 */
void setServoPulse(uint8_t n, double pulse) {
  double pulselength;
  pulselength = 1000000;   // 1,000,000 us per second
  pulselength /= 50;   // 50 Hz
  pulselength /= 4096;  // 12 bits of resolution
  pulse /= pulselength;
  pwm.setPWM(n, 0, pulse);
}
