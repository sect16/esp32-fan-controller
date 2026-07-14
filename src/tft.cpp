#include "config.h"
#include "fanPWM.h"
#include "fanTacho.h"
#include "log.h"
#include "tft.h"
#include "WiFi.h"

#ifdef DRIVER_ILI9341
#include <Adafruit_ILI9341.h>
#include <FreeSans9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#endif
#ifdef DRIVER_ST7735
#include <Adafruit_ST7735.h>
#endif

// prepare driver for display
#ifdef DRIVER_ILI9341
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
#endif
#ifdef DRIVER_ST7735
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
#endif

#ifdef useTFT
const GFXfont *myFont;
const GFXfont *myFontM;
const GFXfont *myFontB;
int textSizeOffset;

// number of screen to display
int screen = SCREEN_NORMALMODE;
int lastRSSI;

unsigned long startCountdown = 0;
void calcDimensionsOfElements(void);
void draw_screen(void);

int16_t tft_getWidth(void) { return tft.width(); }
int16_t tft_getHeight(void) { return tft.height(); }
void tft_fillScreen(void) { tft.fillScreen(TFT_BLACK); }

int16_t getRelativeX(int16_t xBasedOnTFTwithScreenWidth320px)
{
  return (float)(xBasedOnTFTwithScreenWidth320px) / 320.0f * tft_getWidth();
}

int16_t getRelativeY(int16_t yBasedOnTFTwithScreenHeight240px)
{
  return (float)(yBasedOnTFTwithScreenHeight240px) / 240.0f * tft_getHeight();
}

// rect: x, y, width, height
int valueUpRect[4];
int valueDownRect[4];
#if defined(useStandbyButton) || defined(useShutdownButton)
int shutdownRect[4];
int confirmShutdownYesRect[4];
int confirmShutdownNoRect[4];
#endif

int plusMinusHorizontalLineMarginLeft;
int plusMinusHorizontalLineMarginTop;
int plusMinusHorizontalLineLength;
int plusMinusVerticalLineMarginTop;
int plusMinusVerticalLineLength;
int plusMinusVerticalLineMarginLeft;

int tempAreaLeft;
int tempAreaTop;
int tempAreaWidth;
int fanAreaLeft;
int fanAreaTop;
int fanAreaWidth;
int ambientAreaLeft;
int ambientAreaTop;
int ambientAreaWidth;

#if defined(useStandbyButton) || defined(useShutdownButton)
int shutdownWidthAbsolute;
int shutdownHeightAbsolute;
#endif

#endif // useTFT

void initTFT(void)
{
#ifdef useTFT
#ifdef DRIVER_ILI9341
  analogWrite(TFT_LED, TFT_PWM_DUTY);
  tft.begin();
  myFont = &FreeSans9pt7b;
  myFontM = &FreeMono9pt7b;
  myFontB = &FreeSansBold9pt7b;
  textSizeOffset = 0;
  lastRSSI = 9;
#endif
#ifdef DRIVER_ST7735
  tft.initR(INITR_BLACKTAB);
  myFont = NULL;
  textSizeOffset = 0;
#endif

  tft.setFont(myFont);
  tft.setRotation(TFT_ROTATION);

  calcDimensionsOfElements();

  tft.fillScreen(TFT_BLACK);
  draw_screen();

  Log.printf("  TFT successfully initialized.\r\n");
  Log.printf("  tftx = %d, tfty = %d\r\n", tft.width(), tft.height());
#else
  Log.printf("    TFT is disabled in config.h\r\n");
#endif
}

#ifdef useTFT
void calcDimensionsOfElements(void)
{
  int marginTopAbsolute = 12;
  int marginLeftAbsolute = 14;
  int areaHeightAbsolute = (240 - 4 * marginTopAbsolute) / 3;

  int valueUpDownWidthAbsolute = 80;
  int valueUpDownHeightAbsolute = 55;
#if defined(useStandbyButton) || defined(useShutdownButton)
  shutdownWidthAbsolute = 40;
  shutdownHeightAbsolute = 40;
#endif
  int valueUpRectTop;
  int valueDownRectTop;
#if defined(useStandbyButton) || defined(useShutdownButton)
  int shutdownRectTop;
#endif

  tempAreaLeft = getRelativeX(marginLeftAbsolute);
  fanAreaLeft = getRelativeX(marginLeftAbsolute);
  ambientAreaLeft = getRelativeX(marginLeftAbsolute);
  tempAreaTop = getRelativeY(marginTopAbsolute) + 15;
  fanAreaTop = getRelativeY(marginTopAbsolute + areaHeightAbsolute + marginTopAbsolute) + 20;
  valueUpRectTop = fanAreaTop;
  ambientAreaTop = getRelativeY(marginTopAbsolute + areaHeightAbsolute + marginTopAbsolute + areaHeightAbsolute + marginTopAbsolute) + 30;
  valueDownRectTop = ambientAreaTop;

#if defined(useStandbyButton) || defined(useShutdownButton)
  tempAreaWidth = getRelativeX(320 - marginLeftAbsolute - shutdownWidthAbsolute - marginLeftAbsolute);
  shutdownRectTop = getRelativeY(240 - shutdownHeightAbsolute - marginTopAbsolute);
#else
  tempAreaWidth = getRelativeX(320 - marginLeftAbsolute);
#endif

#ifdef useTouch
  fanAreaWidth = getRelativeX(320 - marginLeftAbsolute - valueUpDownWidthAbsolute - marginLeftAbsolute);
  ambientAreaWidth = fanAreaWidth;
#else
  fanAreaWidth = getRelativeX(320 - marginLeftAbsolute);
  ambientAreaWidth = fanAreaWidth;
#endif

  valueUpRect[0] = getRelativeX(320 - valueUpDownWidthAbsolute - marginLeftAbsolute);
  valueUpRect[1] = valueUpRectTop;
  valueUpRect[2] = getRelativeX(valueUpDownWidthAbsolute);
  valueUpRect[3] = getRelativeY(valueUpDownHeightAbsolute);

  valueDownRect[0] = getRelativeX(320 - valueUpDownWidthAbsolute - marginLeftAbsolute);
  valueDownRect[1] = valueDownRectTop;
  valueDownRect[2] = getRelativeX(valueUpDownWidthAbsolute);
  valueDownRect[3] = getRelativeY(valueUpDownHeightAbsolute);

  plusMinusHorizontalLineLength = (valueUpRect[2] / 2) - 4;
  plusMinusHorizontalLineMarginLeft = (valueUpRect[2] - plusMinusHorizontalLineLength) / 2;
  plusMinusHorizontalLineMarginTop = valueUpRect[3] / 2;

  plusMinusVerticalLineLength = plusMinusHorizontalLineLength;
  plusMinusVerticalLineMarginTop = (valueUpRect[3] - plusMinusVerticalLineLength) / 2;
  plusMinusVerticalLineMarginLeft = valueUpRect[2] / 2;

#if defined(useStandbyButton) || defined(useShutdownButton)
  shutdownRect[0] = getRelativeX(320 - shutdownWidthAbsolute - marginLeftAbsolute);
  shutdownRect[1] = shutdownRectTop;
  shutdownRect[2] = getRelativeX(shutdownWidthAbsolute);
  shutdownRect[3] = getRelativeY(shutdownHeightAbsolute);

  confirmShutdownYesRect[0] = getRelativeX(40);
  confirmShutdownYesRect[1] = getRelativeY(90);
  confirmShutdownYesRect[2] = getRelativeX(60);
  confirmShutdownYesRect[3] = getRelativeY(60);
  confirmShutdownNoRect[0] = getRelativeX(200);
  confirmShutdownNoRect[1] = getRelativeY(90);
  confirmShutdownNoRect[2] = getRelativeX(60);
  confirmShutdownNoRect[3] = getRelativeY(60);
#endif
}

void printText(int areaX, int areaY, int areaWidth, int lineNr, const char *str, uint8_t textSize, const GFXfont *f, bool wipe, int foreground)
{
  GFXcanvas1 testCanvas(tft_getWidth(), tft_getHeight());
  int16_t x1, y1;
  uint16_t w, h;
  testCanvas.setFont(f);
  testCanvas.setTextSize(textSize);
  testCanvas.setTextWrap(false);
  testCanvas.getTextBounds("0WIYgy,", 0, 0, &x1, &y1, &w, &h);

  int textHeight = h;
  int textAreaHeight = textHeight + 2;
  int textAreaOffset = -y1;

#ifdef useTouch
  tft.setFont(f);
  tft.setTextSize(textSize);
  tft.setTextWrap(false);
  if (wipe)
  {
    tft.fillRect(areaX, areaY + lineNr * textAreaHeight, areaWidth, textAreaHeight, TFT_BLACK);
  }
  tft.setCursor(areaX, areaY + textAreaOffset + lineNr * textAreaHeight);
  tft.print(str);
#else
  GFXcanvas1 canvas(areaWidth, textAreaHeight);
  canvas.setFont(f);
  canvas.setTextSize(textSize);
  canvas.setTextWrap(false);
  canvas.setCursor(0, textAreaOffset);
  canvas.print(str);
  tft.drawBitmap(areaX, areaY + lineNr * textAreaHeight, canvas.getBuffer(), areaWidth, textAreaHeight, foreground, TFT_BLACK);
#endif
}

void switchOff_screen(boolean switchOff)
{
  if (switchOff)
  {
    Log.printf("  Will switch TFT off.\r\n");
    digitalWrite(TFT_LED, !LED_ON);
  }
  else
  {
    Log.printf("  Will switch TFT on.\r\n");
    digitalWrite(TFT_LED, LED_ON);
  }
}

void draw_screen(void)
{
  char buffer[100];
  char percentEscaped = 0x25;
  char degreesSymbol = 0x7F;

  if (screen == SCREEN_NORMALMODE)
  {
    int color = TFT_WHITE;
    /*
    sprintf(buffer, "Radiator Temperature:");
    printText(tempAreaLeft, tempAreaTop - 25, tempAreaWidth - 50, 0, buffer, textSizeOffset + 1, myFontB, false, color);

    if (temperature[0] <= temperatureMin) color = TFT_GREEN;
    else if (temperature[0] <= temperatureMin + (temperatureMax - temperatureMin) / 2) color = TFT_YELLOW;
    else color = TFT_RED;

    sprintf(buffer, "%.1f", temperature[0]);
    printText(tempAreaLeft, tempAreaTop, tempAreaWidth - 230, 0, buffer, textSizeOffset + 2, myFont, true, color);
    sprintf(buffer, "%.1f", temperature[1]);
    printText(tempAreaLeft, tempAreaTop, tempAreaWidth - 230, 1, buffer, textSizeOffset + 2, myFont, true, color);

    color = TFT_WHITE;
    sprintf(buffer, "%cC High", degreesSymbol);
    printText(tempAreaLeft + 70, tempAreaTop, tempAreaWidth - 150, 0, buffer, textSizeOffset + 2, myFont, false, color);
    sprintf(buffer, "%cC Low", degreesSymbol);
    printText(tempAreaLeft + 70, tempAreaTop, tempAreaWidth, 1, buffer, textSizeOffset + 2, myFont, false, color);
    */
    printText(fanAreaLeft, fanAreaTop, fanAreaWidth, 0, "RPM:    Load:     Duty / Target:", textSizeOffset + 1, myFontB, false, TFT_WHITE);

    int load1 = 100 * rpm[0] / FANMAXRPM1;
    if (load1 < 1)
      color = TFT_WHITE;
    else if (load1 < 50)
      color = TFT_GREEN;
    else if (load1 < 80)
      color = TFT_YELLOW;
    else
      color = TFT_RED;
    sprintf(buffer, "%4d (%3d%c) %3d / 256 (%3d%c)", rpm[0], load1, percentEscaped, pwm1, (100 * pwm1) / 256, percentEscaped);
    printText(fanAreaLeft, fanAreaTop + 8, fanAreaWidth, 0 + 1, buffer, textSizeOffset + 1, myFontM, true, color);
    int load2 = 100 * rpm[1] / FANMAXRPM2;
    if (load2 < 1)
      color = TFT_WHITE;
    else if (load2 < 50)
      color = TFT_GREEN;
    else if (load2 < 80)
      color = TFT_YELLOW;
    else
      color = TFT_RED;
    sprintf(buffer, "%4d (%3d%c) %3d / 256 (%3d%c)", rpm[1], load2, percentEscaped, pwm2, (100 * pwm2) / 256, percentEscaped);
    printText(fanAreaLeft, fanAreaTop + 8, fanAreaWidth, 1 + 1, buffer, textSizeOffset + 1, myFontM, true, color);

    int ambientLine = 0;
    color = TFT_WHITE;
    sprintf(buffer, "Flow Rate:");
    printText(ambientAreaLeft, ambientAreaTop, ambientAreaWidth - 210, ambientLine, buffer, textSizeOffset + 1, myFontB, false, color);

    if (rpm[3] < 50)
      color = TFT_RED;
    else if (rpm[3] < 100)
      color = TFT_YELLOW;
    else
      color = TFT_GREEN;
    sprintf(buffer, "%.0f L/hour", rpm[3] * 60 * 0.01);
    printText(ambientAreaLeft + 100, ambientAreaTop, ambientAreaWidth, ambientLine++, buffer, textSizeOffset + 2, myFont, true, color);

    int8_t rssi = WiFi.RSSI();
    int xOffset = 190;

    if (!WiFi.isConnected())
      rssi = 0;
    else if (rssi >= -55)
      rssi = 5;
    else if (rssi < -55 && rssi > -65)
      rssi = 4;
    else if (rssi < -65 && rssi > -75)
      rssi = 3;
    else if (rssi < -75 && rssi > -85)
      rssi = 2;
    else if (rssi < -85 && rssi > -96)
      rssi = 1;
    else
      rssi = 0;

    if (lastRSSI != rssi)
    {
      int yBar1 = 18, yBar2 = 16, yBar3 = 12, yBar4 = 8, yBar5 = 4;
      if (rssi == 5)
      {
        tft.fillRect(xOffset + 102, yBar1, 4, 2, TFT_GREEN);
        tft.fillRect(xOffset + 107, yBar2, 4, 4, TFT_GREEN);
        tft.fillRect(xOffset + 112, yBar3, 4, 8, TFT_GREEN);
        tft.fillRect(xOffset + 117, yBar4, 4, 12, TFT_GREEN);
        tft.fillRect(xOffset + 122, yBar5, 4, 16, TFT_GREEN);
      }
      else if (rssi == 4)
      {
        tft.fillRect(xOffset + 102, yBar1, 4, 2, TFT_GREEN);
        tft.fillRect(xOffset + 107, yBar2, 4, 4, TFT_GREEN);
        tft.fillRect(xOffset + 112, yBar3, 4, 8, TFT_GREEN);
        tft.fillRect(xOffset + 117, yBar4, 4, 12, TFT_GREEN);
        tft.drawRect(xOffset + 122, yBar5, 4, 16, TFT_GREEN);
      }
      else if (rssi == 3)
      {
        tft.fillRect(xOffset + 102, yBar1, 4, 2, TFT_YELLOW);
        tft.fillRect(xOffset + 107, yBar2, 4, 4, TFT_YELLOW);
        tft.fillRect(xOffset + 112, yBar3, 4, 8, TFT_YELLOW);
        tft.drawRect(xOffset + 117, yBar4, 2, 12, TFT_YELLOW);
        tft.drawRect(xOffset + 122, yBar5, 4, 16, TFT_YELLOW);
      }
      else if (rssi == 2)
      {
        tft.fillRect(xOffset + 102, yBar1, 4, 2, TFT_YELLOW);
        tft.fillRect(xOffset + 107, yBar2, 4, 4, TFT_YELLOW);
        tft.drawRect(xOffset + 112, yBar3, 4, 8, TFT_YELLOW);
        tft.drawRect(xOffset + 117, yBar4, 2, 12, TFT_YELLOW);
        tft.drawRect(xOffset + 122, yBar5, 4, 16, TFT_YELLOW);
      }
      else if (rssi == 1)
      {
        tft.fillRect(xOffset + 102, yBar1, 4, 2, TFT_YELLOW);
        tft.drawRect(xOffset + 107, yBar2, 4, 4, TFT_YELLOW);
        tft.drawRect(xOffset + 112, yBar3, 4, 8, TFT_YELLOW);
        tft.drawRect(xOffset + 117, yBar4, 4, 12, TFT_YELLOW);
        tft.drawRect(xOffset + 122, yBar5, 4, 16, TFT_YELLOW);
      }
      else
      {
        tft.drawRect(xOffset + 102, yBar1, 4, 2, TFT_RED);
        tft.drawRect(xOffset + 107, yBar2, 4, 4, TFT_RED);
        tft.drawRect(xOffset + 112, yBar3, 4, 8, TFT_RED);
        tft.drawRect(xOffset + 117, yBar4, 4, 12, TFT_RED);
        tft.drawRect(xOffset + 122, yBar5, 4, 16, TFT_RED);
      }
      lastRSSI = rssi;
    }

#ifdef useTouch
    tft.fillRoundRect(valueUpRect[0], valueUpRect[1], valueUpRect[2], valueUpRect[3], 4, TFT_GREEN);
    tft.drawLine(valueUpRect[0] + plusMinusHorizontalLineMarginLeft, valueUpRect[1] + plusMinusHorizontalLineMarginTop, valueUpRect[0] + plusMinusHorizontalLineMarginLeft + plusMinusHorizontalLineLength, valueUpRect[1] + plusMinusHorizontalLineMarginTop, TFT_BLACK);
    tft.drawLine(valueUpRect[0] + plusMinusVerticalLineMarginLeft, valueUpRect[1] + plusMinusVerticalLineMarginTop, valueUpRect[0] + plusMinusVerticalLineMarginLeft, valueUpRect[1] + plusMinusVerticalLineMarginTop + plusMinusVerticalLineLength, TFT_BLACK);

    tft.fillRoundRect(valueDownRect[0], valueDownRect[1], valueDownRect[2], valueDownRect[3], 4, TFT_GREEN);
    tft.drawLine(valueDownRect[0] + plusMinusHorizontalLineMarginLeft, valueDownRect[1] + plusMinusHorizontalLineMarginTop, valueDownRect[0] + plusMinusHorizontalLineMarginLeft + plusMinusHorizontalLineLength, valueDownRect[1] + plusMinusHorizontalLineMarginTop, TFT_BLACK);
#endif

#if defined(useStandbyButton) || defined(useShutdownButton)
    tft.fillRoundRect(shutdownRect[0], shutdownRect[1], shutdownRect[2], shutdownRect[3], getRelativeX(4), TFT_RED);
    tft.drawCircle(shutdownRect[0] + getRelativeX(shutdownWidthAbsolute / 2), shutdownRect[1] + getRelativeY(shutdownHeightAbsolute / 2), getRelativeX(shutdownWidthAbsolute * 0.375), TFT_WHITE);
    tft.drawLine(shutdownRect[0] + getRelativeX(shutdownWidthAbsolute / 2), shutdownRect[1] + getRelativeY(shutdownHeightAbsolute / 4), shutdownRect[0] + getRelativeX(shutdownWidthAbsolute / 2), shutdownRect[1] + getRelativeY(shutdownHeightAbsolute * 3 / 4), TFT_WHITE);
#endif
  }
#ifdef useShutdownButton
  else if (screen == SCREEN_CONFIRMSHUTDOWN)
  {
    printText(getRelativeX(44), getRelativeY(50), getRelativeX(200), 0, "Please confirm shutdown", textSizeOffset + 1, myFont, false, TFT_WHITE);
    tft.fillRoundRect(confirmShutdownYesRect[0], confirmShutdownYesRect[1], confirmShutdownYesRect[2], confirmShutdownYesRect[3], 4, TFT_RED);
    printText(confirmShutdownYesRect[0] + getRelativeX(12), confirmShutdownYesRect[1] + getRelativeY(22), getRelativeX(200), 0, "Yes", textSizeOffset + 1, myFont, false, TFT_WHITE);
    tft.fillRoundRect(confirmShutdownNoRect[0], confirmShutdownNoRect[1], confirmShutdownNoRect[2], confirmShutdownNoRect[3], 4, TFT_GREEN);
    printText(confirmShutdownNoRect[0] + getRelativeX(18), confirmShutdownNoRect[1] + getRelativeY(22), getRelativeX(200), 0, "No", textSizeOffset + 1, myFont, false, TFT_WHITE);
  }
  else if (screen == SCREEN_COUNTDOWN)
  {
    int intSecondsSinceShutdown = (int)((unsigned long)(millis() - startCountdown) / 1000 + 0.5);
    tft.fillScreen(TFT_BLACK);
    sprintf(buffer, "%d", SHUTDOWNCOUNTDOWN - intSecondsSinceShutdown);
    printText(getRelativeX(115), getRelativeY(80), getRelativeX(200), 0, buffer, textSizeOffset + 4, myFont, false, TFT_WHITE);

    if ((unsigned long)(millis() - startCountdown) > SHUTDOWNCOUNTDOWN * 1000 + 15000)
    {
      Log.printf("hm, still alive? Better show mainscreen again\r\n");
      screen = SCREEN_NORMALMODE;
      tft.fillRect(0, 0, 320, 240, TFT_BLACK);
      draw_screen();
    }
  }
#endif
}
#endif // useTFT