#include <Arduino.h>
#include <avr/eeprom.h>
#include "GyverTimer.h"
#include "MaxButton.h"
#include <Sensors.h>

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans9pt7b.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

struct Game {
    bool begin = false;
    byte coins = 0;
    int points = 0;
} game;
struct Dispensor {
  float ticketRatio = eeprom_read_float(0);
  byte ticketMax = eeprom_read_byte((uint8_t*)4);
  byte ticketMin = eeprom_read_byte((uint8_t*)5);
  int tickets = 0;
} dispensor;
bool menuON = true;

GTimer gameTimer(MS);

// ----------- Coin --------------
GButton coinBtn(4);

void coinSetup() {
  coinBtn.setDebounce(10);
}
void coinFn() {
  if (coinBtn.isClick()) {
    game.coins++;
    menuON = true;
  }
}
// ------------------------------------


// ----------- Start Button ------------------
GButton startBtn(5);

void startBtnSetup() {
  startBtn.setDebounce(10);
}

void startBtnFn() {
  if(startBtn.isPress()) {
    if(game.begin || game.coins < 1) return;
    game.coins--;
    game.begin = true;
    game.points = 0;
    gameTimer.setTimeout(60000);
    menuON = true;
  }
}
// ------------------------------------


// ----------- Dispensor --------------
GButton dspOut(3);

void dispensorSetup() {
  dspOut.setDebounce(0);
  pinMode(2, OUTPUT); //dspIN
}

void ticketDec() {
  if(dispensor.tickets > 0) dispensor.tickets--;
}

void dispensorFn() {
  if(!dispensor.tickets) return;
  digitalWrite(2, HIGH);
  if(dspOut.isClick()) {
    dispensor.tickets--;
    menuON = true;
    if(!dispensor.tickets) digitalWrite(2, LOW);
  }
}
// ------------------------------------


// ----------- Sensors --------------
Sensors s1(7, HIGH_PULL, NORM_CLOSE);

void sensorsFn() {
  if(s1.isPress()) {
    game.points++;
    menuON = true;
  }
}
// ------------------------------------


//+ ----------- Game ------------------
void gameSetup() {
  if(dispensor.ticketRatio > 100) dispensor.ticketRatio = 100;
  if(dispensor.ticketRatio < 0) dispensor.ticketRatio = 0;
}

void gameFn() {
  if(!game.begin) return;
  if(gameTimer.isReady()) {
    game.begin = false;
    dispensor.tickets += dispensor.ticketMin + (int)(game.points * dispensor.ticketRatio);
    if (dispensor.tickets > dispensor.ticketMax) dispensor.tickets = dispensor.ticketMax;
  }
  menuON = true;
}
// ------------------------------------


// -------------- Display ----------------------------
bool menuSetup = false;
byte menuList = 1;

void displaySetup() {
  if (menuList == 1) display.print("[ ");
  display.print("Cof - "); display.println(dispensor.ticketRatio);
  if (menuList == 2) display.print("[ ");
  display.print("max - "); display.println(dispensor.ticketMax);
  if (menuList == 3) display.print("[ ");
  display.print("min - "); display.println(dispensor.ticketMin);
}

void displayState() {
  display.print("Coins: "); display.println(game.coins);
  display.print("Points: "); display.println(game.points);
  display.print("Tickets: "); display.println(dispensor.tickets);
}

void displayRander() {
  if(!menuON) return;

  display.clearDisplay();
  display.setFont(&FreeSans9pt7b);
  //display.setTextSize(1);
  display.setTextColor(WHITE);        
  display.setCursor(0,15);

  if (menuSetup) {
    displaySetup();
  } else {
    displayState();
  }

  display.display();
  menuON = false;
}

GButton btnLeft(15);
GButton btnOk(16);
GButton btnRight(17);

void keyboardSetup() {

  btnLeft.setDebounce(30);
  btnOk.setDebounce(30);
  btnRight.setDebounce(30);

  btnOk.setTimeout(500);
  btnLeft.setTimeout(500);
  btnLeft.setStepTimeout(100);
  btnRight.setTimeout(500);
  btnRight.setStepTimeout(100);
}
// ---------------- КНОПКИ МЕНЮ ----------------------
void buttonMenu() { 
  if (btnOk.isHolded()) {
    menuON = true;
    if (menuSetup) {
      if (dispensor.ticketMax < dispensor.ticketMin) dispensor.ticketMax = dispensor.ticketMin;
      eeprom_update_float(0, dispensor.ticketRatio);
      eeprom_update_byte((uint8_t*)4, dispensor.ticketMax);
      eeprom_update_byte((uint8_t*)5, dispensor.ticketMin);
      menuON = true;
    }
    menuSetup = !menuSetup;
  }
  
  // меню настроек 
  if(!menuSetup) return;

  if (btnOk.isClick()) {
    menuON = true;
    if (!menuSetup) return;
    menuList++;
    if (menuList > 3) menuList = 1;
  }

  if(btnLeft.isClick() or btnLeft.isStep()) {
    menuON = true;
    if (!menuSetup) return;
    if(menuList == 1)
    dispensor.ticketRatio -= 0.25;
    if(dispensor.ticketRatio < 0) dispensor.ticketRatio = 100;
    if(menuList == 2) dispensor.ticketMax--;
    if(dispensor.ticketMax < dispensor.ticketMin) dispensor.ticketMax = dispensor.ticketMin;
    if(menuList == 3) dispensor.ticketMin--;
  }

  if(btnRight.isClick() or btnRight.isStep()) {
    menuON = true;
    if (!menuSetup) return;
    if(menuList == 1) dispensor.ticketRatio += 0.25;
    if(dispensor.ticketRatio > 100) dispensor.ticketRatio = 0;
    if(menuList == 2) dispensor.ticketMax++;
    if(menuList == 3) dispensor.ticketMin++;
  }
}
// ---------------------------------------------------


void tick() {
  coinBtn.tick();
  if(dispensor.tickets) dspOut.tick();
  if(game.begin){
    s1.tick();
  } else {
    btnOk.tick();
    if(menuSetup){
      btnLeft.tick();
      btnRight.tick();
    }
    if(game.coins)startBtn.tick();
  }
}

void sensorTick() {
  if(game.begin) s1.tick();
}

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(1000);
  display.clearDisplay();
  display.display();
  keyboardSetup();
  gameSetup();
  coinSetup();
  startBtnSetup();
  dispensorSetup();
}

void loop() {

  tick();
  coinFn();
  startBtnFn();
  sensorsFn();
  dispensorFn();
  gameFn();
  buttonMenu();
  //sensorTick();
  displayRander();
}
