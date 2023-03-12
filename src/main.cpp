#include "Arduino.h"
#include "uRTCLib.h"
#include "SevSeg.h"
#include <Button.h>
#include <ButtonEventCallback.h>
#include <PushButton.h>
#include <Bounce2.h>

uRTCLib rtc(0x68);
SevSeg sevseg;

const int BUZZER = 3;
const int LED_R = 4;
const int LED_G = 5;
const int LED_B = 6;

const int BUTTON_LOW = 14;
const int BUTTON_TWO = 15;
const int BUTTON_ONE = 16;
const int BUTTON_FOUR = 17;
const int BUTTON_THREE = 18;

const int TILT_SWITCH = A12;

PushButton selectButton = PushButton(41);
PushButton incrButton = PushButton(40);

PushButton one = PushButton(BUTTON_ONE);
PushButton two = PushButton(BUTTON_TWO);
PushButton three = PushButton(BUTTON_THREE);
PushButton four = PushButton(BUTTON_FOUR);

boolean started, turned;
int tiltCounter;

char daysOfTheWeek[7][12] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};

// modes
const int PRINT = -1;
const int SHOW_TIME = 0;
const int SET_WEEKDAY = 1;
const int SET_DAY = 2;
const int SET_MONTH = 3;
const int SET_YEAR = 4;
const int SET_HOUR = 5;
const int SET_MINUTE = 6;

char dayCaptions[7][5] = {"lund", "nnar", "nner", "jeud", "vend", "sann", "dinn"};
char monthsCaptions[12][5] = {"janv", "fevr", "nnar", "avri", "nnai", "juin", "juil", "aout", "sept", "octo", "nove", "dece"};

int mode = PRINT;

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

void led(int r, int g, int b)
{
  analogWrite(LED_R, r);
  analogWrite(LED_G, g);
  analogWrite(LED_B, b);
}

void buttonDown(Button &button)
{
  int freq = 0;
  if(button.is(one)) {
    led(255, 20, 0);
    freq = 1000;
  }
  if (button.is(two))
  {
    led(0, 255, 20);
    freq = 2000;
  }
  if (button.is(three))
  {
    led(180, 200, 0);
    freq = 3000;
  }
  if (button.is(four))
  {
    led(200, 20, 150);
    freq = 4000;
  }
  tone(BUZZER, freq, 150);
}

void buttonUp(Button &button, uint16_t duration)
{
  led(0,0,0);
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

void updateTilt()
{
  boolean newTilt = analogRead(TILT_SWITCH) > 512;
  if(newTilt != turned) {
    tiltCounter++;
    if(tiltCounter > 30) {
      turned = newTilt;
      tiltCounter = 0;
    }
  } else {
    tiltCounter = 0;
  }
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

  // front buttons
  pinMode(14, OUTPUT);
  digitalWrite(14, LOW);

  one.onPress(buttonDown);
  two.onPress(buttonDown);
  three.onPress(buttonDown);
  four.onPress(buttonDown);

  one.onRelease(buttonUp);
  two.onRelease(buttonUp);
  three.onRelease(buttonUp);
  four.onRelease(buttonUp);

  // setting buttons
  selectButton.onPress(select);
  incrButton.onPress(increment);
  incrButton.onHoldRepeat(1000, 100, incrementHold);

  // LED
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
}

void loop()
{
  if(!started) {
    tone(BUZZER, 1000, 20);
    started = true;
  }
  boolean wasTurned = turned;
  updateTilt();
  if(turned) {
    if(!wasTurned )
    {
      mode = SHOW_TIME;
      tone(BUZZER, 440, 30);
    }
    selectButton.update();
    incrButton.update();
    sevseg.refreshDisplay();
    rtc.refresh();
    updateDisplay();
  } else {
    one.update();
    two.update();
    three.update();
    four.update();
    if(wasTurned)
      tone(BUZZER, 440, 30);
      mode = PRINT;
      sevseg.blank();
      sevseg.refreshDisplay();
  }
}