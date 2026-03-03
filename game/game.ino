#include <rgb_lcd.h>
rgb_lcd lcd;

volatile int curCol, curRow;

const int pinButton = 3;
const int pinBuzzer = 7;

typedef enum {
  Start, Play, Dead
} GameState;

volatile GameState state = Start;

byte dino[8] = {
  0b00100,
  0b01110,
  0b00110,
  0b01100,
  0b01110,
  0b11100,
  0b01100,
  0b10100
};

byte rock[8] = {
  0b00010,
  0b00110,
  0b01110,
  0b00111,
  0b01110,
  0b01110,
  0b11110,
  0b11111
};

byte coin[8] = {
  0b00100,
  0b01110,
  0b01110,
  0b11011,
  0b11011,
  0b01110,
  0b01110,
  0b00100
};

char dinoChar = byte(7);
char rockChar = byte(6);
char coinChar = byte(5);

bool jump = false;
bool jumping = false;
bool died = false;
unsigned long jumpTime = 0;
const unsigned long jumpDuration = 350;

int score = 0;
int dinoY = 1;

const int initRocks[] = { 5, 12, 16, 19, 23, 27, 30, 34, 36, 39, 41, 44, 47, 49, 52 };
const int countRocks = sizeof(initRocks) / sizeof(initRocks[0]);
int rockX[countRocks];
const int initCoins[] = { 8, 17, 25, 29, 32, 40, 43, 48, 55};
const int countCoins = sizeof(initCoins) / sizeof(initCoins[0]);
int coinX[countCoins];

int gameSpeed = 200;
unsigned long lastMove = 0;

unsigned long deathShowStart = 0;
const unsigned long deathScreenMinTime = 2000;

volatile bool btnPressed = false;

void setup() {
  pinMode(pinBuzzer, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(pinButton), buttonHandler, RISING);
  lcd.begin(16, 2);
  lcd.createChar(7, dino);
  lcd.createChar(6, rock);
  lcd.createChar(5, coin);
  resetGame();
}

void buttonHandler() {
  btnPressed = true;
}

void resetGame() {
  score = 0;
  jump = false;
  jumping = false;
  jumpTime = 0;
  dinoY = 1;
  lastMove = millis();
  deathShowStart = 0;
  lcd.setRGB(0, 0, 255);

  for (int i = 0; i < countRocks; ++i) {
    rockX[i] = initRocks[i];
  }

  for (int i = 0; i < countCoins; ++i) {
    coinX[i] = initCoins[i];
  }
}

void titlescreen() {
  char buf[16];
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("DINO RUN");
  lcd.setCursor(0, 1);
  sprintf(buf, "%c    %c  %c   %c", dinoChar, rockChar, coinChar, rockChar);
  lcd.print(buf);

  if (btnPressed) {
    btnPressed = false;
    state = Play;
    resetGame();
  }
}

void gameplay() {
  unsigned long now = millis();

  if (btnPressed) {
    btnPressed = false;
    if (!jumping) {
      tone(pinBuzzer, 523, 100);
      jump = true;
      jumpTime = now;
    }
  }

  if (jump) {
    jumping = true;
    dinoY = 0;
    if (now - jumpTime >= jumpDuration) {
      jump = false;
      jumping = false;
      dinoY = 1;
    }
  }

  if (now - lastMove >= (unsigned long)gameSpeed) {
    lastMove = now;

    for (int i = 0; i < countRocks; ++i) {
      rockX[i]--;
      if (rockX[i] < -1) rockX[i] = 25 + random(12, 22);
    }
  
    for (int i = 0; i < countCoins; ++i) {
      coinX[i]--;
      if (coinX[i] < -1) coinX[i] = 32 + random(15, 30);
    }
  }

  lcd.clear();

  lcd.setCursor(3, 0);
  char scoreBuf[16];
  sprintf(scoreBuf, "SCORE: %3d", score);
  lcd.print(scoreBuf);

  char player[2];
  char obstacle[2];
  char collect[2];
  
  sprintf(player, "%c", dinoChar);
  sprintf(obstacle, "%c", rockChar);
  sprintf(collect, "%c", coinChar);

  lcd.setCursor(1, dinoY);
  lcd.print(player);

  bool hit = false;
  for (int i = 0; i < countRocks; ++i) {
    int x = rockX[i];
    if (x >= 0 && x < 16) {
      lcd.setCursor(x, 1);
      lcd.print(obstacle);
      if (dinoY == 1 && x == 1) {
        hit = true;
      }
    }
  }

  for (int i = 0; i < countCoins; ++i) {
    int x = coinX[i];
    if (x >= 0 && x < 16) {
      lcd.setCursor(x, 1);
      if (dinoY == 1 && x == 1) {
        tone(pinBuzzer, 880, 80);
        score++;
      }
      else lcd.print(collect);
    }
  }

  if (hit) {
    unsigned long n = millis();
    lcd.setRGB(255, 0, 0);
    tone(pinBuzzer, 698, 1000);

    if (n >= 2000) {
      state = Dead;
      deathShowStart = now;
    }
  }
}

void dead() {
  unsigned long now = millis();
  lcd.setRGB(0, 0, 255);

  lcd.clear();
  char buf[16];
  lcd.setCursor(3, 0);
  sprintf(buf, "SCORE: %3d", score);
  lcd.print(buf);
  lcd.setCursor(4, 1);
  lcd.print("GAMEOVER");
  if (now - deathShowStart < deathScreenMinTime) return;

  if (btnPressed) {
    btnPressed = false;
    state = Start;
    resetGame();
  }
}

void loop() {
  delay(200);
  switch (state) {
    case Start:
      titlescreen();
      break;
    case Play:
      gameplay();
      break;
    case Dead:
      dead();
      break;
  }
  delay(200);
}
