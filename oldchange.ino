#include <Servo.h>

#define GEARCHANGE A4
#define CLUTCH A5
#define GEARPOS A0
#define CLUTCHPOS A1
#define CLUTCHPOT A2
#define CLUTCHBUT 4
#define NEUTBUT 8
#define NEUTRAL 5

#define POT_MAX 800
#define POT_MIN 910

#define UPCHANGE 3
#define DOWNCHANGE 2

#define DELAY 500

#define midpoint 90
#define midpointms 1450
#define halfup 65
#define downchange 180
#define upchange 0
#define movetime 500

#define clutched 0
#define declutched 150

Servo gearchg;
Servo clutch;

unsigned long lastmove = 0;
int currmove = 0;

int inneutral = 0;

int change = 0;
int changeinprogress = 0;
unsigned long lastshow = 0;
int nowclutched = 0;
unsigned long lastclutched = 0;

unsigned long laststart = 0;
int started = 0;

int RELEASE = 0;

void setup()
{
  pinMode(GEARCHANGE, OUTPUT);
  pinMode(CLUTCH, OUTPUT);
  pinMode(UPCHANGE, INPUT_PULLUP);
  pinMode(DOWNCHANGE, INPUT_PULLUP);
  pinMode(CLUTCHBUT, INPUT_PULLUP);
  pinMode(NEUTBUT, INPUT_PULLUP);
  pinMode(NEUTRAL, INPUT_PULLUP);

  gearchg.attach(GEARCHANGE);
  clutch.attach(CLUTCH);
  gearchg.writeMicroseconds(midpointms);
  clutch.write(clutched);

  Serial.begin(9600);
}

void loop()
{

  if (digitalRead(NEUTRAL) == 0)
  {
    inneutral = 1;
  }
  else
  {
    inneutral = 0;
  }

  int clutchpot = analogRead(CLUTCHPOT);
  int clutchpos = clutched;
  if (digitalRead(CLUTCHBUT) == 0)
  {

    clutchpos = declutched;
  }
  else if (clutchpot > POT_MIN - 5)
  {

    clutchpos = clutched;
  }
  else if (clutchpot < POT_MAX && clutchpot > 50)
  {

    clutchpos = declutched;
  }
  else if (clutchpot > 50)
  {

    clutchpos = (int) ((float) clutched + (((float) declutched - (float)  clutched) * (( (float) clutchpot - (float) POT_MIN) / ((float) POT_MAX - (float) POT_MIN))));
  }
  else
  {

    clutchpos = clutched;
  }
  clutch.write(clutchpos);

  if ((digitalRead(UPCHANGE) == 0) && (digitalRead(DOWNCHANGE) == 0))
  {
    RELEASE = 1;
  }
  else if ((digitalRead(UPCHANGE) == 0) && (RELEASE == 0))
  {

    if (started == 0)
    {
      laststart = millis();
      started = 1;
    }
    else if (millis() - laststart > 25)
    {
      clutch.write(declutched);
      delay(100);
      //gearchg.attach(GEARCHANGE);
      if (!inneutral)
      {
        gearchg.write(upchange);
      }
      else
      {
        gearchg.write(downchange);
      }
      delay(DELAY);
      //gearchg.write(midpoint);
      gearchg.writeMicroseconds(midpointms);
      delay(150);
      clutch.write(clutchpos);
      delay(100);
      started = 0;
      RELEASE = 1;
    }
  }
  else if ((digitalRead(DOWNCHANGE) == 0) && (RELEASE == 0))
  {
    if (started == 0)
    {
      laststart = millis();
      started = 1;
    }
    else if (millis() - laststart > 25)
    {

      clutch.write(declutched);
      delay(100);
      //gearchg.attach(GEARCHANGE);
      gearchg.write(downchange);
      delay(DELAY);
      //gearchg.write(midpoint);
      gearchg.writeMicroseconds(midpointms);
      delay(150);
      //clutch.write(clutched);
      for (int i = declutched; i > clutchpos; i--)
      {
        clutch.write(i);
        delay(5);
      }
      clutch.write(clutchpos);
      delay(100);
      started = 0;
      RELEASE = 1;
    }
  }
  else if ((digitalRead(NEUTBUT) == 0) && (RELEASE == 0))
  {
    if (started == 0)
    {
      laststart = millis();
      started = 1;
    }
    else if (millis() - laststart > 50)
    {
      clutch.write(declutched);
      delay(100);
      //gearchg.attach(GEARCHANGE);
      gearchg.write(halfup);
      delay(DELAY);
      //gearchg.write(midpoint);
      gearchg.writeMicroseconds(midpointms);
      delay(150);
      clutch.write(clutchpos);
      delay(100);
      started = 0;
      RELEASE = 1;
    }
  }
  else if ((digitalRead(DOWNCHANGE)) == 1 && (digitalRead(UPCHANGE) == 1) && (digitalRead(NEUTBUT) == 1))
  {
    RELEASE = 0;
    started = 0;
  }
  else
  {
    //gearchg.write(midpoint);
    gearchg.writeMicroseconds(midpointms);
    delay(10);
  }
}
