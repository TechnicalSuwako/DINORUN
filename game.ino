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

void setup() {
  pinMode(pinBuzzer, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(pinButton), buttonHandler, RISING);
  lcd.begin(16, 2);
  lcd.createChar(7, dino);
  lcd.createChar(6, rock);
  lcd.createChar(5, coin);
}

char dinoChar = byte(7);
char rockChar = byte(6);
char coinChar = byte(5);

bool jump = false;
bool jumping = false;
bool died = false;
unsigned long jumpTime = 0;
int score = 0;
int dinoY = 1;
const int initRocks[] = { 5, 12, 16, 19, 23, 27, 30, 34, 36, 39, 41, 44, 47, 49, 52 };
const int countRocks = sizeof(initRocks) / sizeof(initRocks[0]);
int rockX[countRocks];
const int initCoins[] = { 8, 17, 25, 29, 32, 40, 43, 48, 55};
const int countCoins = sizeof(initCoins) / sizeof(initCoins[0]);
int coinX[countCoins];

void buttonHandler() {
  if (jumping || died) return;
  play(pinBuzzer, 523, 0, 100);
  switch (state) {
    case Start:
      state = Play;
      break;
    case Play:
      jumpTime = 0;
      jump = true;
      break;
    case Dead:
      state = Start;
      break;
  }
}

int gameSpeed = 150;
unsigned long lastMove = 0;

void titlescreen() {
  char buf1[16], buf2[16];
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("DINO RUN");
  lcd.setCursor(0, 1);
  sprintf(buf2, "%c    %c  %c   %c", dinoChar, rockChar, coinChar, rockChar);
  lcd.print(buf2);
  score = 0;
  jump = false;
  jumping = false;
  jumpTime = 0;
  dinoY = 1;
  lastMove = 0;

  for (int i = 0; i < sizeof(rockX) / sizeof(rockX[0]); ++i) {
    rockX[i] = initRocks[i];
  }

  for (int i = 0; i < sizeof(coinX) / sizeof(coinX[0]); ++i) {
    coinX[i] = initCoins[i];
  }
}

void gameplay() {
  unsigned long now = millis();
  if (now - lastMove >= gameSpeed) {
    lastMove = now;

    for (int i = 0; i < sizeof(rockX) / sizeof(int); ++i) {
      rockX[i]--;
      if (rockX[i] < -1) rockX[i] = 25 + random(15);
    }
  
    for (int i = 0; i < sizeof(coinX) / sizeof(int); ++i) {
      coinX[i]--;
      if (coinX[i] < -1) coinX[i] = 30 + random(20);
    }
  }

  if (jump) {
    jumping = true;
    jumpTime++;
    dinoY = 0;
    if (jumpTime > 1) jump = false;
  } else {
    jumping = false;
    jumpTime = 0;
    dinoY = 1;
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

  for (int i = 0; i < sizeof(rockX) / sizeof(rockX[0]); ++i) {
    int x = rockX[i];
    if (x >= 0 && x < 16) {
      lcd.setCursor(x, 1);
      lcd.print(obstacle);
      if (dinoY == 1 && x == 1) {
        play(pinBuzzer, 698, 0, 1000);
        delay(1000);
        state = Dead;
        died = true;
      }
    }
  }

  for (int i = 0; i < sizeof(coinX) / sizeof(coinX[0]); ++i) {
    int x = coinX[i];
    if (x >= 0 && x < 16 && coinX[i] != 0) {
      lcd.setCursor(x, 1);
      if (dinoY == 1 && x == 1 && score <= 999) score++;
      else lcd.print(collect);
    }
  }
}

void dead() {
  lcd.clear();
  char buf[16];
  lcd.setCursor(3, 0);
  sprintf(buf, "SCORE: %3d", score);
  lcd.print(buf);
  lcd.setCursor(4, 1);
  lcd.print("GAMEOVER");

  if (died) {
    delay(1000);
    died = false;
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
