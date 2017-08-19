#include <Servo.h>
#include <SPI.h>

#define GEARCHANGE A4
#define CLUTCH A5
#define GEARPOS A0
#define CLUTCHPOS A1
#define CLUTCHPOT A2
#define CLUTCHBUT 4
#define NEUTBUT 8
#define NEUTRAL 5
#define ECUCUT A6 //I guess?

#define MOSI 11
#define MISO 12
#define SCK 13
#define SS 10

#define CUTUP 0 //Disabled until green light

#define POT_MAX 800
#define POT_MIN 910

#define SENDDELAY 200 //every 200ms, send gear

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

#define PRECUTDELAY 20 //20ms before
#define CUTDELAY 200 //200ms during
#define POSTCUTDELAY 150 //150ms after
#define CUTFULL 1 //Cut all servo time

Servo gearchg;
Servo clutch;

#define ECUDELAY 1 //we do like a pulse of 20ms or whatever and the ecu handles the time
#define ECUDURING 2  //we handle the time
int CUTTYPE = ECUDURING;

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
unsigned long lastsent = 0;

char gears[7] = {'N', '1', '2', '3', '4', '5', '6'};
int gear = 0;

void setup()
{
  pinMode(GEARCHANGE, OUTPUT);
  pinMode(CLUTCH, OUTPUT);
  pinMode(ECUCUT, OUTPUT);
  pinMode(UPCHANGE, INPUT_PULLUP);
  pinMode(DOWNCHANGE, INPUT_PULLUP); //these are pull up because *shrug*
  pinMode(CLUTCHBUT, INPUT_PULLUP);
  pinMode(NEUTBUT, INPUT_PULLUP);
  pinMode(NEUTRAL, INPUT_PULLUP);

  pinMode(SS, OUTPUT);
  digitalWrite(SS, 1); //Now I am the master

  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2);

  gearchg.attach(GEARCHANGE);
  clutch.attach(CLUTCH);
  gearchg.writeMicroseconds(midpointms);
  clutch.write(clutched);
  digitalWrite(ECUCUT, 0);

  Serial.begin(9600);
}

void loop()
{

  if (millis() - lastsent > SENDDELAY)
  {
    SPI.beginTransaction (SPISettings (200000, MSBFIRST, SPI_MODE0));
    digitalWrite (SS, LOW);
    SPI.transfer (gears[gear]);
    digitalWrite (SS, HIGH);
    SPI.endTransaction ();
    lastsent = millis();
  }

  if (digitalRead(NEUTRAL) == 0)
  {
    inneutral = 1;
    gear = 0;
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
  else if (clutchpot < POT_MAX && clutchpot > 50) //50 is a hack for the shitty clutch pot, replace if the clutch pot changes
  {
    clutchpos = declutched;
  }
  else if (clutchpot > 50) //also hack here
  {
    clutchpos = (int) ((float) clutched + (((float) declutched - (float)  clutched) * (( (float) clutchpot - (float) POT_MIN) / ((float) POT_MAX - (float) POT_MIN))));
  }
  else
  {
    clutchpos = clutched;
  }
  clutch.write(clutchpos);

  if ((digitalRead(UPCHANGE) == 0) && (digitalRead(DOWNCHANGE) == 0)) //In case of both paddles at once
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
      if (!CUTUP || inneutral) //Don't igcut on neutral
      {
        clutch.write(declutched);
        delay(100);
        if (!inneutral)
        {
          gearchg.write(upchange);
        }
        else
        {
          gearchg.write(downchange);
        }
        delay(DELAY);
        gearchg.writeMicroseconds(midpointms);
        delay(150);
        clutch.write(clutchpos); //Don't reclutch unless the driver wants to
        delay(100);
      }
      else
      {
        digitalWrite(ECUCUT, 1);
        delay(PRECUTDELAY);
        if (CUTTYPE == ECUDELAY)
        {
          digitalWrite(ECUCUT, 0);
        }
        gearchg.write(upchange);
        delay(CUTDELAY);
        if (!CUTFULL)
        {
          digitalWrite(ECUCUT, 0);
        }
        gearchg.writeMicroseconds(midpointms);
        delay(POSTCUTDELAY);
        digitalWrite(ECUCUT, 0);
      }
      if(gear < 6)
      {
        gear++;
      }
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
      gearchg.write(downchange);
      delay(DELAY);
      gearchg.writeMicroseconds(midpointms);
      delay(150);
      for (int i = declutched; i > clutchpos; i--) //Slow down the clutch on downshift. Needs to be tuned
      {
        clutch.write(i);
        delay(5);
      }
      clutch.write(clutchpos); //Don't reclutch unless the driver wants to
      delay(100);
      if(gear > 1)
      {
        gear--;
      }
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
      gearchg.write(halfup);
      delay(DELAY);
      gearchg.writeMicroseconds(midpointms);
      delay(150);
      clutch.write(clutchpos); //Don't reclutch unless the driver wants to
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
    gearchg.writeMicroseconds(midpointms);
    delay(10);
  }
}
