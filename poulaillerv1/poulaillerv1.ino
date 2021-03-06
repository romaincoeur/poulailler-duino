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
const int MANUAL_UP = 8; // Set the door open in a manual mode
const int MANUAL_DOWN = 7; // Set the door closed in a manual mode
const unsigned long MAX_TIME = 10; // Limite de temps ouveture ou fermeture avant anomalie en secondes
const int DAY = 6;
const int NIGHT = 23;
const boolean DEBUG = false;
const int CHECK_FREQUENCY = 1000; // Fréquence de vérification de la nuit ou du jour (en ms)

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
  if (!RTC.isrunning()) {
    if (DEBUG) Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

  if (DEBUG) {
    logEvent("");
    if (doorState) Serial.println("Door open");
    else Serial.println("Door closed");
  }

  pinMode(LED, OUTPUT);   // tell Arduino LED is an output
  pinMode(BUTEE_UP, INPUT); // and BUTTON is an input
  pinMode(BUTTE_BOTTOM, INPUT);
  pinMode(MANUAL_UP, INPUT);
  pinMode(MANUAL_DOWN, INPUT);
  pinMode(MOTEUR_UP, OUTPUT);
  pinMode(MOTEUR_DOWN, OUTPUT);
}


bool openDoor() {
  if (DEBUG) logEvent("Open Door");
  DateTime start = RTC.now();
  while (state_butee_up == 0) {
    val_butee_up = digitalRead(BUTEE_UP);
    if (val_butee_up == HIGH) state_butee_up = 1;
    digitalWrite(MOTEUR_UP, HIGH);
    if (start.unixtime() + MAX_TIME <= RTC.now().unixtime()) return false; // Unix
    // if (start.second() + MAX_TIME <= RTC.now().second()) return false; // Windows
  }
  doorState = 1;
  state_butee_up = 0;
  digitalWrite(MOTEUR_UP, LOW);
  return true;
}

bool closeDoor() {
  if (DEBUG) logEvent("Close Door");
  DateTime start = RTC.now();
  while (state_butee_bottom == 0) {
    val_butee_bottom = digitalRead(BUTTE_BOTTOM);
    if (val_butee_bottom == HIGH) state_butee_bottom = 1;
    digitalWrite(MOTEUR_DOWN, HIGH);
    if (start.unixtime() + MAX_TIME <= RTC.now().unixtime()) return false; // Unix
    // if (start.second() + MAX_TIME <= RTC.now().second()) return false; // Windows
  }
  doorState = 0;
  state_butee_bottom = 0;
  digitalWrite(MOTEUR_DOWN, LOW);
  return true;
}

void die() {
  if (DEBUG) logEvent("Dying...");
  digitalWrite(MOTEUR_DOWN, LOW);
  digitalWrite(MOTEUR_UP, LOW);
  while (true) {
    digitalWrite(LED, HIGH);
    delay(300);
    digitalWrite(LED, LOW);
    delay(300);
  }

}


void loop() {
  if (digitalRead(MANUAL_UP) || digitalRead(MANUAL_DOWN)) {
    // Manual mode
    if (digitalRead(MANUAL_UP) && doorState == 0) {
      if (DEBUG) logEvent("MANUAL UP MODE");
      if (!openDoor()) die();
    } else if (digitalRead(MANUAL_DOWN) && doorState == 1) {
      if (DEBUG) logEvent("MANUAL DOWN MODE");
      if (!closeDoor()) die();
    }
  } else {
    // Auto mode
    if (DEBUG) logEvent("AUTO MODE");
    int now = RTC.now().hour();
    if (now >= DAY && now < NIGHT) {
      if (doorState==0) {
        if (!openDoor()) die();
      }
    } else if (doorState == 1) {
      if (!closeDoor()) die();
    }
  }
  
  delay(CHECK_FREQUENCY);
}


void logEvent(String message)
{
  showTime(RTC.now().unixtime()); // Unix
  // showTime(RTC.now().second()); // Windows
  Serial.print(" - ");
  Serial.print(message);
  Serial.println();
}


void showTime(time_t t) {
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

void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

