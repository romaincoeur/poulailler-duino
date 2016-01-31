// Commande automatique d'ouverture et fermeture d'une porte de poulailler

#include "RTClib.h"
#include "Time.h"
#include <Wire.h>

RTC_DS1307 RTC;

const int LED = 13;   // the pin for the LED
const int BUTEE_UP = 12;
const int BUTTE_BOTTOM = 11;
const int MOTEUR_UP = 10;
const int MOTEUR_DOWN = 9;
const unsigned long MAX_TIME = 10; // Limite de temps ouveture ou fermeture avant anomalie en secondes
const int DAY = 8;
const int NIGHT = 19;
const boolean DEBUG = true;

int val_butee_up = 0;
int val_butee_bottom = 0;
int old_val_butee_up = 0; // old variables store the previous values
int old_val_butee_bottom = 0;
int state_butee_up = 0;	// 0 = Butee off and 1 = Butee on
int state_butee_bottom = 0;
int doorState = 0;          // 0 closed and 1 open


void setup() {
  if (DEBUG) Serial.begin(57600);
  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning()) {
    if (DEBUG) Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  
  pinMode(LED, OUTPUT);   // tell Arduino LED is an output
  pinMode(BUTEE_UP, INPUT); // and BUTTON is an input
  pinMode(BUTTE_BOTTOM, INPUT);
  pinMode(MOTEUR_UP, OUTPUT);
  pinMode(MOTEUR_DOWN, OUTPUT);
}


bool openDoor() {
  if (DEBUG) logEvent("Open Door");
  DateTime start = RTC.now();
  while (state_butee_up==0) {
    val_butee_up = digitalRead(BUTEE_UP);
    if (val_butee_up == HIGH) state_butee_up = 1;
    digitalWrite(MOTEUR_UP, HIGH);
    if (start.unixtime()+MAX_TIME <= RTC.now().unixtime()) return false;
  }
  doorState = 1;
  state_butee_up = 0;
  digitalWrite(MOTEUR_UP, LOW);
  return true;
}

bool closeDoor() {
  if (DEBUG) logEvent("Close Door");
  DateTime start = RTC.now();
  while (state_butee_bottom==0) {
    val_butee_bottom = digitalRead(BUTTE_BOTTOM);
    if (val_butee_bottom == HIGH) state_butee_bottom = 1;
    digitalWrite(MOTEUR_DOWN, HIGH);
    if (start.unixtime()+MAX_TIME <= RTC.now().unixtime()) return false;
  }
  doorState = 0;
  state_butee_bottom = 0;
  digitalWrite(MOTEUR_DOWN, LOW);
  return true;
}

void die() {
  digitalWrite(MOTEUR_DOWN, LOW);
  digitalWrite(MOTEUR_UP, LOW);
  while (true) {
    digitalWrite(LED, HIGH);
    delay(300);
    digitalWrite(LED, LOW);
    delay(300);
  }
  
}


void loop(){
  if (RTC.now().hour() >= DAY && RTC.now().hour() < NIGHT && doorState == 0) {
    if (!openDoor())
      die();
  } else if ((RTC.now().hour() >= NIGHT || RTC.now().hour() < DAY) && doorState == 1) {
    if (!closeDoor())
      die();
  }
}


void logEvent(String message)
{
   showTime(RTC.now().unixtime());
   Serial.print(" - ");
   Serial.print(message);
   Serial.println();
}


void showTime(time_t t){
  // display the given time 
  Serial.print(day(t));
  Serial.print(" ");
  Serial.print(month(t));
  Serial.print(" ");
  Serial.print(year(t));
  Serial.print(" ");
  Serial.print(hour(t));
  printDigits(minute(t));
  printDigits(second(t));
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

