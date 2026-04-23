#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== PINS =====
#define IN1 19
#define IN2 18
#define PWM 23

#define IR_PIN 33
#define TRIG 32
#define ECHO 25

#define LED1 13
#define LED2 14
#define BUZZER 4

#define BTN_START 26
#define BTN_MODE 27

// ===== STATES =====
bool running = false;
bool goingUp = true;
bool lastStart = HIGH;
bool lastMode = HIGH;

int mode = 0;

bool roofDetected = false;
bool pauseActive = false;

unsigned long stopTime = 0;

bool midHoldDone = false;
bool midHoldActive = false;
unsigned long midHoldTime = 0;

float maxHeight = 0;
float halfHeight = 0;

int threshold = 500;
int speedUp = 100;
int speedDown = 50;

// ===== YOUR EXACT WELCOME (UNCHANGED) =====
int cx = 64;
int cy = 26;
int orbR = 14;
int botOffset = 5;

void drawOrb() {
  display.drawCircle(cx, cy, orbR, WHITE);
  display.drawCircle(cx, cy, orbR + 1, WHITE);
}

void drawBot(float angle) {

  float r = orbR + botOffset;

  int cx_bot = (int)(cx + cos(angle) * r);
  int cy_bot = (int)(cy + sin(angle) * r);

  float dx = cos(angle);
  float dy = sin(angle);

  float nx = -sin(angle);
  float ny = cos(angle);

  int bx = (int)(cx_bot + dx * 2);
  int by = (int)(cy_bot + dy * 2);

  int topX = (int)(bx + dx * 8);
  int topY = (int)(by + dy * 8);

  display.drawLine(bx, by, topX, topY, WHITE);
  display.drawLine(bx + nx*2, by + ny*2, topX + nx*2, topY + ny*2, WHITE);
  display.drawLine(bx - nx*2, by - ny*2, topX - nx*2, topY - ny*2, WHITE);

  display.drawLine(topX + nx*2, topY + ny*2, topX - nx*2, topY - ny*2, WHITE);

  int midX = (bx + topX) / 2;
  int midY = (by + topY) / 2;

  int armLen = 5;

  int arm1x = (int)(midX + nx * armLen + dx);
  int arm1y = (int)(midY + ny * armLen + dy);

  int arm2x = (int)(midX - nx * armLen + dx);
  int arm2y = (int)(midY - ny * armLen + dy);

  display.drawLine(midX, midY, arm1x, arm1y, WHITE);
  display.drawLine(midX, midY, arm2x, arm2y, WHITE);

  int wx1 = (int)(cx_bot + nx * 4);
  int wy1 = (int)(cy_bot + ny * 4);

  int wx2 = (int)(cx_bot - nx * 4);
  int wy2 = (int)(cy_bot - ny * 4);

  display.drawCircle(wx1, wy1, 2, WHITE);
  display.drawCircle(wx2, wy2, 2, WHITE);
}

void drawDots(float offset) {
  for (int i = 0; i < 360; i += 20) {
    float a = radians(i) + offset;
    int x = (int)(cx + cos(a) * (orbR + 4));
    int y = (int)(cy + sin(a) * (orbR + 4));
    display.drawPixel(x, y, WHITE);
  }
}

void drawScan(float angle) {
  int x = (int)(cx + cos(angle) * orbR);
  int y = (int)(cy + sin(angle) * orbR);
  display.fillCircle(x, y, 2, WHITE);
}

void centerText(const char* txt, int y, int size = 1) {
  display.setTextSize(size);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(txt, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, y);
  display.print(txt);
}

void bottomText(const char* txt) {
  display.fillRect(0, 52, 128, 12, BLACK);
  display.setTextSize(1);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(txt, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, 54);
  display.print(txt);
}

void bootAnim() {

  float angle = -PI/2;

  for (int i = 0; i < 50; i++) {
    display.clearDisplay();
    drawOrb();
    angle += 0.04;
    drawBot(angle);
    bottomText("BOOTING");
    display.display();
    delay(20);
  }

  for (int i = 0; i < 50; i++) {
    display.clearDisplay();
    drawOrb();
    angle += 0.04;
    drawBot(angle);
    bottomText("INITIALIZING");
    display.display();
    delay(20);
  }

  for (int i = 0; i < 70; i++) {
    display.clearDisplay();
    drawOrb();
    angle += 0.05;
    drawBot(angle);
    drawDots(angle);
    bottomText("SCANNING");
    display.display();
    delay(20);
  }

  for (int i = 0; i < 70; i++) {
    display.clearDisplay();
    drawOrb();
    angle += 0.06;
    drawBot(angle);
    drawScan(angle);
    bottomText("LOADING DRIVERS");
    display.display();
    delay(20);
  }

  for (int i = 0; i < 60; i++) {
    display.clearDisplay();
    drawOrb();
    angle += 0.03 + 0.02 * (1 - cos(angle));
    drawBot(angle);
    drawDots(angle);
    bottomText("ALMOST THERE");
    display.display();
    delay(20);
  }

  for (int i = 0; i < 40; i++) {
    display.clearDisplay();
    centerText("MR SYSTEMS", 26, 2);
    display.display();
    delay(20);
  }

  display.clearDisplay();

  display.drawCircle(cx, cy, orbR, WHITE);
  display.drawLine(cx - 5, cy, cx, cy + 5, WHITE);
  display.drawLine(cx, cy + 5, cx + 6, cy - 4, WHITE);

  centerText("READY", 44, 2);

  display.display();
  delay(1000);

  display.clearDisplay();
  display.display();
}

// ===== MOTOR + LOGIC (UNCHANGED) =====
void setSpeed(int s){ ledcWrite(PWM,s); }
void forward(){ digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW); }
void backward(){ digitalWrite(IN1,LOW); digitalWrite(IN2,HIGH); }
void stopMotors(){ digitalWrite(IN1,LOW); digitalWrite(IN2,LOW); }

float getDistance(){
  digitalWrite(TRIG,LOW); delayMicroseconds(2);
  digitalWrite(TRIG,HIGH); delayMicroseconds(10);
  digitalWrite(TRIG,LOW);
  long d=pulseIn(ECHO,HIGH,30000);
  if(d==0)return -1;
  return d*0.034/2;
}

void beep(int t=100){
  digitalWrite(BUZZER,HIGH);
  delay(t);
  digitalWrite(BUZZER,LOW);
}

// ===== SETUP =====
void setup(){

  pinMode(IN1,OUTPUT); pinMode(IN2,OUTPUT);
  pinMode(IR_PIN,INPUT);
  pinMode(TRIG,OUTPUT); pinMode(ECHO,INPUT);

  pinMode(LED1,OUTPUT); pinMode(LED2,OUTPUT);
  pinMode(BUZZER,OUTPUT);

  pinMode(BTN_START,INPUT_PULLUP);
  pinMode(BTN_MODE,INPUT_PULLUP);

  ledcAttach(PWM,1000,8);

  Wire.begin(21,22);
  display.begin(SSD1306_SWITCHCAPVCC,0x3C);
  display.setTextColor(WHITE);

  bootAnim(); // EXACT WELCOME
}

// ===== LOOP (UNCHANGED LOGIC) =====
void loop(){

  bool s=digitalRead(BTN_START);
  bool m=digitalRead(BTN_MODE);

  if(lastMode==HIGH && m==LOW) mode=!mode;
  lastMode=m;

  if(lastStart==HIGH && s==LOW){
    running=!running;
    if(running){
      goingUp=true;
      roofDetected=false;
      pauseActive=false;
      midHoldDone=false;
      midHoldActive=false;
      maxHeight=0;
    }
    beep(120);
  }
  lastStart=s;

  if(!running){
    stopMotors();
    digitalWrite(LED2,LOW);
    digitalWrite(LED1,HIGH);
    display.clearDisplay();
    centerText(mode==0?"ROUND 1":"ROUND 2",25,2);
    display.display();
    return;
  }

  float dist=getDistance();
  int ir=analogRead(IR_PIN);

  if(goingUp && dist>0 && dist>maxHeight) maxHeight=dist;

  if(goingUp){

    digitalWrite(LED2,HIGH);
    digitalWrite(LED1,LOW);

    if(pauseActive){
      stopMotors();
      display.clearDisplay();
      centerText("CEILING",25,2);
      display.display();

      if(millis()-stopTime>=400){
        goingUp=false;
        pauseActive=false;
        halfHeight=maxHeight/2;
      }
      return;
    }

    setSpeed(speedUp);
    forward();

    display.clearDisplay();
    centerText("UP",25,2);
    display.display();

    if(ir<threshold && !roofDetected){
      roofDetected=true;
      stopMotors();
      beep(200);
      pauseActive=true;
      stopTime=millis();
    }
  }

  else{

    digitalWrite(LED2,HIGH);
    digitalWrite(LED1,LOW);

    if(mode==1 && !midHoldDone && dist>0 && abs(dist-halfHeight)<1.5){
      stopMotors();
      midHoldActive=true;
      midHoldTime=millis();
      midHoldDone=true;
      beep(300);
    }

    if(midHoldActive){
  digitalWrite(LED2,LOW);
  digitalWrite(LED1,HIGH);

  digitalWrite(BUZZER, HIGH); // 🔥 CONTINUOUS BUZZ

  display.clearDisplay();
  centerText("HOLD",25,2);
  display.display();

  if(millis()-midHoldTime>=10000){
    midHoldActive=false;
    digitalWrite(BUZZER, LOW); // 🔇 STOP BUZZ AFTER HOLD
  }
  return;
}

    setSpeed(speedDown);
    backward();

    display.clearDisplay();
    centerText("DOWN",25,2);
    display.display();

    static int confirm=0;
    if(dist>0 && dist<15) confirm++;
    else if(dist>25) confirm=0;

    if(confirm>=1){
      stopMotors();
      digitalWrite(LED2,LOW);
      digitalWrite(LED1,HIGH);
      beep(500);
      running=false;
    }
  }
}