/*
 * Project Clock
 * Description:
 * Author:
 * Date:
 */

#include "application.h"
#include "LiquidCrystal/LiquidCrystal.h"

//LiquidCrystal lcd(A1, A0, D4, D5, D6, D7, D0, D1, D2, D3);
LiquidCrystal lcd(A1, A0, D4, D5, D6, D7);

// Sync every 3 hours
#define SYNC_INTERVAL (60 * 60 * 1000 * 3)
#define WEATHER_SYNC_INTERVAL (60 * 1000 * 30)
#define EXTERNAL_TEMP true //uncomment to use weather API instead of onboard thermometer

#define STATE_WAITING 0
#define STATE_PUBLISHED 1
#define STATE_RECEIVED 2
#define STATE_SYNC_WAIT 3
#define STATE_SYNC_COMPLETE 4

int ts_state = STATE_WAITING;
int offset = -5;
String tzName = "EST";

int tempPin = A2;
double temp = 0.0;
double tempF = 0.0;
int tempVoltage = 0;

String weather = "???";
int weather_state = STATE_WAITING;
unsigned long lastWeatherUpdate = 0;

void weatherHandler(const char *event, const char *data) {
  float fWeather = atoi(data);
  fWeather += 0.5;
  int iWeather = fWeather;
  weather = iWeather;
  weather_state = STATE_RECEIVED;
}

void updateWeather() {
  String data = "";
  switch (weather_state) {
    case STATE_WAITING:
      Serial.println("Sent timezone request.");
      Particle.publish("weather", data, PRIVATE);
      weather_state = STATE_PUBLISHED;
      break;
    case STATE_PUBLISHED:
      // handler changes state to STATE_RECEIVED, do nothing here
      Serial.println("Waiting for weather API hook...");
      break;
    case STATE_RECEIVED:
      Serial.println("Received updated weather.");
      weather_state = STATE_WAITING;
      lastWeatherUpdate = millis();
      break;
  }
}

void timezoneHandler(const char *event, const char *data) {

  char* token = strtok(strdup(data)," ");
  offset = (atoi(token) / 3600);
  token = strtok(NULL, " ");
  tzName = token;

  ts_state = STATE_RECEIVED;
  Serial.print("Set offset to ");
  Serial.println(offset, DEC);

  Serial.println("Set tzName to " + tzName);
}

void syncTime() {
  String data = "";
  switch (ts_state) {
    case STATE_WAITING:
      Serial.println("Sent timezone request.");
      Particle.publish("timezone", data, PRIVATE);
      ts_state = STATE_PUBLISHED;
      break;
    case STATE_PUBLISHED:
      // handler changes state to STATE_RECEIVED, do nothing here
      Serial.println("Waiting for timezone API hook...");
      break;
    case STATE_RECEIVED:
      Serial.println("Received time zone, requesting sync.");
      //Time.zone(offset);
      Particle.syncTime();
      ts_state = STATE_SYNC_WAIT;
      break;
    case STATE_SYNC_WAIT:
      if (Particle.syncTimePending()) {
	Serial.println("Waiting for Particle sync");
      } else {
        Serial.println("Received sync from Particle");
        ts_state = STATE_SYNC_COMPLETE;
      }
      break;
    case STATE_SYNC_COMPLETE:
      ts_state = STATE_WAITING;
      Serial.println("Synchronized time to " + Time.format(Time.now(), TIME_FORMAT_ISO8601_FULL));
      delay(250);
      break;
  }
}

// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin(9600);
  lcd.clear();
  lcd.begin(16,2);
  lcd.print("Initializing...");
  syncTime();

  pinMode(tempPin, INPUT);
  Particle.variable("tempVoltage", &tempVoltage, INT);
  Particle.variable("temp", &temp, DOUBLE);
  Particle.variable("tempF", &tempF, DOUBLE);
  Particle.variable("weather", &weather, STRING);

  Particle.subscribe("hook-response/timezone",timezoneHandler,MY_DEVICES);
  Particle.subscribe("hook-response/weather",weatherHandler,MY_DEVICES);

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  unsigned long lastSync = Particle.timeSyncedLast();
  if(millis() - lastSync > SYNC_INTERVAL) {
    Serial.println("Last sync " + Time.format(lastSync, TIME_FORMAT_ISO8601_FULL));
    syncTime();
  }

  if(lastSync == 0) {
    lcd.setCursor(0,0);
    lcd.print("Syncing time...");
    lcd.setCursor(0,1);
    lcd.print("                ");
    return;
  }

#ifndef EXTERNAL_TEMP
  tempVoltage = analogRead(tempPin);
  temp = (((tempVoltage * 3.3)/4095) - 0.5) * 100;
  tempF = ((temp * 9.0) / 5.0) + 32.0;
  String strTemp = (String)((int)tempF);
#else
  if ((millis() - lastWeatherUpdate > WEATHER_SYNC_INTERVAL) || lastWeatherUpdate == 0) {
    updateWeather();
  }
  String strTemp = weather;
#endif

  Time.zone(offset);
  int currentTime = Time.now();
  String strDOW = Time.format(currentTime,"%a");
  String strDate = Time.format(currentTime, "%Y-%m-%d");
  String strTime = Time.format(currentTime, "%H:%M:%S");

  lcd.setCursor(0,0);
  lcd.print(" " + strDOW + " " + strDate + "  ");
  lcd.setCursor(0,1);
  lcd.print(strTime + " " + tzName + " " + strTemp + (char)223);
  delay(100);
}
