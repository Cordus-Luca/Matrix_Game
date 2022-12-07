#include <LiquidCrystal.h>
#include <LedControl.h>
#include <EEPROM.h>
#include <string.h>
#include <AceSorting.h>

#define LCD_Backlight 3

using ace_sorting::shellSortKnuth;

const byte RS = 9;
const byte enable = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;


const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;
const int pinSW = 2;
const int xPin = A0;
const int yPin = A1;
const int LCDbrightnessPin = 3;
const int buzzerPin = 13;
char text[] = "Github: Cordus-Luca";

int ledState = HIGH;
int buttonState;
int lastButtonState = LOW;
byte buzzState = LOW;

int xValue;
int yValue;

byte blinkIndex = 1;
byte xJoyMoved = false;
byte yJoyMoved = false;
byte lastSwState = LOW;
byte swState = LOW;
byte reading = HIGH;
byte joyMoved = false;

unsigned long previousMillis;
unsigned long previousBlinkMillis;
unsigned long selectBlinkDelay = 300;
unsigned long debounceDelay = 50;
unsigned long lastDebounceTime = 0;
LiquidCrystal lcd(RS, enable, d4, d5, d6, d7);
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);  // DIN, CLK, LOAD, No. DRIVER

byte xPos = 0;
byte yPos = 0;
byte xLastPos = 0;
byte yLastPos = 0;
byte blinkState = false;

const int minThreshold = 200;
const int maxThreshold = 600;

const byte moveInterval = 100;
unsigned long long lastMoved = 0;

const byte matrixSize = 8;
bool matrixChanged = true;
bool buttonValue = true;

int nameXValue;
int nameState = 0;
int menuState = 1;
int startGameMenuState = 1;
int highscoreMenuState = 1;
int settingsMenuState = 1;
int aboutMenuState = 1;
int HTPMenuState = 1;
bool inMenu = false;
int lives = 1;
int level = 1;
int LCDbrightness = 20;     // > 0 && < 25
byte matrixBrightness = 7;  // > 0 && < 15
int sound = 1;

byte menuOptions[] = {
  0, 0, 0, 0, 0  // START GAME, HIGHSCORE, SETTINGS, ABOUT, HOW TO PLAY
};

byte matrix[matrixSize][matrixSize] = {
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 }
};

byte downArrow[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b11111,
  0b01110,
  0b00100,
  0b00000,
  0b00000
};

byte upArrow[8] = {
  0b00000,
  0b00000,
  0b00100,
  0b01110,
  0b11111,
  0b00000,
  0b00000,
  0b00000
};

byte upDownArrow[8] = {
  0b00100,
  0b01110,
  0b11111,
  0b00000,
  0b00000,
  0b11111,
  0b01110,
  0b00100
};

byte selectArrow[8] = {
  0b00000,
  0b00100,
  0b00110,
  0b11111,
  0b11111,
  0b00110,
  0b00100,
  0b00000
};

byte leftRightArrow[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b11011,
  0b11011,
  0b01010,
  0b00000,
  0b00000
};

byte rightArrow[8] = {
  0b00000,
  0b01000,
  0b01100,
  0b01110,
  0b01110,
  0b01100,
  0b01000,
  0b00000
};

byte leftArrow[8] = {
  0b00000,
  0b00010,
  0b00110,
  0b01110,
  0b01110,
  0b00110,
  0b00010,
  0b00000
};

byte heart[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};

byte underline[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b11111
};

struct player {
  char name[3] = { 'A', 'B', 'C' };
  int score = 0;
} stockPlayer, firstPlayer, secondPlayer, thirdPlayer, fourthPlayer, fifthPlayer;

char currentPlayer[3] = { 'T', 'S', 'T' };
int currentScore = 500;

player players[11];

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);


  pinMode(LCDbrightnessPin, OUTPUT);
  pinMode(pinSW, INPUT_PULLUP);

  updateMenu();
  lc.shutdown(0, false);  // turn off power saving, enables display
  lc.clearDisplay(0);     // clear screen

  sound = EEPROM.read(0);
  matrixBrightness = EEPROM.read(12);
  LCDbrightness = EEPROM.read(11);
  firstPlayer.name[0] = EEPROM.read(40);
  firstPlayer.name[1] = EEPROM.read(1);
  firstPlayer.name[2] = EEPROM.read(2);
  firstPlayer.score = EEPROM.read(3);
  secondPlayer.name[0] = EEPROM.read(4);
  secondPlayer.name[1] = EEPROM.read(5);
  secondPlayer.name[2] = EEPROM.read(6);
  secondPlayer.score = EEPROM.read(7);
  thirdPlayer.name[0] = EEPROM.read(8);
  thirdPlayer.name[1] = EEPROM.read(9);
  thirdPlayer.name[2] = EEPROM.read(10);
  thirdPlayer.score = EEPROM.read(31);
  fourthPlayer.name[0] = EEPROM.read(32);
  fourthPlayer.name[1] = EEPROM.read(33);
  fourthPlayer.name[2] = EEPROM.read(34);
  fourthPlayer.score = EEPROM.read(35);
  fifthPlayer.name[0] = EEPROM.read(36);
  fifthPlayer.name[1] = EEPROM.read(37);
  fifthPlayer.name[2] = EEPROM.read(38);
  fifthPlayer.score = EEPROM.read(39);

  matrix[xPos][yPos] = 1;
}

void loop() {
  EEPROM.update(11, LCDbrightness);
  EEPROM.update(12, matrixBrightness);  // 12 = matrixBrightness
  EEPROM.update(0, sound);

  lcd.setCursor(0, 1);
  lc.setIntensity(0, EEPROM.read(12));                // Setting matrix brightness
  analogWrite(LCDbrightnessPin, LCDbrightness * 10);  // Setting LCD backlight brightness
  if (millis() < 3000) {
    lcd.setCursor(0, 0);
    lcd.print("Hello gamer!");
    lcd.setCursor(0, 1);
    lcd.print("                 ");
  } else {
    buttonPress();
    inMenu = buttonValue;

    if (inMenu == false) {
      updateMenu();
      navigate();
      settingsMenuState = 1;
      nameState = 0;
    } else {
      enterMenu();
    }
  }
}

void updatePlayers() {
  EEPROM.update(40, (char)players[0].name[0]);
  EEPROM.update(1, (char)players[0].name[1]);
  EEPROM.update(2, (char)players[0].name[2]);
  EEPROM.update(3, players[0].score);
  EEPROM.update(4, (char)players[1].name[0]);
  EEPROM.update(5, (char)players[1].name[1]);
  EEPROM.update(6, (char)players[1].name[2]);
  EEPROM.update(7, players[1].score);
  EEPROM.update(8, (char)players[2].name[0]);
  EEPROM.update(9, (char)players[2].name[1]);
  EEPROM.update(10, (char)players[2].name[2]);
  EEPROM.update(31, players[2].score);
  EEPROM.update(32, (char)players[3].name[0]);
  EEPROM.update(33, (char)players[3].name[1]);
  EEPROM.update(34, (char)players[3].name[2]);
  EEPROM.update(35, players[3].score);
  EEPROM.update(36, (char)players[4].name[0]);
  EEPROM.update(37, (char)players[4].name[1]);
  EEPROM.update(38, (char)players[4].name[2]);
  EEPROM.update(39, players[4].score);
}

void updateMenu() {

  lcd.createChar(1, downArrow);
  lcd.createChar(2, selectArrow);
  lcd.createChar(3, upArrow);


  switch (menuState) {
    case 0:
      menuState = 1;
      break;

    case 1:
      lcd.clear();
      lcd.print(" Start game");
      lcd.setCursor(0, 1);
      lcd.print(" Highscore");
      lcd.setCursor(15, 1);
      lcd.write(1);
      lcd.setCursor(0, 0);
      lcd.write(2);
      break;

    case 2:
      lcd.clear();
      lcd.print(" Highscore");
      lcd.setCursor(15, 0);
      lcd.write(3);
      lcd.setCursor(0, 1);
      lcd.print(" Settings");
      lcd.setCursor(15, 1);
      lcd.write(1);
      lcd.setCursor(0, 0);
      lcd.write(2);
      break;

    case 3:
      lcd.clear();
      lcd.print(" Settings");
      lcd.setCursor(15, 0);
      lcd.write(3);
      lcd.setCursor(0, 1);
      lcd.print(" About");
      lcd.setCursor(15, 1);
      lcd.write(1);
      lcd.setCursor(0, 0);
      lcd.write(2);
      break;

    case 4:
      lcd.clear();
      lcd.print(" About");
      lcd.setCursor(15, 0);
      lcd.write(3);
      lcd.setCursor(0, 1);
      lcd.print(" How to play");
      lcd.setCursor(0, 0);
      lcd.write(2);
      break;

    case 5:
      lcd.clear();
      lcd.print(" About");
      lcd.setCursor(15, 0);
      lcd.write(3);
      lcd.setCursor(0, 1);
      lcd.print(" How to play");
      lcd.setCursor(0, 1);
      lcd.write(2);
      break;

    case 6:
      menuState = 5;
      break;
  }
}

void enterMenu() {

  switch (menuState) {
    case 1:
      lcd.clear();
      startGame();
      break;

    case 2:
      lcd.clear();
      navigateHighScore();
      highscore();
      break;

    case 3:
      lcd.clear();
      navigateSettings();
      settings();
      break;

    case 4:
      lcd.clear();
      navigateAbout();
      about();
      break;

    case 5:
      lcd.clear();
      howToPlay();
      break;
  }
}

void startGame() {
  lcd.createChar(2, selectArrow);
  lcd.createChar(4, heart);
  lcd.clear();
  lcd.print("Level: ");
  lcd.print(level);
  lcd.setCursor(0, 1);
  lcd.print(" : ");
  lcd.print(lives);
  lcd.setCursor(0, 1);
  lcd.write(4);
  lcd.setCursor(7, 1);
  lcd.print("Nume: ");
  lcd.print(currentPlayer[0]);
  lcd.print(currentPlayer[1]);
  lcd.print(currentPlayer[2]);

  if (millis() - lastMoved > moveInterval) {
    updatePositions();  // calculare stare
  }

  if (matrixChanged == true) {
    updateMatrix();
    matrixChanged = false;
  }
  delay(10);
}

void highscore() {
  Serial.println(players[0].score);
  players[0] = firstPlayer;
  players[1] = secondPlayer;
  players[2] = thirdPlayer;
  players[3] = fourthPlayer;
  players[4] = fifthPlayer;
  players[5].name[0] = currentPlayer[0];
  players[5].name[1] = currentPlayer[1];
  players[5].name[2] = currentPlayer[2];
  players[5].score = currentScore;
  shellSortKnuth(players, 6, sortareHighscore);
  updatePlayers();
  switch (highscoreMenuState) {
    case 0:
      highscoreMenuState = 1;
      break;

    case 1:
      lcd.createChar(2, selectArrow);
      lcd.clear();
      lcd.print("1. ");
      lcd.print(players[0].name[0]);
      lcd.print(players[0].name[1]);
      lcd.print(players[0].name[2]);
      lcd.print(" Scor:");
      lcd.print(players[0].score);
      lcd.setCursor(0, 1);
      lcd.print("2. ");
      lcd.print(players[1].name[0]);
      lcd.print(players[1].name[1]);
      lcd.print(players[1].name[2]);
      lcd.print(" Scor:");
      lcd.print(players[1].score);
      break;

    case 2:
      lcd.createChar(2, selectArrow);
      lcd.clear();
      lcd.print("2. ");
      lcd.print(players[1].name[0]);
      lcd.print(players[1].name[1]);
      lcd.print(players[1].name[2]);
      lcd.print(" Scor:");
      lcd.print(players[1].score);
      lcd.setCursor(0, 1);
      lcd.print("3. ");
      lcd.print(players[2].name[0]);
      lcd.print(players[2].name[1]);
      lcd.print(players[2].name[2]);
      lcd.print(" Scor:");
      lcd.print(players[2].score);
      break;

    case 3:
      lcd.createChar(2, selectArrow);
      lcd.clear();
      lcd.print("3. ");
      lcd.print(players[2].name[0]);
      lcd.print(players[2].name[1]);
      lcd.print(players[2].name[2]);
      lcd.print(" Scor:");
      lcd.print(players[2].score);
      lcd.setCursor(0, 1);
      lcd.print("4. ");
      lcd.print(players[3].name[0]);
      lcd.print(players[3].name[1]);
      lcd.print(players[3].name[2]);
      lcd.print(" Scor:");
      lcd.print(players[3].score);
      break;

    case 4:
      lcd.createChar(2, selectArrow);
      lcd.clear();
      lcd.print("4. ");
      lcd.print(players[3].name[0]);
      lcd.print(players[3].name[1]);
      lcd.print(players[3].name[2]);
      lcd.print(" Scor:");
      lcd.print(players[3].score);
      lcd.setCursor(0, 1);
      lcd.print("5. ");
      lcd.print(players[4].name[0]);
      lcd.print(players[4].name[1]);
      lcd.print(players[4].name[2]);
      lcd.print(" Scor:");
      lcd.print(players[4].score);
      break;

    case 5:
      highscoreMenuState = 4;
      break;
  }

  delay(10);
}

bool sortareHighscore(player first, player second) {
  return first.score > second.score;
}

void settings() {

  switch (settingsMenuState) {
    case 0:
      settingsMenuState = 1;
      break;

    case 1:
      lcd.createChar(1, downArrow);
      lcd.createChar(2, selectArrow);
      lcd.clear();
      lcd.print(" Settings menu");
      lcd.setCursor(0, 0);
      lcd.write(2);
      lcd.setCursor(0, 1);
      lcd.write(" Name: ");
      lcd.print(currentPlayer[0]);
      lcd.print(currentPlayer[1]);
      lcd.print(currentPlayer[2]);
      lcd.setCursor(15, 1);
      lcd.write(1);
      break;

    case 2:  // NAME
      lcd.createChar(1, downArrow);
      lcd.createChar(2, selectArrow);
      lcd.createChar(3, upArrow);
      lcd.createChar(5, upDownArrow);
      lcd.createChar(4, underline);
      lcd.createChar(7, rightArrow);
      lcd.clear();
      lcd.write(" Name: ");
      lcd.print(currentPlayer[0]);
      lcd.print(currentPlayer[1]);
      lcd.print(currentPlayer[2]);
      lcd.setCursor(6, 0);
      lcd.write(7);
      lcd.setCursor(0, 0);
      lcd.write(2);
      lcd.setCursor(11, 0);
      lcd.write(5);
      lcd.setCursor(15, 0);
      lcd.write(3);
      lcd.setCursor(15, 1);
      lcd.write(1);
      lcd.setCursor(0, 1);
      lcd.write(" LCD Bright:");
      lcd.setCursor(12, 1);
      lcd.print(LCDbrightness);

      if (millis() - previousBlinkMillis >= selectBlinkDelay) {

        previousBlinkMillis = millis();

        if (nameState) {
          blinkState = !blinkState;
        }
      }
      if (nameState) {
        if (blinkState) {
          lcd.setCursor(6 + nameState, 0);
          lcd.write(4);
        } else {
          lcd.setCursor(6 + nameState, 0);
          lcd.print(currentPlayer[nameState - 1]);
        }
      }

      xValue = analogRead(xPin);

      if (xJoyMoved == false) {
        if (xValue > maxThreshold) {
          if (nameState < 3) {
            nameState++;
          }
          xJoyMoved = true;
        }
        if (xValue < minThreshold) {
          if (nameState > 0) {
            nameState--;
          }
          xJoyMoved = true;
        }
      }

      if (xValue < maxThreshold && xValue > minThreshold) {
        xJoyMoved = false;
      }

      if (joyMoved == false) {
        if (yValue > maxThreshold) {
          if (currentPlayer[nameState - 1] != 'A') {
            currentPlayer[nameState - 1] = (char)(currentPlayer[nameState - 1] - 1);
          } else {
            currentPlayer[nameState - 1] = 'Z';
          }
          joyMoved = true;
        }
        if (yValue < minThreshold) {
          if (currentPlayer[nameState - 1] != 'Z') {
            currentPlayer[nameState - 1] = (char)(currentPlayer[nameState - 1] + 1);
          } else {
            currentPlayer[nameState - 1] = 'A';
          }
          joyMoved = true;
        }
      }

      if (yValue > minThreshold && yValue < maxThreshold) {
        joyMoved = false;
      }

      break;

    case 3:  // LCD BRIGHTNESS CONTROL
      lcd.createChar(1, downArrow);
      lcd.createChar(2, selectArrow);
      lcd.createChar(7, rightArrow);
      lcd.createChar(8, leftArrow);
      for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < matrixSize; j++) {
          matrix[i][j] = 0;
        }
      }

      matrix[xPos][yPos] = 1;

      updateMatrix();

      lcd.clear();
      lcd.write(" LCD Bright:");
      lcd.setCursor(0, 0);
      lcd.write(2);
      lcd.setCursor(14, 0);
      lcd.write(8);
      lcd.setCursor(15, 0);
      lcd.write(7);
      lcd.setCursor(15, 1);
      lcd.write(1);
      lcd.setCursor(0, 1);
      lcd.write(" Matr Bright:");
      lcd.setCursor(12, 0);
      lcd.print(LCDbrightness);
      lcd.setCursor(13, 1);
      lcd.print(matrixBrightness);

      xValue = analogRead(xPin);

      if (xJoyMoved == false) {
        if (xValue > maxThreshold) {
          if (LCDbrightness < 25) {
            LCDbrightness++;
            EEPROM.update(11, LCDbrightness);
          }
          xJoyMoved = true;
        }
        if (xValue < minThreshold) {
          if (LCDbrightness > 0) {
            LCDbrightness--;
            EEPROM.update(11, LCDbrightness);
          }
          xJoyMoved = true;
        }
      }

      if (xValue < maxThreshold && xValue > minThreshold) {
        xJoyMoved = false;
      }

      break;

    case 4:  // MATRIX BRIGHTNESS CONTROL
      lcd.createChar(1, downArrow);
      lcd.createChar(2, selectArrow);
      lcd.createChar(6, leftRightArrow);
      for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < matrixSize; j++) {
          matrix[i][j] = 1;
        }
      }

      updateMatrix();

      lcd.clear();
      lcd.print(" Matr Bright:");
      lcd.setCursor(0, 0);
      lcd.write(2);
      lcd.setCursor(15, 0);
      lcd.write(6);
      lcd.setCursor(15, 1);
      lcd.write(1);
      lcd.setCursor(0, 1);
      lcd.print(" Sound:");
      lcd.setCursor(13, 0);
      lcd.print(matrixBrightness);
      lcd.setCursor(7, 1);
      lcd.print(sound);

      xValue = analogRead(xPin);

      if (xJoyMoved == false) {
        if (xValue > maxThreshold) {
          if (matrixBrightness < 15) {
            matrixBrightness++;
          }
          xJoyMoved = true;
        }
        if (xValue < minThreshold) {
          if (matrixBrightness > 0) {
            matrixBrightness--;
          }
          xJoyMoved = true;
        }
      }

      if (xValue < maxThreshold && xValue > minThreshold) {
        xJoyMoved = false;
      }

      delay(10);

      break;

    case 5:  // SOUND
      lcd.createChar(2, selectArrow);
      lcd.createChar(3, upArrow);
      lcd.createChar(7, rightArrow);
      lcd.createChar(8, leftArrow);
      for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < matrixSize; j++) {
          matrix[i][j] = 0;
        }
      }

      matrix[xPos][yPos] = 1;

      updateMatrix();

      lcd.clear();
      lcd.write(" Sound:");
      lcd.setCursor(15, 0);
      lcd.write(3);
      lcd.setCursor(10, 0);
      lcd.write(8);
      lcd.setCursor(11, 0);
      lcd.write(7);
      lcd.setCursor(0, 1);
      lcd.write(" Lives:");
      lcd.setCursor(0, 0);
      lcd.write(2);
      lcd.setCursor(7, 0);
      lcd.print(sound);
      lcd.setCursor(7, 1);
      lcd.print(lives);

      xValue = analogRead(xPin);

      if (xJoyMoved == false) {
        if (xValue > maxThreshold) {
          if (sound < 1) {
            sound++;
            EEPROM.update(0, sound);
          }
          xJoyMoved = true;
        }
        if (xValue < minThreshold) {
          if (sound > 0) {
            sound--;
            EEPROM.update(0, sound);
          }
          xJoyMoved = true;
        }
      }

      if (xValue < maxThreshold && xValue > minThreshold) {
        xJoyMoved = false;
      }

      break;

    case 6:  // LIVES
      lcd.createChar(2, selectArrow);
      lcd.createChar(3, upArrow);
      lcd.createChar(7, rightArrow);
      lcd.createChar(8, leftArrow);
      lcd.clear();
      lcd.write(" Sound:");
      lcd.setCursor(15, 0);
      lcd.write(3);
      lcd.setCursor(10, 1);
      lcd.write(8);
      lcd.setCursor(11, 1);
      lcd.write(7);
      lcd.setCursor(0, 1);
      lcd.write(" Lives:");
      lcd.setCursor(0, 1);
      lcd.write(2);
      lcd.setCursor(7, 0);
      lcd.print(sound);
      lcd.setCursor(7, 1);
      lcd.print(lives);

      xValue = analogRead(xPin);

      if (xJoyMoved == false) {
        if (xValue > maxThreshold) {
          if (lives < 3) {
            lives++;
          }
          xJoyMoved = true;
        }
        if (xValue < minThreshold) {
          if (lives > 1) {
            lives--;
          }
          xJoyMoved = true;
        }
      }

      if (xValue < maxThreshold && xValue > minThreshold) {
        xJoyMoved = false;
      }
      break;

    case 7:
      settingsMenuState = 6;
      break;
  }

  delay(10);
}

void about() {

  switch (aboutMenuState) {
    case 0:
      aboutMenuState = 1;
      break;
    case 1:
      lcd.createChar(1, downArrow);
      lcd.clear();
      lcd.print("Game Name");
      lcd.setCursor(0, 1);
      lcd.print("Made by Luca");  
      lcd.setCursor(15,1);
      lcd.write(1);  
      break;
    case 2:
      lcd.createChar(3, upArrow);
      lcd.clear();
      lcd.print("Made by Luca");
      lcd.setCursor(0, 1);
      lcd.print("GH: Cordus-Luca");  
      lcd.setCursor(15,0);
      lcd.write(3); 
      break;
    case 3:
      aboutMenuState = 2;
      break;
  }
  
  delay(10);
}

void howToPlay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("How to play menu");
  // int startScroll = millis();
  // lcd.print(" Move dot on matrix");
  // for (int i = 0; i < 15; i ++) {
  //   if ((millis() - startScroll) > 100) {
  //     startScroll = millis();
  //     lcd.scrollDisplayLeft();
  //   }
  // }
  delay(10);
}

void navigate() {
  yValue = analogRead(yPin);

  if (yJoyMoved == false) {
    if (yValue > maxThreshold) {
      menuState++;
      updateMenu();
      makeSound(200);
      yJoyMoved = true;
    }
    if (yValue < minThreshold) {
      menuState--;
      updateMenu();
      makeSound(200);
      yJoyMoved = true;
    }
  }

  if (yValue > minThreshold && yValue < maxThreshold) {
    yJoyMoved = false;
  }
}

void navigateSettings() {

  yValue = analogRead(yPin);

  if (nameState == 0) {
    if (joyMoved == false) {
      if (yValue > maxThreshold) {
        settingsMenuState++;
        makeSound(200);
        joyMoved = true;
      }
      if (yValue < minThreshold) {
        settingsMenuState--;
        makeSound(200);
        joyMoved = true;
      }
    }

    if (yValue > minThreshold && yValue < maxThreshold) {
      joyMoved = false;
    }
  }
}

void navigateHighScore() {
  yValue = analogRead(yPin);

  if (nameState == 0) {
    if (joyMoved == false) {
      if (yValue > maxThreshold) {
        highscoreMenuState++;
        makeSound(200);
        joyMoved = true;
      }
      if (yValue < minThreshold) {
        highscoreMenuState--;
        makeSound(200);
        joyMoved = true;
      }
    }

    if (yValue > minThreshold && yValue < maxThreshold) {
      joyMoved = false;
    }
  }
}

void navigateAbout() {
  yValue = analogRead(yPin);

  if (nameState == 0) {
    if (joyMoved == false) {
      if (yValue > maxThreshold) {
        aboutMenuState++;
        makeSound(200);
        joyMoved = true;
      }
      if (yValue < minThreshold) {
        aboutMenuState--;
        makeSound(200);
        joyMoved = true;
      }
    }

    if (yValue > minThreshold && yValue < maxThreshold) {
      joyMoved = false;
    }
  }
}

void buttonPress() {
  int reading = digitalRead(pinSW);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == HIGH) {
        makeSound(200);
        buttonValue = !buttonValue;
      }
    }
  }

  lastButtonState = reading;
}

void makeSound(int toneSound) {
  Serial.println("test");
  // unsigned long currentMillis = millis();
  // if (currentMillis - previousMillis >= 100) {
  //   previousMillis = currentMillis;

  //   // if the LED is off turn it on and vice-versa:
  //   if (buzzState == LOW) {
  //     buzzState = HIGH;
  //   } else {
  //     buzzState = LOW;
  //   }
  // }
  // if (buzzState == LOW) {
  //   noTone(buzzerPin);
  // } else {
  //   tone(buzzerPin, 200);
  // }
  if (sound) {
    tone(buzzerPin, toneSound, 30);
  }
}

void updateMatrix() {
  // TODO: update matrix using byte matrix (refresh function)
  for (int i = 0; i < matrixSize; i++) {
    for (int j = 0; j < matrixSize; j++) {
      lc.setLed(0, i, j, matrix[i][j]);
    }
  }
}

void updatePositions() {
  // TODO: joystick control
  xValue = analogRead(xPin);
  yValue = analogRead(yPin);

  xLastPos = xPos;
  yLastPos = yPos;
  if (xValue < minThreshold) {
    xPos--;
    if (xPos < 0) {
      xPos = 7;
    }
  }

  if (xValue > maxThreshold) {
    xPos++;
    if (xPos > 7) {
      xPos = 0;
    }
  }

  if (yValue > maxThreshold) {
    if (yPos < matrixSize - 1) {
      yPos++;
    } else {
      yPos = 0;
    }
  }

  if (yValue < minThreshold) {
    if (yPos > 0) {
      yPos--;
    } else {
      yPos = matrixSize - 1;
    }
  }

  if (xLastPos != xPos || yLastPos != yPos) {
    matrix[xLastPos][yLastPos] = 0;
    matrix[xPos][yPos] = 1;
    matrixChanged = true;
    lastMoved = millis();
  }
}