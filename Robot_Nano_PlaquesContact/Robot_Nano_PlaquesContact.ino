#include <Wire.h>
#define CONTACT 1

int button1 = 2;
int ledR1 = 3;
int button2 = 4;
int ledR2 = 5;
int ledB2 = 6;
int button3 = 7;
int ledR3 = 8;
int button4 = 11;
int ledR4 = 10;
int ledB4 = 9;
int contact = 0;
int led[] = {ledR1, ledR2, ledR3, ledR4};
int ledShield[] = {ledB2, ledB4};
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned long currentTime = 0;
int gotHit = 0;
int invincible = 0;
byte shield = 9;
int x = 0;
int lifeDelta = 0; //1 => Bouclier activé, touché sur le bouclier => +1 point de vie ; 0 => Pas touché ; 2 => touché, -1 PV
int previousLifeDelta = 0;
int nbLed = 0;
int nbLedShield = 1;
int shieldState = 0; //0 => bouclier pas activé; 1 => bouclier activé
int state = 1;
void setup() {
  Wire.begin(CONTACT);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  Serial.begin(9600);
  pinMode(ledR1, OUTPUT);
  pinMode(ledR2, OUTPUT);
  pinMode(ledB2, OUTPUT);
  pinMode(ledR3, OUTPUT);
  pinMode(ledR4, OUTPUT);
  pinMode(ledB4, OUTPUT);

  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);
}

void checkContacts() {

  if (lifeDelta == 0)
  {
    if (digitalRead(button4) == 0)
    {
      contact = 2;  //Plaque de contact avec bouclier
      nbLed = 3;
      //Serial.println("bouton 4 touché");
    }
    else if (digitalRead(button3) == 0)
    {
      contact = 1;  //Plaque de contact sans bouclier
      nbLed = 2;
    //Serial.println("bouton 3 touché");
    }
    else if (digitalRead(button2) == 0)
    {
      contact = 1; //Plaque de contact sans bouclier
      nbLed = 1;
      //Serial.println("bouton 2 touché");
    }
    else if (digitalRead(button1) == 0)
    {
      contact = 1;  //Plaque de contact sans bouclier
      nbLed = 0;
      //Serial.println("bouton 1 touché");
    }
    else
    {
      contact = 0;
    }
    if (contact != 0 && shieldState == 0 && invincible == 0)
    {
      //Serial.print("got hit");
      gotHit = 1;
      lifeDelta = 2;
      resetTime();
    }
    if (shieldState == 1 && contact == 2)
    {
      lifeDelta = 1;
      //Serial.println("YAY 1HP IM TU GUD");
    }
  }

  if (digitalRead(button1) == 1 && digitalRead(button2) == 1 && digitalRead(button3) == 1 && digitalRead(button4) == 1)
  {
    invincible = 0;
  }
}

void shieldCheck()
{
  if (shieldState == 1)
  {
    invincible = 1;
    digitalWrite(ledShield[nbLedShield], HIGH);
  }
  else 
  {
    digitalWrite(ledShield[nbLedShield], LOW);
  }
}

void invincibleCheck()
{
  invincible = 1;
  if (currentTime > 2000)
  {
    gotHit = 0;
  }
}
void resetTime()
{

  previousMillis = currentMillis;

}

void hitLed(int nbLed) {

  if (currentTime < 500)
  {
    digitalWrite(led[nbLed], HIGH);
  }
  else if (currentTime < 550)
  {
    digitalWrite(led[nbLed], LOW);
  }
  else if (currentTime < 1000)
  {
    digitalWrite(led[nbLed], HIGH);
  }
  else if (currentTime < 1050)
  {
    digitalWrite(led[nbLed], LOW);
  }
  else if (currentTime < 1500)
  {
    digitalWrite(led[nbLed], HIGH);
  }
  else if (currentTime < 1550)
  {
    digitalWrite(led[nbLed], LOW);
  }
  else if (currentTime < 2000)
  {
    digitalWrite(led[nbLed], HIGH);
  }
}

void stopHitLed(int nbLed) {

  if (shieldState == 0 || lifeDelta == 2 && shieldState == 1)
  {
    lifeDelta = 0;
  }
  digitalWrite(led[nbLed], LOW);
}

void loop() {
 
  currentMillis = millis();
  currentTime = currentMillis - previousMillis;

  //Serial.println(currentTime);
  shieldCheck();

  if (gotHit == 1)
  {
    invincibleCheck();
    hitLed(nbLed);
  }
  else
  {
    stopHitLed(nbLed);
    checkContacts();
  }


}
void requestEvent(int howMany) {
  
  Serial.println("request Event");
  /*if (shieldState == 1 && contact == 2) {*/
  Wire.write(lifeDelta);/*
  }
  else Wire.write(contactState); */
  Serial.println(lifeDelta);
}

void receiveEvent(int howMany) {
  // state = Wire.read();

  shieldState = Wire.read();

  Serial.print("State : ");
  //Serial.print(state);
  //Serial.print("  shieldState : ");
  //Serial.println(shieldState);
  // //Serial.println(shieldState);
  //Master wants to know button state

  //    else  //Master indicates slave what to do
  //    {
  //      int shield = Wire.read();
  //      int button = Wire.read();
  //    }
}
