#include "Arduino.h"
#include "uRTCLib.h"
#include "SevSeg.h"
#include <Button.h>
#include <ButtonEventCallback.h>
#include <PushButton.h>
#include <Bounce2.h>
#include "USBPrinter.h"

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

boolean started, turned, printed;
int tiltCounter;

// modes
const int PRINT = -1;
const int SHOW_TIME = 0;
const int SET_WEEKDAY = 1;
const int SET_DAY = 2;
const int SET_MONTH = 3;
const int SET_YEAR = 4;
const int SET_HOUR = 5;
const int SET_MINUTE = 6;

char daysOfWeek[7][9] = { "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi", "Dimanche" };
char months[12][10] = { "janvier", "février", "mars", "avril", "mai", "juin", "juillet", "août", "septembre", "octobre", "novembre", "décembre" };

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
  Serial.print(daysOfWeek[rtc.dayOfWeek()-1]);
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

class PrinterOper : public USBPrinterAsyncOper
{
  public:
    uint8_t OnInit(USBPrinter *pPrinter);
};

uint8_t PrinterOper::OnInit(USBPrinter *pPrinter)
{
  Serial.println(F("USB Printer OnInit"));
  Serial.print(F("Bidirectional="));
  Serial.println(pPrinter->isBidirectional());
  return 0;
}

USB     usb;
PrinterOper AsyncOper;
USBPrinter  p(&usb, &AsyncOper);

void setup()
{
  // seg display
  byte numDigits = 4;
  byte digitPins[] = {27, 33, 35, 36};
  byte segmentPins[] = {29, 37, 32, 28, 26, 31, 34, 30};

  sevseg.begin(COMMON_CATHODE, numDigits, digitPins, segmentPins);
  sevseg.setBrightness(40);

  Serial.begin(9600);
  //delay(1000);

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

  // USB
  if (usb.Init()) {
    Serial.println(F("USB host failed to initialize"));
    while (1) delay(1);
  }
}

const char ESC = 0x1B;
const char LF = 0x0A;

void printLabel(char* dayOfWeek, char* dayOfMonth, char* month, char* year, char* name)
{
  // Bold
  p.write(ESC);
  p.write('E');
  // Select font
  p.write(ESC);
  p.write('k');
  p.write(0x0A); // Brussel (outline font)
  // Font size
  p.write(ESC);
  p.write('X');
  p.write((uint8_t)0);
  p.write(0x38); // Quite big
  p.write((uint8_t)0);
  // Artificial command needed after font size
  p.write(ESC);
  p.write('5');
  //
  p.write(dayOfWeek);
  // Remove Bold
  p.write(ESC);
  p.write('F');
  // Select font
  p.write(ESC);
  p.write('k');
  p.write(0x0B); // Helsinki (outline font)
  //
  p.write(' ');
  p.write(dayOfMonth);
  p.write(LF);
  // Align
  p.write(ESC);
  p.write('a');
  p.write(0x02); // Right
  // Font size
  p.write(ESC);
  p.write('X');
  p.write((uint8_t)0);
  p.write(0x20); // Quite small
  p.write((uint8_t)0);
  // Italic
  p.write(ESC);
  p.write('4');
  //
  p.write(month);
  p.write(' ');
  p.write(year);
  // Remove italic
  p.write(ESC);
  p.write('5');
  //
  p.write(LF);
  // Align
  p.write(ESC);
  p.write('a');
  p.write((uint8_t)0x00); // Left
  // Font size
  p.write(ESC);
  p.write('X');
  p.write((uint8_t)0);
  p.write(0x28); // Medium
  p.write((uint8_t)0);
  if(name) {
    // Bold
    p.write(ESC);
    p.write('E');
    //
    p.write(name);
    p.write(LF);
  }
  p.write(LF);
  p.write(LF);
  p.write(LF);
  p.write(LF);
}

void loop()
{
  if(!started) {
    tone(BUZZER, 1000, 20);
    started = true;
  }

  if (p.isReady() && !printed) {
    Serial.println(F("Init ESC POS printer"));
    // Reset
    p.write(ESC);
    p.write('@');
    //
    p.write(LF);
    // Select Charset
    p.write(ESC);
    p.write('t');
    p.write(0x02); // Western (windows-1252)
    // Left margin
    p.write(ESC);
    p.write('k');
    p.write(0x01); // from column
    // Right margin
    p.write(ESC);
    p.write('Q');
    p.write(0x29); // to column

    rtc.refresh();
    char dayOfWeek[10];
    sprintf(dayOfWeek, "%s", daysOfWeek[rtc.dayOfWeek() - 1]);
    char dayOfMonth[3];
    sprintf(dayOfMonth, "%02d", rtc.day());
    char month[10];
    sprintf(month, "%s", months[rtc.month() - 1]);
    char year[5];
    sprintf(year, "20%02d", rtc.year());
    printLabel(dayOfWeek, dayOfMonth, month, year, 0);
    printed = true;
  }

  usb.Task();
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
    if(wasTurned) {
      tone(BUZZER, 440, 30);
      mode = PRINT;
      sevseg.blank();
      sevseg.refreshDisplay();
    }
  }
}