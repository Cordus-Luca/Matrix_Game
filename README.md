# Matrix Game

# Backstory

The main reason I chose Snake as my game was because I wanted to see if I could improve a classic game everyone knows by adding my own spin on it with multiple speeds and modifiers. in addition it is a very good candidate for an 8x8 matrix display game, as it does not need many pixels to work well.

# Game description

The player controls a long, thin creature, resembling a snake, which roams around on a bordered plane, picking up apples, trying to avoid hitting its own tail or the edges of the playing area. Each time the snake eats an apple, its tail grows longer, making the game increasingly difficult. The user controls the direction of the snake's head (up, down, left, or right), and the snake's body follows.

# Game/menu Demo
Link video: https://www.youtube.com/watch?v=6_MLjZ3GQEg

# Used Components

Used components: 16 X 2 LCD, 8 X 8 Led matrix display, Shift register, Potentiometer, Joystick, Buzzer

# Navigation

Main menu: 
  - To switch from main menu and sub-menu press the button (joystick).

Navigation: 
  - Made with the up and down directions on the joystick. To see what navigation options u have look to the right of the screen at the arrows.

Start game:
  - Shows points, speed, wall collision, apple dissapearing and name of the player.
  - If button is pressed game is paused and you can change settings mid-game.

Highscore: 
  - List of top 5 players with name and scores.

Settings: 
  - Can change name, LCD brightness, matrix brightness (whole matrix lights up while on this option), sound (on/off), the speed of the snake, wall collision(ON/OFF), reset highscore and if the apple will dissapear after a set time(ON/OFF).
  - Default settings are: Name = TST, Speed = Slow, Wall collision = OFF, Apple dissapearing = OFF.
  - To change name, while on said option input right or left direction to select a letter (selected letter is underlined) and up or down to change it (A-Z). To be able to move again in the settings menu from the name selection leave the letter selection by going left until underline dissapears.
  - To change other values simply imput left or right while the selection arrow is pointing to the option.
  - To reset highscore click on the button (will not change menu only in this state.
  - All data except speed, wall collision and apple dissapearing is stored into EEPROM. 

About: 
  - Lists game name
  - Creator name 
  - Creator github username.

How to play: 
  - Brief instructions on how to play.

# How to play

Use the joystick to change the direction of the snake(UP, DOWN, LEFT, RIGHT) and eat apples to gain points.
If the snake eats itself or hits a wall you lose.
Modulators:
  - Speed changes the speed of the snake. If not changed manually, it will gradually change as you progress further in the game.
  - If wall collision is turned ON, the snake will on longer be able to teleport from one side  of the matrix to the other.
  - If apple dissapearing is turned ON, the apple will dissapear after a set time. If the player doesn't manage to eat the apple befure it dissapears to another location, the score goes down by 1 point.
  - The score goes up faster if modulators are turne ON: 
    - Speed: Slow(1), Fast(2), Turbo(3);
    - Wall collision: OFF(0), ON(1);
    - Apple dissapearing: OFF(0), ON(1 if succesful, -1 if not);
