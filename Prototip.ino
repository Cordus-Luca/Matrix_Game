#include <LiquidCrystal.h>
#include <LedControl.h>
#include <EEPROM.h>
#include <string.h>
#include <AceSorting.h>

#define LCD_Backlight 3

using ace_sorting::shellSortKnuth;

typedef struct Snake Snake;
struct Snake {
  int head[2];      // HEAD COORD [COL, ROW]
  int body[40][2];  // [LENGTH][ROW, COL]
  int len;          
  int dir[2];       // DIRECTION
};

typedef struct Apple Apple;
struct Apple {
  int rPos;
  int cPos;
};

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
char HTPtext[] = "Use Joystick to move UP, DOWN, LEFT and RIGHT to eat the APPLES.";
char HTP2text[] = "If you eat YOURSELF or a WALL you LOSE!";

int ledState = HIGH;
int buttonState;
int lastButtonState = LOW;
byte buzzState = LOW;

int xValue;
int yValue;
int speed = 1000;

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
int speedLevel = 1;
int level = 1;
int LCDbrightness = 20;     // > 10 && < 25
byte matrixBrightness = 7;  // > 0 && < 15
bool sound = 1;

bool isDead = false;
bool walls = false;

bool dissapear = false;
unsigned int appleDelay = 3;
unsigned int appleTimer = 0;

int i, j;  //Counters

byte matr[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };  // SALVAM MATRICEA IN BINAR

Snake snake = { { 1, 0 }, { { 0, 0 }, { 1, 0 } }, 2, { 1, 0 } };  // STARTING POINT, BODY COORDS, LENGTH, DIRECTION(RIGHT)
Apple apple = { (int)random(0, 8), (int)random(0, 8) };           // Apple coords
float oldTime = 0;
float timer = 0;

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

byte point[8] = {
  0b00000,
  0b01110,
  0b11111,
  0b11111,
  0b11111,
  0b01110,
  0b00000,
  0b00000  
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
int currentScore;

player players[11];

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);


  pinMode(LCDbrightnessPin, OUTPUT);
  pinMode(pinSW, INPUT_PULLUP);

  updateMenu();
  lc.shutdown(0, false); 
  lc.clearDisplay(0); 

  sound = EEPROM.read(0);
  matrixBrightness = EEPROM.read(12);
  LCDbrightness = EEPROM.read(11);
  updatePlayers();

  matrix[xPos][yPos] = 1;
}

void loop() {
  EEPROM.update(11, LCDbrightness);
  EEPROM.update(12, matrixBrightness);  // 12 = matrixBrightness
  EEPROM.update(0, sound);
  //Serial.println(menuState);
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

    if (settingsMenuState == 8 && buttonValue == false) {
      resetHighscore();
    }

    if (settingsMenuState == 8) {
      buttonValue = true;
    }

    inMenu = buttonValue;

    
    
    if (inMenu == false) {
      isDead = false;
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
}

void gameLoop() {
  float deltaTime = calculateDeltaTime();
  timer += deltaTime;

  //Check For Inputs
  int xValue = analogRead(xPin);
  int yValue = analogRead(yPin);

  if (joyMoved == false) {
    if (yValue < minThreshold && snake.dir[1] == 0) {
      Serial.println("SUS");
      snake.dir[0] = 0;  // SUS = SCADE LINIE
      snake.dir[1] = -1;
      joyMoved = true;
    } else if (yValue > maxThreshold && snake.dir[1] == 0) {
      Serial.println("JOS");
      snake.dir[0] = 0;  // JOS = CRESTE LINIE
      snake.dir[1] = 1;
      joyMoved = true;
    } else if (xValue < minThreshold && snake.dir[0] == 0) {
      Serial.println("STANGA");
      snake.dir[0] = -1;  // STANGA = SCADE COLOANA
      snake.dir[1] = 0;
      joyMoved = true;
    } else if (xValue > maxThreshold && snake.dir[0] == 0) {
      Serial.println("DREAPTA");
      snake.dir[0] = 1;  // DREAPTA = CRESTE COLOANA
      snake.dir[1] = 0;
      joyMoved = true;
    }
  }
  if (xValue < maxThreshold && yValue < maxThreshold && xValue > minThreshold && yValue > minThreshold) {
    joyMoved = false;
  }
  if (speedLevel == 1) {
    speed = 500;
  } else if (speedLevel == 2) {
    speed = 350;
  } else if (speedLevel == 3) {
    speed = 150;
  }

  if (timer > speed) {
    timer = 0;
    Update();
  }

  render();
}

void storePlayers() {
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
  if (isDead) {
    lcd.clear();
    lcd.print("Reached score:");
    lcd.print(currentScore);
    delay(1500);
    lcd.clear();
    while(buttonValue) {
      lcd.setCursor(0, 0);
      if (currentScore > firstPlayer.score) {
        lcd.print("New highscore!");
        lcd.setCursor(0, 1);
        lcd.print("Score: ");
        lcd.print(currentScore);
      } else if(currentScore > secondPlayer.score) {
        lcd.print("Second Place!");
        lcd.setCursor(0, 1);
        lcd.print("Score: ");
        lcd.print(currentScore);
      } else if(currentScore > thirdPlayer.score) {
        lcd.print("Third Place!");
        lcd.setCursor(0, 1);
        lcd.print("Score: ");
        lcd.print(currentScore);
      } else if(currentScore > fourthPlayer.score) {
        lcd.print("Fourth Place!");
        lcd.setCursor(0, 1);
        lcd.print("Score: ");
        lcd.print(currentScore);
      } else if(currentScore > fifthPlayer.score) {
        lcd.print("Fifth Place!");
        lcd.setCursor(0, 1);
        lcd.print("Score: ");
        lcd.print(currentScore);
      } else {
        lcd.print("You Lose!");
        lcd.setCursor(0, 1);
        lcd.print("Score:");
        lcd.print(currentScore);
      }
      buttonPress();
    }
    snake = { { 1, 0 }, { { 0, 0 }, { 1, 0 } }, 2, { 1, 0 } };  
    apple = { (int)random(0, 8), (int)random(0, 8) };
    
    if (buttonValue == false) {
      lcd.clear();
      isDead = false;
      
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
      storePlayers();
      updatePlayers();
      currentScore = 0;
      updateMenu();
    }
    delay(20);
  } else {    
    lcd.createChar(2, selectArrow);
    lcd.clear();
    lcd.print("P:");
    lcd.print(currentScore);
    lcd.setCursor(5, 0);
    if (dissapear) {
      lcd.print("D:ON");
    } else {
      lcd.print("D:OFF");
    }

    lcd.setCursor(11,0);
    if (walls) {
      lcd.print("W:ON");
    } else {
      lcd.print("W:OFF");
    }

    if (currentScore > 10 && currentScore < 20 && speedLevel != 3) {
      speedLevel = 2;
    }
    if (currentScore >= 20) {
      speedLevel = 3;
    }
    
    lcd.setCursor(0, 1);
    lcd.print(" :");
    if (speedLevel == 1) {
      lcd.print("Slow");        
    } else if (speedLevel == 2) {
      lcd.print("Fast");
    } else {
      lcd.print("Turbo");
    }
    lcd.setCursor(0, 1);
    lcd.print("S");
    lcd.setCursor(7, 1);
    lcd.print("Nume: ");
    lcd.print(currentPlayer[0]);
    lcd.print(currentPlayer[1]);
    lcd.print(currentPlayer[2]);
    gameLoop();
  }
}

void highscore() {
  lcd.createChar(1, downArrow);
  lcd.createChar(3, upArrow);
  
  switch (highscoreMenuState) {
    case 0:
      highscoreMenuState = 1;
      break;

    case 1:
      lcd.createChar(2, selectArrow);
      lcd.clear();
      lcd.print("1. ");
      lcd.print(firstPlayer.name[0]);
      lcd.print(firstPlayer.name[1]);
      lcd.print(firstPlayer.name[2]);
      lcd.print(" Scor:");
      lcd.print(firstPlayer.score);
      lcd.setCursor(0, 1);
      lcd.print("2. ");
      lcd.print(secondPlayer.name[0]);
      lcd.print(secondPlayer.name[1]);
      lcd.print(secondPlayer.name[2]);
      lcd.print(" Scor:");
      lcd.print(secondPlayer.score);
      lcd.setCursor(15, 1);
      lcd.write(1);
      break;

    case 2:
      lcd.createChar(2, selectArrow);
      lcd.clear();
      lcd.print("2. ");
      lcd.print(secondPlayer.name[0]);
      lcd.print(secondPlayer.name[1]);
      lcd.print(secondPlayer.name[2]);
      lcd.print(" Scor:");
      lcd.print(secondPlayer.score);
      lcd.setCursor(0, 1);
      lcd.print("3. ");
      lcd.print(thirdPlayer.name[0]);
      lcd.print(thirdPlayer.name[1]);
      lcd.print(thirdPlayer.name[2]);
      lcd.print(" Scor:");
      lcd.print(thirdPlayer.score);
      lcd.setCursor(15, 0);
      lcd.write(3);
      lcd.setCursor(15, 1);
      lcd.write(1);
      break;

    case 3:
      lcd.createChar(2, selectArrow);
      lcd.clear();
      lcd.print("3. ");
      lcd.print(thirdPlayer.name[0]);
      lcd.print(thirdPlayer.name[1]);
      lcd.print(thirdPlayer.name[2]);
      lcd.print(" Scor:");
      lcd.print(thirdPlayer.score);
      lcd.setCursor(0, 1);
      lcd.print("4. ");
      lcd.print(fourthPlayer.name[0]);
      lcd.print(fourthPlayer.name[1]);
      lcd.print(fourthPlayer.name[2]);
      lcd.print(" Scor:");
      lcd.print(fourthPlayer.score);
      lcd.setCursor(15, 0);
      lcd.write(3);
      lcd.setCursor(15, 1);
      lcd.write(1);
      break;

    case 4:
      lcd.createChar(2, selectArrow);
      lcd.clear();
      lcd.print("4. ");
      lcd.print(fourthPlayer.name[0]);
      lcd.print(fourthPlayer.name[1]);
      lcd.print(fourthPlayer.name[2]);
      lcd.print(" Scor:");
      lcd.print(fourthPlayer.score);
      lcd.setCursor(0, 1);
      lcd.print("5. ");
      lcd.print(fifthPlayer.name[0]);
      lcd.print(fifthPlayer.name[1]);
      lcd.print(fifthPlayer.name[2]);
      lcd.print(" Scor:");
      lcd.print(fifthPlayer.score);
      lcd.setCursor(15, 0);
      lcd.write(3);
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
      lcd.write(" Name:  ");
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
      lcd.createChar(8, leftArrow);
      lcd.clear();
      lcd.write(" Name:");
      lcd.write(8);
      lcd.write(7);
      lcd.print(currentPlayer[0]);
      lcd.print(currentPlayer[1]);
      lcd.print(currentPlayer[2]);
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
      lcd.print(LCDbrightness - 10);

      if (millis() - previousBlinkMillis >= selectBlinkDelay) {

        previousBlinkMillis = millis();

        if (nameState) {
          blinkState = !blinkState;
        }
      }
      if (nameState) {
        if (blinkState) {
          lcd.setCursor(7 + nameState, 0);
          lcd.write(4);
        } else {
          lcd.setCursor(7 + nameState, 0);
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

      delay(10);

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
      lcd.print(LCDbrightness - 10);
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
          if (LCDbrightness > 10) {
            LCDbrightness--;
            EEPROM.update(11, LCDbrightness);
          }
          xJoyMoved = true;
        }
      }

      if (xValue < maxThreshold && xValue > minThreshold) {
        xJoyMoved = false;
      }
      
      delay(10);

      break;

    case 4:  // MATRIX BRIGHTNESS CONTROL
      lcd.createChar(1, downArrow);
      lcd.createChar(2, selectArrow);
      lcd.createChar(7, rightArrow);
      lcd.createChar(8, leftArrow);
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
      lcd.setCursor(14, 0);
      lcd.write(8);
      lcd.setCursor(15, 0);
      lcd.write(7);
      lcd.setCursor(15, 1);
      lcd.write(1);
      lcd.setCursor(0, 1);
      lcd.print(" Sound:");
      lcd.setCursor(13, 0);
      lcd.print(matrixBrightness);
      lcd.setCursor(7, 1);
      if (sound == 1) {
        lcd.print("ON");
      } else {
        lcd.print("OFF");
      }

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
      lcd.createChar(1, downArrow);
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
      lcd.write(" Speed:");
      lcd.setCursor(0, 0);
      lcd.write(2);
      lcd.setCursor(7, 0);
      if (sound == 1) {
        lcd.print("ON");
      } else {
        lcd.print("OFF");
      }
      lcd.setCursor(7, 1);
      if (speedLevel == 1) {
        lcd.print("Slow");        
      } else if (speedLevel == 2) {
        lcd.print("Fast");
      } else {
        lcd.print("Turbo");
      }
      lcd.setCursor(15, 1);
      lcd.write(1);

      xValue = analogRead(xPin);

      if (xJoyMoved == false) {
        if (xValue > maxThreshold || xValue < minThreshold) {
          sound = !sound;
          EEPROM.update(0, sound);
          xJoyMoved = true;
        }
      }

      if (xValue < maxThreshold && xValue > minThreshold) {
        xJoyMoved = false;
      }

      delay(10);

      break;

    case 6:  // SPEED
      lcd.createChar(1, downArrow);
      lcd.createChar(2, selectArrow);
      lcd.createChar(3, upArrow);
      lcd.createChar(7, rightArrow);
      lcd.createChar(8, leftArrow);
      lcd.clear();
      lcd.setCursor(15, 0);
      lcd.write(3);
      lcd.setCursor(0, 0);
      lcd.write(" Speed:");
      lcd.setCursor(0, 0);
      lcd.write(2);
      lcd.setCursor(7, 0);
      if (speedLevel == 1) {
        lcd.print("Slow");        
      } else if (speedLevel == 2) {
        lcd.print("Fast");
      } else {
        lcd.print("Turbo");
      }
      lcd.setCursor(12, 0);
      lcd.write(8);
      lcd.setCursor(13, 0);
      lcd.write(7);
      lcd.setCursor(0, 1);
      lcd.print(" Walls:");
      if (walls) {
        lcd.print("ON");
      } else {
        lcd.print("OFF");
      }
      lcd.setCursor(15, 1);
      lcd.write(1);

      xValue = analogRead(xPin);

      if (xJoyMoved == false) {
        if (xValue > maxThreshold) {
          if (speedLevel < 3) {
            speedLevel++;
          }
          xJoyMoved = true;
        }
        if (xValue < minThreshold) {
          if (speedLevel > 1) {
            speedLevel--;
          }
          xJoyMoved = true;
        }
      }

      if (xValue < maxThreshold && xValue > minThreshold) {
        xJoyMoved = false;
      }
      delay(10);
      break;

    case 7: // WALLS
      lcd.createChar(1, downArrow);
      lcd.createChar(2, selectArrow);
      lcd.createChar(3, upArrow);
      lcd.createChar(7, rightArrow);
      lcd.createChar(8, leftArrow);
      lcd.clear();
      lcd.setCursor(15, 0);
      lcd.write(3);
      lcd.setCursor(12, 0);
      lcd.write(8);
      lcd.setCursor(13, 0);
      lcd.write(7);
      lcd.setCursor(15, 1);
      lcd.write(1);
      lcd.setCursor(0, 0);
      lcd.write(2);
      lcd.print("Walls:");
      if (walls) {
        lcd.print("ON");
      } else {
        lcd.print("OFF");
      }
      lcd.setCursor(0, 1);
      lcd.print(" RESET HS");
      

      xValue = analogRead(xPin);

      if (xJoyMoved == false) {
        if (xValue > maxThreshold || xValue < minThreshold) {
          walls = !walls;          
          xJoyMoved = true;
        }
      }

      if (xValue < maxThreshold && xValue > minThreshold) {
        xJoyMoved = false;
      }
      break;
    
    case 8: // RESET
      lcd.createChar(1, point);
      lcd.createChar(2, selectArrow);
      lcd.createChar(3, upArrow);
      lcd.createChar(7, rightArrow);
      lcd.createChar(8, leftArrow);
      lcd.clear();
      lcd.setCursor(15, 0);
      lcd.write(3);
      lcd.setCursor(0, 0);
      lcd.write(2);
      lcd.print("RESET HS ");
      lcd.write(1);
      lcd.setCursor(0, 1);
      lcd.print(" Dissapear:");
      if (dissapear) {
        lcd.print("ON");
      } else {
        lcd.print("OFF");
      }

      delay(10);
      break;

    case 9: // DISSAPEAR
      lcd.createChar(1, point);
      lcd.createChar(2, selectArrow);
      lcd.createChar(3, upArrow);
      lcd.createChar(7, rightArrow);
      lcd.createChar(8, leftArrow);
      lcd.clear();
      lcd.setCursor(15, 0);
      lcd.write(3);
      lcd.setCursor(0, 0);
      lcd.print(" RESET HS ");
      lcd.setCursor(0, 1);
      lcd.write(2);
      lcd.print("Dissapear:");
      if (dissapear) {
        lcd.print("ON");
      } else {
        lcd.print("OFF");
      }

      lcd.setCursor(14, 1);
      lcd.write(8);
      lcd.setCursor(15, 1);
      lcd.write(7);

      xValue = analogRead(xPin);

      if (xJoyMoved == false) {
        if (xValue > maxThreshold || xValue < minThreshold) {
          dissapear = !dissapear;          
          xJoyMoved = true;
        }
      }

      if (xValue < maxThreshold && xValue > minThreshold) {
        xJoyMoved = false;
      }
      delay(10);
      break;

    case 10:
      settingsMenuState = 9;
      break;
  }

  delay(20);
}

void about() {

  switch (aboutMenuState) {
    case 0:
      aboutMenuState = 1;
      break;
    case 1:
      lcd.createChar(1, downArrow);
      lcd.clear();
      lcd.print("Name: Snake");
      lcd.setCursor(0, 1);
      lcd.print("Made by Luca");
      lcd.setCursor(15, 1);
      lcd.write(1);
      break;
    case 2:
      lcd.createChar(3, upArrow);
      lcd.clear();
      lcd.print("Made by Luca");
      lcd.setCursor(0, 1);
      lcd.print("GH: Cordus-Luca");
      lcd.setCursor(15, 0);
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
  lcd.print("Joystick control");
  lcd.setCursor(0, 1);
  lcd.print("Eat for points");
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
  if (sound) {
    tone(buzzerPin, toneSound, 30);
  }
}

void updateMatrix() {
  for (int i = 0; i < matrixSize; i++) {
    for (int j = 0; j < matrixSize; j++) {
      lc.setLed(0, i, j, matrix[i][j]);
    }
  }
}

void updatePositions() {
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

float calculateDeltaTime() {
  float currentTime = millis();
  float dt = currentTime - oldTime;
  oldTime = currentTime;
  return dt;
}

void reset() {
  for (int j = 0; j < 8; j++) {
    matr[j] = 0;
  }
}

void Update() {
  reset();

  int newHead[2] = { snake.head[0] + snake.dir[0], snake.head[1] + snake.dir[1] };

  //Handle Borders
  if (walls == false) {
    if (newHead[0] == 8) {  // IF COLOANA == 8, URMATOAREA COLOANA = 0
      newHead[0] = 0;
    } else if (newHead[0] == -1) { // IF COLOANA == -1, URMATOAREA COLOANA = 7
      newHead[0] = 7;
    } else if (newHead[1] == 8) { // IF LINIA == 8, URMATOAREA LINIE = -1
      newHead[1] = 0;
    } else if (newHead[1] == -1) { // IF LINIA == -1, URMATOAREA LINIE = 7
      newHead[1] = 7;
    }
  } else {
    if (newHead[0] == 8 || newHead[0] == -1 || newHead[1] == 8 || newHead[1] == -1) {
    isDead = true;
    }
  }
  
  // CHECK SELF HIT
  for (j = 0; j < snake.len; j++) {
    if (snake.body[j][0] == newHead[0] && snake.body[j][1] == newHead[1]) {
      Serial.println(snake.body[j][0]);
      Serial.println(snake.body[j][1]);
      snake = { { 1, 5 }, { { 0, 5 }, { 1, 5 } }, 2, { 1, 0 } };  //Reinitialize the snake object
      int ok = 0;
      while(ok == 0) {      // APPLE NOT IN SNAKE
        apple.rPos = (int)random(0, 8);
        apple.cPos = (int)random(0, 8);
        for (j = 0; j < snake.len; j++) {
          if(snake.body[j][0] != apple.rPos && snake.body[j][1] != apple.cPos) {
            ok = 1;
          }
        }
      }
      apple = { apple.rPos, apple.cPos };
      isDead = true;
      
      return;
    }
  }
  
  appleReset();  

  // CHECK APPLE EAT
  if (newHead[0] == apple.rPos && newHead[1] == apple.cPos) {
    makeSound(400);
    snake.len = snake.len + 1;
    int ok = 0;
    while(ok == 0) {      // APPLE NOT IN SNAKE
      apple.rPos = (int)random(0, 8);
      apple.cPos = (int)random(0, 8);
      for (j = 0; j < snake.len; j++) {
        if(snake.body[j][0] != apple.rPos && snake.body[j][1] != apple.cPos) {
          ok = 1;
        }
      }
    }
    
    currentScore = currentScore + speedLevel;
    if (walls) {
      currentScore++;
    }

    if (dissapear) {
      currentScore++;
    }

  } else {
    removeFirst();  
  }

  snake.body[snake.len - 1][0] = newHead[0];
  snake.body[snake.len - 1][1] = newHead[1];

  snake.head[0] = newHead[0];
  snake.head[1] = newHead[1];

  // UPDATE THE MATR
  for (j = 0; j < snake.len; j++) {
    matr[snake.body[j][0]] |= (128 >> snake.body[j][1]); // SE SHIFTEAZA COLOANA
  }

  matr[apple.rPos] |=  (128 >> apple.cPos);
}

void render() {

  for (i = 0; i < 8; i++) {
    lc.setRow(0, i, matr[i]);
  }
}

void removeFirst() {
  for (j = 1; j < snake.len; j++) {
    snake.body[j - 1][0] = snake.body[j][0];
    snake.body[j - 1][1] = snake.body[j][1];
  }
}

void resetHighscore() {
  firstPlayer.name[0] = " ";
  firstPlayer.name[1] = " ";
  firstPlayer.name[2] = " ";
  firstPlayer.score = 0;
  secondPlayer.name[0] = " ";
  secondPlayer.name[1] = " ";
  secondPlayer.name[2] = " ";
  secondPlayer.score = 0;
  thirdPlayer.name[0] = " ";
  thirdPlayer.name[1] = " ";
  thirdPlayer.name[2] = " ";
  thirdPlayer.score = 0;
  fourthPlayer.name[0] = " ";
  fourthPlayer.name[1] = " ";
  fourthPlayer.name[2] = " ";
  fourthPlayer.score = 0;
  fifthPlayer.name[0] = " ";
  fifthPlayer.name[1] = " ";
  fifthPlayer.name[2] = " ";
  fifthPlayer.score = 0;  
  storePlayers();
}

void appleReset() { 
  float deltaTime2 = calculateDeltaTime();
  appleTimer += deltaTime2;
  Serial.println(appleTimer);
  if (dissapear) {
    if (appleTimer > appleDelay) {
      appleTimer = 0;
      int ok = 0;
      while(ok == 0) {      // APPLE NOT IN SNAKE
        apple.rPos = (int)random(0, 8);
        apple.cPos = (int)random(0, 8);
        for (j = 0; j < snake.len; j++) {
          if(snake.body[j][0] != apple.rPos && snake.body[j][1] != apple.cPos) {
            ok = 1;
          }
        }
      }
      apple = { apple.rPos, apple.cPos };
      currentScore -= 1;
    }
  }
}