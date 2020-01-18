# MP3 DFPlayer Mini
This sketch will play all the files on an SD Card and will let you test that the mp3 files are loaded properly and the card works.

## Installation
Insert the SD Card and load the code.  Open the Serial Monitor and you should see the card being initialized and all the 
tracks should be played in order

![DFPlayer Serial](https://raw.githubusercontent.com/redheadedstep/padawanps4/master/General/MP3/MP3_Serial.png)

## Load files
To load the files, follow these instructions:

- Insert the SD Card into your computer and create a folder called `mp3`.
- Copy all the MP3 files to the `mp3` folder
- Name all the MP3 files as `0001.mp3`, `0002.mp3`, etc

![MP3 Folder](https://raw.githubusercontent.com/redheadedstep/padawanps4/master/General/MP3/MP3_Folder_1.png)

## Connect the board
Connect the board with the following pins:

- Arduino Digital Pin 10 => TX
- Arduino Digital Pin 11 => RX
- Arduino 3.3V (or 5V) => VCC
- Arduino Ground => Ground (Same side as VCC)
- Speaker Pin #1 => Speaker+
- Speaker Pin #2 => Speaker-

![DFPlayer Mini Pins](https://raw.githubusercontent.com/redheadedstep/padawanps4/master/General/MP3/DFPlayer_Mini_Pin_1.png)

## Troubleshooting
### Timeouts or nothing plays
If the card doesn't have enough power or you try to load too many files, it may timeout.  If that happens, increase the timeout
with the `setTimeOut()` function.

### Card cannot be started
Some knock-off cards exist.  Verify that your card says *DFPlayer Mini* on it.  If it says *MP3-TF-16P*, then it may be a 
knock-off card and is not guaranteed to work or support all the library functions.
