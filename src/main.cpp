#include <SPI.h>
#include <SD.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold12pt7b.h>

// SPI Bus
#define SPI_SCK 7  // D8
#define SPI_MISO 8 // D9
#define SPI_MOSI 9 // D10

// EPD Pins
#define EPD_CS 3   // D2
#define EPD_DC 4   // D3
#define EPD_RST 5  // D4
#define EPD_BUSY 6 // D5

// SD Pin
#define SD_CS 44 // D7

// Button Pins
#define BTN_ADC1 1   // D0 - Back, Confirm, Left, Right
#define BTN_ADC2 2   // D1 - Vol Up, Vol Down
#define BTN_POWER 43 // D6 - Power Button

// ADC Schwellenwerte (vom Xteink X4)
#define THRESH_RIGHT_MAX 200
#define THRESH_LEFT_MAX 2000
#define THRESH_CONFIRM_MAX 3100
#define THRESH_BACK_MAX 3999
#define THRESH_VOLDOWN_MAX 200
#define THRESH_VOLUP_MAX 3999

GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> display(
    GxEPD2_290_C90c(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

String lastButton = "";

void showOnDisplay(String text)
{
    display.setFullWindow();
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(10, 40);
        display.print("Button:");
        display.setCursor(10, 80);
        display.print(text);
    } while (display.nextPage());
}

String readButton()
{
    // Power Button (direkt digital)
    if (digitalRead(BTN_POWER) == LOW)
        return "POWER";

    // Ladder 1: Back, Confirm, Left, Right
    int adc1 = analogRead(BTN_ADC1);
    Serial.println("ADC1: " + String(adc1));
    if (adc1 < THRESH_RIGHT_MAX)
        return "RIGHT";
    if (adc1 < THRESH_LEFT_MAX)
        return "LEFT";
    if (adc1 < THRESH_CONFIRM_MAX)
        return "CONFIRM";
    if (adc1 < THRESH_BACK_MAX)
        return "BACK";

    // Ladder 2: Vol Up, Vol Down
    int adc2 = analogRead(BTN_ADC2);
    Serial.println("ADC2: " + String(adc1));
    if (adc2 < THRESH_VOLDOWN_MAX)
        return "VOL DOWN";
    if (adc2 < THRESH_VOLUP_MAX)
        return "VOL UP";

    return "";
}

void setup()
{
    Serial.begin(115200);

    pinMode(BTN_POWER, INPUT_PULLUP);

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SD_CS);

    if (!SD.begin(SD_CS))
    {
        Serial.println("SD init failed!");
    }
    else
    {
        Serial.println("SD init OK!");
    }

    display.init(115200);
    display.setRotation(1);

    showOnDisplay("Press a button!");
    Serial.println("Ready.");
}

void loop()
{
    String btn = readButton();

    if (btn != "" && btn != lastButton)
    {
        Serial.println("Button: " + btn);
        showOnDisplay(btn);
        lastButton = btn;
        delay(300);
    }

    if (btn == "")
    {
        lastButton = "";
    }

    delay(50); // Polling alle 50ms wie beim X4
}