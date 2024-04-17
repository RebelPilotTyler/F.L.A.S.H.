/*F.L.A.S.H. | Flashing Lights and Amplified Sound Housing | Program
F.L.A.S.H. is a modular system that attaches onto the side rail and muzzle of a Nerf blaster.
It adds customizable lights and sound to any blaster!
Made for SIP (Student Innovation Project) at the University of Advancing Technology.
Written by Tyler Widener, tylerwidenerlm@gmail.com. Used ChatGPT for researching programming functions and techniques.

=========PINS==========
---LCD:
Register Select: A0
Enable: A1
D4: A2
D5: A3
D6: A4
D7: A5

--SD Card Module:
CS: 10 (14)
SCK: 13
MOSI: 11
MISO: 12

--Rotary Encoder:
SW: 7
DT (B): 4
CLK (A): 2

--Misc:
Speaker (Connected via LM386 Amp): 9
Light Green: 6
Light Red: 5
Light Blue: 3
IR Sensor: 8
*/

//======PIN DECLARATIONS======
const int speaker = 9;
const int lightR = 5;
const int lightG = 6;
const int lightB = 3;
const int ir = 8;
const int channelA = 2;
const int channelB = 4;
const int sw = 7;

//=========LIBRARIES==========
#include <LiquidCrystal.h>  //For LCD
#include <TMRpcm.h>         //For Speaker. Documentation link: https://github.com/TMRh20/TMRpcm/wiki
#include <pcmConfig.h>
#include <pcmRF.h>
#include <SPI.h>  //For SD Card
#include <SD.h>
#include <string.h>

//=========VARIABLES==========
char encoderState = '0';  //For keeping track of the encoder's state. 0 is neutral, r is turning right and l is turning left.
int lastButtonState = 0;

TMRpcm audio;
String selectedAudio = "";

String mainMenuItems[] = { "Color", "Sound", "Volume" };  //Array of the Items in the Main Menu
int currentMainMenuItem = 0;                              //Index of current main menu item

int currentRedValue = 255;
int currentGreenValue = 0;
int currentBlueValue = 0;

int currentVolume = 7;

File root;
File entry;

//=======INITIALIZATIONS======
LiquidCrystal lcd_1(A0, A1, A2, A3, A4, A5);

//=========FUNCTIONS==========
void clearScreen() {  //Clears the screen of the LCD
  lcd_1.setCursor(0, 0);
  lcd_1.print("                ");
  lcd_1.setCursor(0, 1);
  lcd_1.print("                ");
}

void encoderPulse() {  //Interrupt function for advancing the encoder up or down.
  int aState = digitalRead(channelA);
  int bState = digitalRead(channelB);
  if (aState != bState) {  //If the encoder is turned clockwise, right.
    encoderState = 'r';
  } else if (aState == bState) {  //If the encoder is turned counterclockwise, left.
    encoderState = 'l';
  }
  delay(120);
}

void mainMenu() {  //Function that contains the main menu
  delay(100);
  while (true) {
    flash();
    clearScreen();
    lcd_1.setCursor(0, 0);
    lcd_1.print((mainMenuItems[currentMainMenuItem] + " <--"));
    lcd_1.setCursor(0, 1);
    if (currentMainMenuItem == 2) {
      lcd_1.print(mainMenuItems[0]);
    } else {
      lcd_1.print(mainMenuItems[currentMainMenuItem + 1]);
    }
    if (encoderState == 'r') {
      encoderState = '0';
      currentMainMenuItem += 1;
      if (currentMainMenuItem == 3) {
        currentMainMenuItem = 0;
      }
    } else if (encoderState == 'l') {
      encoderState = '0';
      currentMainMenuItem -= 1;
      if (currentMainMenuItem == -1) {
        currentMainMenuItem = 2;
      }
    }
    if (digitalRead(sw) == LOW) {
      delay(500);
      if (currentMainMenuItem == 0) {  //If the color option is selected
        colorMenu();
      } else if (currentMainMenuItem == 1) {  //If the sound option is selected
        soundMenu();
      } else if (currentMainMenuItem == 2) {  //If the volume option is selected
        volumeMenu();
      }
    }
  }
}

void colorMenu() {
  String colorMenuItems[] = { "<- Back", "Red", "Orange", "Yellow", "Green", "Blue", "Purple", "Custom" };
  int currentColorMenuItem = 0;
  delay(100);
  while (true) {
    flash();
    clearScreen();
    lcd_1.setCursor(0, 0);
    lcd_1.print((colorMenuItems[currentColorMenuItem] + " <--"));
    lcd_1.setCursor(0, 1);
    if (currentColorMenuItem == 7) {
      lcd_1.print(colorMenuItems[0]);
    } else {
      lcd_1.print(colorMenuItems[currentColorMenuItem + 1]);
    }
    if (encoderState == 'r') {
      encoderState = '0';
      currentColorMenuItem += 1;
      if (currentColorMenuItem == 8) {
        currentColorMenuItem = 0;
      }
    } else if (encoderState == 'l') {
      encoderState = '0';
      currentColorMenuItem -= 1;
      if (currentColorMenuItem == -1) {
        currentColorMenuItem = 7;
      }
    }
    if (digitalRead(sw) == LOW) {
      delay(500);
      if (currentColorMenuItem == 0) {  //If the back option is selected, return to main menu
        //mainMenu();
        return;
      } else if (currentColorMenuItem == 1) {  //If red is selected, set color to red and return to main menu
        currentRedValue = 255;
        currentGreenValue = 0;
        currentBlueValue = 0;
        //mainMenu();
        return;
      } else if (currentColorMenuItem == 2) {  //If orange is selected, set color to orange and return to main menu
        currentRedValue = 247;
        currentGreenValue = 105;
        currentBlueValue = 2;
        //mainMenu();
        return;
      } else if (currentColorMenuItem == 3) {  //If yellow is selected, set color to yellow and return to main menu
        currentRedValue = 247;
        currentGreenValue = 223;
        currentBlueValue = 2;
        //mainMenu();
        return;
      } else if (currentColorMenuItem == 4) {  //If green is selected, set color to green and return to main menu
        currentRedValue = 0;
        currentGreenValue = 255;
        currentBlueValue = 0;
        //mainMenu();
        return;
      } else if (currentColorMenuItem == 5) {  //If blue is selected, set color to blue and return to main menu
        currentRedValue = 0;
        currentGreenValue = 0;
        currentBlueValue = 255;
        //mainMenu();
        return;
      } else if (currentColorMenuItem == 6) {  //If purple is selected, set color to purple and return to main menu
        currentRedValue = 110;
        currentGreenValue = 0;
        currentBlueValue = 189;
        //mainMenu();
        return;
      } else if (currentColorMenuItem == 7) {  //If custom is selected, go to the custom color menu
        customColorMenu();
      }
    }
  }
}

void customColorMenu() {  //Function that controls the custom color menu
  String customColorMenuItems[] = { "<- Main Menu", "Red Value", "Green Value", "Blue Value" };
  int currentCustomColorItem = 0;
  delay(100);
  while (true) {
    flash();
    clearScreen();
    lcd_1.setCursor(0, 0);
    lcd_1.print((customColorMenuItems[currentCustomColorItem] + " <--"));
    lcd_1.setCursor(0, 1);
    if (currentCustomColorItem == 3) {
      lcd_1.print(customColorMenuItems[0]);
    } else {
      lcd_1.print(customColorMenuItems[currentCustomColorItem + 1]);
    }
    if (encoderState == 'r') {
      encoderState = '0';
      currentCustomColorItem += 1;
      if (currentCustomColorItem == 4) {
        currentCustomColorItem = 0;
      }
    } else if (encoderState == 'l') {
      encoderState = '0';
      currentCustomColorItem -= 1;
      if (currentCustomColorItem == -1) {
        currentCustomColorItem = 3;
      }
    }
    if (digitalRead(sw) == LOW) {
      delay(500);
      if (currentCustomColorItem == 0) {  //If the back option is selected, return to main menu
        //mainMenu();
        return;
      } else if (currentCustomColorItem == 1) {  //If red is selected, let user adjust it and then return to custom color menu
        while (digitalRead(sw) != LOW) {
          clearScreen();
          lcd_1.setCursor(0, 0);
          lcd_1.print("Red: " + String(currentRedValue));
          if (encoderState == 'r') {
            encoderState = '0';
            if (currentRedValue == 255) {

            } else {
              currentRedValue += 1;
            }
          } else if (encoderState == 'l') {
            encoderState = '0';
            if (currentRedValue == 0) {

            } else {
              currentRedValue -= 1;
            }
          }
        }
        customColorMenu();
      } else if (currentCustomColorItem == 2) {  //If green is selected, let user adjust it and then return to custom color menu
        while (digitalRead(sw) != LOW) {
          clearScreen();
          lcd_1.setCursor(0, 0);
          lcd_1.print("Green: " + String(currentGreenValue));
          if (encoderState == 'r') {
            encoderState = '0';
            if (currentGreenValue == 255) {

            } else {
              currentGreenValue += 1;
            }
          } else if (encoderState == 'l') {
            encoderState = '0';
            if (currentGreenValue == 0) {

            } else {
              currentGreenValue -= 1;
            }
          }
        }
        customColorMenu();
      } else if (currentCustomColorItem == 3) {  //If blue is selected, let user adjust it and then return to custom color menu
        while (digitalRead(sw) != LOW) {
          clearScreen();
          lcd_1.setCursor(0, 0);
          lcd_1.print("Blue: " + String(currentBlueValue));
          if (encoderState == 'r') {
            encoderState = '0';
            if (currentBlueValue == 255) {

            } else {
              currentBlueValue += 1;
            }
          } else if (encoderState == 'l') {
            encoderState = '0';
            if (currentBlueValue == 0) {

            } else {
              currentBlueValue -= 1;
            }
          }
        }
        customColorMenu();
      }
    }
  }
}

void soundMenu() {  //Function that controls the sound menu
  noInterrupts();
  delay(500);
  String soundMenuItems[3] = { "<- Back", "", "" };
  int currentSoundMenuItem = 0;
  int menuSize = 0;
  root = SD.open("/");  //Initial Section Opens SD card, and fills an array with all of it's filenames
  if (!root) {
    clearScreen();
    lcd_1.print("SD Failure");
    delay(3000);
    //mainMenu();
    return;
  }
  while (true) {  //While loop that adds array items.
    clearScreen();
    menuSize += 1;
    flash();
    entry = root.openNextFile();
    if (!entry) {
      break;
    } else if (menuSize == 10) {
      break;
    }
    lcd_1.print(entry.name());
    soundMenuItems[menuSize] = entry.name();
    entry.close();
    delay(1000);
  }
  interrupts();
  while (true) {  //This while loop holds the actual sound menu
    delay(100);
    flash();
    clearScreen();
    lcd_1.setCursor(0, 0);
    lcd_1.print((soundMenuItems[currentSoundMenuItem] + " <--"));
    lcd_1.setCursor(0, 1);
    if (currentSoundMenuItem == (menuSize - 1)) {
      lcd_1.print(soundMenuItems[0]);
    } else {
      lcd_1.print(soundMenuItems[currentSoundMenuItem + 1]);
    }
    if (encoderState == 'r') {
      encoderState = '0';
      currentSoundMenuItem += 1;
      if (currentSoundMenuItem == (menuSize)) {
        currentSoundMenuItem = 0;
      }
    } else if (encoderState == 'l') {
      encoderState = '0';
      currentSoundMenuItem -= 1;
      if (currentSoundMenuItem == -1) {
        currentSoundMenuItem = (menuSize - 1);
      }
    }
    if (digitalRead(sw) == LOW) {
      delay(100);
      if (currentSoundMenuItem == 0) {
        //mainMenu();
        return;
      } else {
        selectedAudio = soundMenuItems[currentSoundMenuItem];
        //mainMenu();
        return;
      }
    }
  }
}

void volumeMenu() {  //Function that controls the volume menu
  delay(100);
  while (digitalRead(sw) != LOW) {
    flash();
    clearScreen();
    lcd_1.setCursor(0, 0);
    lcd_1.print("Volume: " + String(currentVolume));
    if (encoderState == 'r') {
      encoderState = '0';
      if (currentVolume == 7) {

      } else {
        audio.setVolume(currentVolume + 1);
        currentVolume += 1;
      }
    } else if (encoderState == 'l') {
      encoderState = '0';
      if (currentVolume == 1) {

      } else {
        audio.setVolume(currentVolume - 1);
        currentVolume -= 1;
      }
    }
  }
  //mainMenu();
  return;
}

void flash() {  //Function that needs to be called in every loop. Self-evaluates if there is a need to flash or not.
  noInterrupts();
  if (digitalRead(ir) == LOW) {
    audio.play(selectedAudio.c_str());
    //audio.play("EL16HFE.wav");
    analogWrite(lightR, currentRedValue);
    analogWrite(lightG, currentGreenValue);
    analogWrite(lightB, currentBlueValue);
    delay(200);
    analogWrite(lightR, 0);
    analogWrite(lightG, 0);
    analogWrite(lightB, 0);
  } else {
    interrupts();
    return;
  }
  interrupts();
}

void test() {  //A function purely for testing various properties
  while (true) {
    lcd_1.setCursor(0, 1);
    lcd_1.print("                ");
    lcd_1.setCursor(0, 1);
    lcd_1.print(SD.exists("EL16HFE.wav"));
    if (SD.exists("EL16HFE.wav")) {
      audio.play("EL16HFE.wav");
    }
    delay(1000);
  }
}

//===========MAIN=============
void setup() {
  lcd_1.begin(16, 2);     //Starts the LCD
  lcd_1.setCursor(0, 0);  //Sets the cursor for the LCD. First # is column, second is row.
  if (SD.begin(10) == 0) {
    lcd_1.print("SD Failure 1");
    return;
  }
  lcd_1.print("F. L. A. S. H.");

  //======VARIABLE SETUP======
  pinMode(lightG, OUTPUT);
  pinMode(lightR, OUTPUT);
  pinMode(lightB, OUTPUT);
  pinMode(speaker, OUTPUT);
  pinMode(ir, INPUT_PULLUP);
  pinMode(channelB, INPUT);
  pinMode(sw, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(channelA), encoderPulse, CHANGE);
  audio.speakerPin = 9;
  audio.setVolume(7);
  audio.quality(0);

  delay(3000);

  root = SD.open("/");  //A couple lines to set the default audio track to the first one listed on the SD card.
  if (!root) {
    lcd_1.print("SD Failure 2");
    delay(3000);
    return;
  }
  entry = root.openNextFile();
  selectedAudio = entry.name();
  entry.close();
  entry = root.openNextFile();
  selectedAudio = entry.name();
  entry.close();
  selectedAudio[strlen(selectedAudio.c_str()) - 1] = 'v';
  selectedAudio[strlen(selectedAudio.c_str()) - 2] = 'a';
  selectedAudio[strlen(selectedAudio.c_str()) - 3] = 'w';
  lcd_1.setCursor(0, 1);
  lcd_1.print(selectedAudio.c_str());
  delay(2000);

  //mainMenu();
}
void loop() {
  flash();
  clearScreen();
  lcd_1.setCursor(0, 0);
  lcd_1.print((mainMenuItems[currentMainMenuItem] + " <--"));
  lcd_1.setCursor(0, 1);
  if (currentMainMenuItem == 2) {
    lcd_1.print(mainMenuItems[0]);
  } else {
    lcd_1.print(mainMenuItems[currentMainMenuItem + 1]);
  }
  if (encoderState == 'r') {
    encoderState = '0';
    currentMainMenuItem += 1;
    if (currentMainMenuItem == 3) {
      currentMainMenuItem = 0;
    }
  } else if (encoderState == 'l') {
    encoderState = '0';
    currentMainMenuItem -= 1;
    if (currentMainMenuItem == -1) {
      currentMainMenuItem = 2;
    }
  }
  if (digitalRead(sw) == LOW) {
    delay(500);
    if (currentMainMenuItem == 0) {  //If the color option is selected
      colorMenu();
    } else if (currentMainMenuItem == 1) {  //If the sound option is selected
      soundMenu();
    } else if (currentMainMenuItem == 2) {  //If the volume option is selected
      //volumeMenu();
      delay(100);
      while (digitalRead(sw) != LOW) {
        flash();
        clearScreen();
        lcd_1.setCursor(0, 0);
        lcd_1.print("Volume: " + String(currentVolume));
        if (encoderState == 'r') {
          encoderState = '0';
          if (currentVolume == 7) {

          } else {
            audio.setVolume(currentVolume + 1);
            currentVolume += 1;
          }
        } else if (encoderState == 'l') {
          encoderState = '0';
          if (currentVolume == 1) {

          } else {
            audio.setVolume(currentVolume - 1);
            currentVolume -= 1;
          }
        }
      }
    }
  }
}