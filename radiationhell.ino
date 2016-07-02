
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>


// For the breakout, you can use any 2 or 3 pins
// These pins will also work for the 1.8" TFT shield
#define TFT_CS     11
#define TFT_RST    10  // you can also connect this to the Arduino reset
// in which case, set this #define pin to 0!
#define TFT_DC     9

// Option 1 (recommended): must use the hardware SPI pins
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

// Option 2: use any pins but a little slower!
#define TFT_SCLK 13   // set these to be whatever pins you like!
#define TFT_MOSI 11   // set these to be whatever pins you like!

#define JOYSTICK_UP A1
#define JOYSTICK_DOWN A2
#define JOYSTICK_LEFT A0
#define JOYSTICK_RIGHT A3
#define BUTTON_A A4
#define BUTTON_B A5

bool soundon = true;
int x = 100;
int y = 100;


int colors[] = {ST7735_RED, ST7735_GREEN, ST7735_BLUE, ST7735_MAGENTA, ST7735_YELLOW};
int allcolors = 5;
int score = 0;

struct bombs
{
  int x;
  int y;
  int radius;
  int life;
  int power;
};

#define allbombs 100
int bi = 0;
struct bombs b[allbombs];
int bomb_delay = 0;
int jumpvelocity = 0;
#define jumppower 20
int jumpheight = 0;
int jumpdelay = 0;
int formerheight = 0;
int formerx = 0;
int formery = 0;


void setup(void) {

  Serial.begin(9600);
  Serial.print("Hello! ST7735 TFT Test");


  pinMode(JOYSTICK_UP, INPUT);
  pinMode(JOYSTICK_DOWN, INPUT);
  pinMode(JOYSTICK_RIGHT, INPUT);
  pinMode(JOYSTICK_LEFT, INPUT);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);



  // Use this initializer if you're using a 1.8" TFT
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab

  tft.setRotation(2);

  Serial.println("Initialized");

  setup2();
  return;

}

void setup2()
{
  tft.fillScreen(ST7735_BLACK);
  memset(&b, 0, sizeof(b));
  bi = 0;
  bomb_delay = 0;
  jumpvelocity = 0;
  jumpheight = 0;
  jumpdelay = 0;
  formerheight = 0;
  formerx = 0;
  formery = 0;
}

bool isup()
{
  if(!analogRead(JOYSTICK_UP))
    return true;
  else
    return false;
}
bool isdown()
{
  if(!analogRead(JOYSTICK_DOWN))
    return true;
  else
    return false;
}
bool isleft()
{
  if(!analogRead(JOYSTICK_LEFT))
    return true;
  else
    return false;
}
bool isright()
{
  if(!analogRead(JOYSTICK_RIGHT))
    return true;
  else
    return false;
}

bool isbuttonb()
{
  if(!digitalRead(BUTTON_B))
    return true;
  else
    return false;
}

bool isbuttona()
{
  if(!digitalRead(BUTTON_A))
    return true;
  else
    return false;
}


#define NOTE_G3  392
#define NOTE_F3  349
#define NOTE_D3  294
#define NOTE_C3  262
int notes[] = {NOTE_G3, 250, NOTE_F3, 250, NOTE_G3, 500, NOTE_F3, 1000, NOTE_D3, 250, NOTE_C3, 250, NOTE_D3, 500, NOTE_C3, 1000, 0};
void play_mock()
{
  if(!soundon) return;
  for(int i = 0; i < 1000; i += 2)
  {
    if(notes[i] == 0) break;
    tone(6, notes[i]);
    delay(notes[i+1]);
  }
  noTone(6);
}

bool dead()
{
  play_mock();
  while(1)
  {
    char buf[200];
    tft.fillScreen(ST7735_RED);
    tft.setCursor(0, tft.height()/2-30);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(1);
    tft.setTextWrap(true);
    tft.print("Game Over!\n");
    sprintf(buf, "score: %d", score);
    tft.print(buf);

    delay(1000);
    tft.fillScreen(ST7735_BLACK);
    tft.setTextColor(ST7735_RED);
    tft.setCursor(0, tft.height()/2-30);
    tft.print("Game Over!\n");
    tft.print(buf);
    delay(1000);

    if(isbuttona())
    {
      setup2();
      return true;
    }
  }
  return true;
}

void processBombs()
{
  for(int i = 0; i < allbombs; i ++)
  {
    if(b[i].life != 0)
    {
      b[i].life --;
      if(b[i].life != 0)
      {
        b[i].radius += b[i].power;
        if(soundon) 
          tone(6, random(200, 230));
      }
      else
        noTone(6);
    }
  }
}

void drawBombs()
{
  //create new bomb
  if(bomb_delay == 0)
  {
    Serial.println("making new bomb!");
    int i;
    if(bi < allbombs)
    {
      // bomb available
      b[bi].radius = 1;
      b[bi].x = random(tft.width());
      b[bi].y = random(tft.height());
      b[bi].power = random(1,2);
      b[bi].life = random(10, 40);
      char buf[100];
      sprintf(buf, "new bomb %d at %d/%d, power %d life %d", bi, b[bi].x, b[bi].y, b[bi].power, b[bi].life);
      Serial.println(buf);
      bi ++;
    }
    bomb_delay = 50;
  }
  else
  {
    bomb_delay --;
  }

  processBombs();

  //now draw
  for(int i = 0; i < allbombs; i ++)
  {
    if(b[i].life != 0)
    {
      // draw bombs
      tft.drawCircle(b[i].x, b[i].y, b[i].radius, colors[random(allcolors)]);
    }
  }
}

bool check_life()
{
  if(y < 0) return dead();
  if(y > tft.height()) return dead();
  if(x > tft.width()) return dead();
  if(x < 0) return dead();
  for(int i = 0; i < allbombs; i ++)
  {
    int distx, disty, dist;
    distx = abs(b[i].x - x);
    disty = abs(b[i].y - y);
    dist = sqrt(distx*distx + disty*disty);
    if(dist < b[i].radius)
    {
      // dead
      char buf[200];
      sprintf(buf, "dead:\ndistx = %d\ndisty = %d\ndist = %d", distx, disty, dist);
      Serial.println(buf);
      return dead();
    }
  }
  return false;
}

void loop() {
  if(jumpheight > 0)
  {
    tft.fillRect(x-jumpheight/2, y-jumpheight/2, jumpheight, jumpheight, ST7735_GREEN);
    // delete former height which is lame
    if(formerheight > jumpheight)
      tft.drawRect(formerx-formerheight/2, formery-formerheight/2, formerheight, formerheight, ST7735_BLACK);
  }
  else
  {
    tft.fillRect(x-1, y-1, 1, 1, ST7735_GREEN);
  }
  
  // process jumping velocity/height
  if(jumpvelocity != 0)
  {
    if(jumpvelocity > (jumppower / 2))
    {
      formerheight = jumpheight;
      jumpheight ++;
    }
    else
    {
      formerheight = jumpheight;
      jumpheight --;
    }
    if(jumpvelocity) jumpvelocity --;
    if(jumpheight == 0) formerheight = 0;
  }
  formerx = x;
  formery = y;

  if(isup())
    y --;
  if(isdown())
    y ++;
  if(isright())
    x ++;
  if(isleft())
    x --;
  if(isbuttonb())
    soundon = !soundon;
  if((jumpheight == 0) && (jumpdelay == 0) && isbuttona())
  {
    Serial.println("start jumping");
    //start jump
    jumpvelocity = jumppower;
  }
  // if jumping, don't die
  if(jumpheight == 0)
    if(check_life())
      return;
  drawBombs();
  delay(20);
  score ++;
}


