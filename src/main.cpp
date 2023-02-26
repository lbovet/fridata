#include "Arduino.h"
#include "uRTCLib.h"
#include "SevSeg.h"
#include <Button.h>
#include <ButtonEventCallback.h>
#include <PushButton.h>
#include <Bounce2.h>

uRTCLib rtc(0x68);
SevSeg sevseg;

PushButton selectButton = PushButton(41);
PushButton incrButton = PushButton(40);

char daysOfTheWeek[7][12] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};

// modes
const int SHOW_TIME = 0;
const int SET_WEEKDAY = 1;
const int SET_DAY = 2;
const int SET_MONTH = 3;
const int SET_YEAR = 4;
const int SET_HOUR = 5;
const int SET_MINUTE = 6;

char dayCaptions[7][5] = {"lund", "nnar", "nner", "jeud", "vend", "sann", "dinn"};
char monthsCaptions[12][5] = {"janv", "fevr", "nnar", "avri", "nnai", "juin", "juil", "aout", "sept", "octo", "nove", "dece"};

int mode = SHOW_TIME;

void send()
{
  Serial.print("Current Date & Time: ");
  Serial.print(rtc.year());
  Serial.print('/');
  Serial.print(rtc.month());
  Serial.print('/');
  Serial.print(rtc.day());

  Serial.print(" (");
  Serial.print(daysOfTheWeek[rtc.dayOfWeek()-1]);
  Serial.print(") ");

  Serial.print(rtc.hour());
  Serial.print(':');
  Serial.print(rtc.minute());
  Serial.print(':');
  Serial.println(rtc.second());
}

char display[5] = "----";

void updateDisplay()
{
  switch (mode)
  {
  case SHOW_TIME:
    if(rtc.second() % 2 == 0)
      sprintf(display, "%02d.%02d", rtc.hour(), rtc.minute());
    else
      sprintf(display, "%02d%02d", rtc.hour(), rtc.minute());
    break;
  case SET_YEAR:
    sprintf(display, "20%02d", rtc.year());
    break;
  case SET_MONTH:
    sprintf(display, "%s", monthsCaptions[rtc.month() - 1]);
    break;
  case SET_DAY:
    sprintf(display, "j %02d", rtc.day());
    break;
  case SET_HOUR:
    sprintf(display, "h %02d", rtc.hour());
    break;
  case SET_MINUTE:
    sprintf(display, "nn%02d", rtc.minute());
    break;
  case SET_WEEKDAY:
    sprintf(display, "%s", dayCaptions[rtc.dayOfWeek() - 1]);
    break;

  default:
    break;
  }
  sevseg.setChars(display);
}

void select(Button &button)
{
  if (mode < SET_MINUTE)
  {
    mode++;
  }
  else
  {
    mode = SHOW_TIME;
  }
  send();
}

void increment(Button &button)
{
  switch (mode)
  {
  case SET_YEAR:
    if (rtc.year() < 50)
      rtc.set(rtc.second(), rtc.minute(), rtc.hour(), rtc.dayOfWeek(), rtc.day(), rtc.month(), rtc.year() + 1);
    else
      rtc.set(rtc.second(), rtc.minute(), rtc.hour(), rtc.dayOfWeek(), rtc.day(), rtc.month(), 20);
    break;
  case SET_MONTH:
    if (rtc.month() < 12)
      rtc.set(rtc.second(), rtc.minute(), rtc.hour(), rtc.dayOfWeek(), rtc.day(), rtc.month() + 1, rtc.year());
    else
      rtc.set(rtc.second(), rtc.minute(), rtc.hour(), rtc.dayOfWeek(), rtc.day(), 1, rtc.year());
    break;
  case SET_DAY:
    if (rtc.day() < 31)
      rtc.set(rtc.second(), rtc.minute(), rtc.hour(), rtc.dayOfWeek(), rtc.day() + 1, rtc.month(), rtc.year());
    else
      rtc.set(rtc.second(), rtc.minute(), rtc.hour(), rtc.dayOfWeek(), 1, rtc.month(), rtc.year());
    break;
  case SET_HOUR:
    if (rtc.hour() < 23)
      rtc.set(rtc.second(), rtc.minute(), rtc.hour() + 1, rtc.dayOfWeek(), rtc.day(), rtc.month(), rtc.year());
    else
      rtc.set(rtc.second(), rtc.minute(), 0, rtc.dayOfWeek(), rtc.day(), rtc.month(), rtc.year());
    break;
  case SET_MINUTE:
    if (rtc.minute() < 59)
      rtc.set(rtc.second(), rtc.minute()+1, rtc.hour(), rtc.dayOfWeek(), rtc.day(), rtc.month(), rtc.year());
    else
      rtc.set(rtc.second(), 0, rtc.hour(), rtc.dayOfWeek(), rtc.day(), rtc.month(), rtc.year());
    break;
  case SET_WEEKDAY:
    if (rtc.dayOfWeek() < 7)
      rtc.set(rtc.second(), rtc.minute(), rtc.hour(), rtc.dayOfWeek() + 1, rtc.day(), rtc.month(), rtc.year());
    else
      rtc.set(rtc.second(), rtc.minute(), rtc.hour(), 1, rtc.day(), rtc.month(), rtc.year());
    break;
  default:
    break;
  }
  rtc.refresh();
  send();
}

void incrementHold(Button &button, uint16_t duration, uint16_t repeatCount)
{
  increment(button);
}

void setup()
{
  // seg display
  byte numDigits = 4;
  byte digitPins[] = {27, 33, 35, 36};
  byte segmentPins[] = {29, 37, 32, 28, 26, 31, 34, 30};

  sevseg.begin(COMMON_CATHODE, numDigits, digitPins, segmentPins);
  sevseg.setBrightness(40);

  Serial.begin(9600);
  delay(1000);

  URTCLIB_WIRE.begin();
  //rtc.set(10, 32, 12, 6, 25, 2, 23);

  // button
  selectButton.onPress(select);
  incrButton.onPress(increment);
  incrButton.onHoldRepeat(1000, 100, incrementHold);
}

void loop()
{
  selectButton.update();
  incrButton.update();
  sevseg.refreshDisplay();
  rtc.refresh();
  updateDisplay();
}