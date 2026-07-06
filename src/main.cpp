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

// ADC Ranges direkt aus CrossPoint Community SDK
#define ADC_NO_BUTTON 3900
#define THRESH_RIGHT 750
#define THRESH_LEFT 2090
#define THRESH_CONFIRM 3100
#define THRESH_BACK ADC_NO_BUTTON
#define THRESH_DOWN 1120
#define THRESH_UP ADC_NO_BUTTON

#define DEBOUNCE_MS 50

GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> display(
    GxEPD2_290_C90c(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

String lastButton = "";

void showOnDisplay(String line1, String line2 = "")
{
    display.setFullWindow();
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(10, 40);
        display.print(line1);
        if (line2 != "")
        {
            display.setCursor(10, 80);
            display.print(line2);
        }
    } while (display.nextPage());
}

bool testSD()
{
    Serial.println("--- SD Test ---");

    File f = SD.open("/sdtest.txt", FILE_WRITE);
    if (!f)
    {
        Serial.println("SD Schreiben: FEHLER");
        return false;
    }
    f.println("SD Test OK");
    f.close();
    Serial.println("SD Schreiben: OK");

    f = SD.open("/sdtest.txt");
    if (!f)
    {
        Serial.println("SD Lesen: FEHLER");
        return false;
    }
    String content = "";
    while (f.available())
        content += (char)f.read();
    f.close();
    content.trim();
    Serial.println("SD Lesen: " + content);
    Serial.println("--- SD Test Ende ---");
    return true;
}

String readButton()
{
    if (digitalRead(BTN_POWER) == LOW)
        return "POWER";

    int adc1 = analogRead(BTN_ADC1);
    if (adc1 < THRESH_RIGHT)
        return "RIGHT";
    if (adc1 < THRESH_LEFT)
        return "LEFT";
    if (adc1 < THRESH_CONFIRM)
        return "CONFIRM";
    if (adc1 < THRESH_BACK)
        return "BACK";

    int adc2 = analogRead(BTN_ADC2);
    if (adc2 < THRESH_DOWN)
        return "VOL DOWN";
    if (adc2 < THRESH_UP)
        return "VOL UP";

    return "";
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    pinMode(BTN_POWER, INPUT_PULLUP);
    analogSetAttenuation(ADC_11db);

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SD_CS);

    // Display ZUERST initialisieren
    display.init(115200);
    display.setRotation(1);

    // Dann SD Test
    if (!SD.begin(SD_CS))
    {
        Serial.println("SD init: FEHLER");
        showOnDisplay("SD init", "FEHLER!");
        delay(2000);
    }
    else
    {
        Serial.println("SD init: OK");
        bool sdOk = testSD();
        showOnDisplay("SD init: OK", sdOk ? "SD Test: OK" : "SD Test: FAIL");
        delay(2000);
    }

    showOnDisplay("Press a button!");
    Serial.println("Ready.");
}

void loop()
{
    String btn = readButton();

    if (btn != "" && btn != lastButton)
    {
        int adc1 = analogRead(BTN_ADC1);
        int adc2 = analogRead(BTN_ADC2);
        Serial.println("Button: " + btn + " (ADC1=" + adc1 + " ADC2=" + adc2 + ")");
        showOnDisplay("Button:", btn);
        lastButton = btn;
        delay(DEBOUNCE_MS);
    }

    if (btn == "")
        lastButton = "";

    delay(50);
}