#define ENABLE_GxEPD2_GFX 0

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Wire.h>
#include <ds3231.h>
// Include libraries:
#include <GxEPD2_BW.h>  // Include GxEPD2 library for black and white displays
#include <Adafruit_GFX.h>  // Include Adafruit_GFX library

#define MAX_DISPLAY_BUFFER_SIZE 640
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
#if defined(__AVR)
GxEPD2_BW<GxEPD2_750, MAX_HEIGHT(GxEPD2_750)> display(GxEPD2_750(/*CS=10*/ SS, /*DC=*/ 8, /*RST=*/ 9, /*BUSY=*/ 7));
//GxEPD2_BW<GxEPD2_750_T7, MAX_HEIGHT(GxEPD2_750_T7)> display(GxEPD2_750_T7(/*CS=10*/ SS, /*DC=*/ 8, /*RST=*/ 9, /*BUSY=*/ 7)); // GDEW075T7 800x480
#endif
#if defined(ESP32)
//GxEPD2_BW<GxEPD2_750, GxEPD2_750::HEIGHT> display(GxEPD2_750(/*CS=5*/ 15, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // inland esp32dev
GxEPD2_BW<GxEPD2_750, GxEPD2_750::HEIGHT> display(GxEPD2_750(/*CS=5*/ 15, /*DC=*/ 12, /*RST=*/ 27, /*BUSY=*/ 4)); // adafruit huzzah32
//GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> display(GxEPD2_750_T7(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // GDEW075T7 800x480
#endif
//#include <Fonts/FreeMono9pt7b.h>
//#include <Fonts/FreeMono12pt7b.h>
//#include <Fonts/FreeMono18pt7b.h>
//#include <Fonts/FreeMono24pt7b.h>
//#include <Fonts/FreeMonoBold9pt7b.h>
//#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
//#include <Fonts/FreeMonoBold24pt7b.h>
// SD chip select pin
const uint8_t chipSelect = 13;

struct ts t; 


// Maximum line length plus space for zero byte.
const size_t LINE_DIM = 50;
int offset = 2;
char line[LINE_DIM];
String city;
char fajr[6];
char duhr[6];
char asr[6];
char maghrib[6];
char isha[6];
//------------------------------------------------------------------------------
//// store error strings in flash memory
//#define error(s) sd.errorHalt_P(PSTR(s))
//------------------------------------------------------------------------------
void setup(void) {
//    
  display.init(115200);  // Initiate the display
  
  display.setRotation(0);  // Set orientation. Goes from 0, 1, 2 or 3
  
  display.setTextWrap(false);  // By default, long lines of text are set to automatically “wrap” back to the leftmost column.
                               // To override this behavior (so text will run off the right side of the display - useful for
                               // scrolling marquee effects), use setTextWrap(false). The normal wrapping behavior is restored
                               // with setTextWrap(true).
  Serial.begin(115200);
      if(!SD.begin()){
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }
  Wire.begin();
  DS3231_init(DS3231_CONTROL_INTCN);

  DS3231_get(&t);
  Serial.print(F("Date : "));
  Serial.print(t.mday);
  Serial.print(F("/"));
  Serial.print(t.mon);
  Serial.print(F("/"));
  Serial.print(t.year);
  Serial.print(F("\t Hour : "));
  Serial.print(t.hour);
  Serial.print(F(":"));
  Serial.print(t.min);
  Serial.print(F("."));
  Serial.println(t.sec);
  
  size_t n;
  city = "atlanta";
  while (!Serial) {}  // Wait for Leonardo
  String filePath = getFilePath(t.year, t.mon, city);
  char fp[34];
  filePath.toCharArray(fp, 34);
  Serial.println(fp);

  readFile(SD, fp, t.mday);
  
}
void readFile(fs::FS &fs, const char * path, int day){
  
    int ln = 1;
   Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
//    while(day + offset > ln) {
    while(file.available()){
      char buffer[64];
      int l = file.readBytesUntil('\n', buffer, sizeof(buffer));
      buffer[l] = 0;
//      Serial.println(buffer);
            ln++;
      if (day + offset == ln) {
        Serial.print(": ");
        Serial.print(buffer);
        getValue(buffer, 0x09, 1, fajr);
        Serial.print("fajr:\t");
        Serial.write(fajr);
        getValue(buffer, 0x09, 4, duhr);
        Serial.print("duhr:\t");
        Serial.print(duhr);
        getValue(buffer, 0x09, 5, asr);
        Serial.print("asr:\t");
        Serial.print(asr);
        getValue(buffer, 0x09, 6, maghrib);
        Serial.print("maghrib:\t");
        Serial.print(maghrib);
        getValue(buffer, 0x09, 7, isha);
        Serial.print("isha:\t");
        Serial.print(isha);
      }
//      int currentChar = file.read();
//      // if newline
//      if (currentChar == 10) {
//        ln++
//      }
    }
//    while(file.available()){
      
      //      line = file.read();
              // Print line number.

//        
//        if (line[n - 1] != '\n') {
//          // Line is too long or last line is missing nl.
//          Serial.println(F(" <-- missing nl"));
//        }  
//    }
    file.close();

  Serial.println(F("\nDone")); 
}
void getValue(String data, char separator, int index, char *memLoc)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  if (found>index) {
    Serial.println(data.substring(strIndex[0], strIndex[1]));
    data.substring(strIndex[0], strIndex[1]).toCharArray(memLoc,  6);
  } 
}

String getFilePath(int year, int month, String city) {
  //"prayer_times/2020/09/atlanta.csv"
  String fp = F("/prayer_times/");
  fp = fp + year;
  fp = fp + "/";
  if (month < 10) {
    fp = fp + "0";  
  }
  fp = fp + month;
  fp = fp + "/";
  fp = fp + city;
  fp = fp + ".csv";
  Serial.println(fp);
  return fp;
}

void loop(void) {
  display.setFullWindow();
  
  display.fillScreen(GxEPD_WHITE);  // Clear previous graphics to start over to print new things.
  display.firstPage();  // Tell the graphics class to use paged drawing mode

    do
  {
    // Put everything you want to print in this screen:
    
    
    display.fillScreen(GxEPD_WHITE);  // Clear previous graphics to start over to print new things.
    // Print text - "Hello World!":
    display.setTextColor(GxEPD_BLACK);  // Set color for text
    display.setFont(&FreeMonoBold18pt7b);  // Set font
    int timex = 520;
    int titlex = 390;
    display.setCursor(titlex, 80);  // Set the position to start printing text (x,y)
    display.println("fajr: ");  // Print some text
    display.setCursor(timex, 80);  // Set the position to start printing text (x,y)
    display.println(fajr);  // Print some text
    display.setCursor(titlex, 150);  // Set the position to start printing text (x,y)
    display.println("duhr: ");  // Print some text
    display.setCursor(timex, 150);  // Set the position to start printing text (x,y)
    display.println(duhr);  // Print some text
    display.setCursor(titlex + 26, 220);  // Set the position to start printing text (x,y)
    display.println("asr: ");  // Print some text
    display.setCursor(timex, 220);  // Set the position to start printing text (x,y)
    display.println(asr);  // Print some text
    display.setCursor(titlex - 64, 290);  // Set the position to start printing text (x,y)
    display.println("maghrib: ");  // Print some text
    display.setCursor(timex, 290);  // Set the position to start printing text (x,y)
    display.println(maghrib);  // Print some text
    display.setCursor(titlex, 360);  // Set the position to start printing text (x,y)
    display.println("isha: ");  // Print some text
    display.setCursor(timex, 360);  // Set the position to start printing text (x,y)
    display.println(isha);  // Print some text
  }
  while (display.nextPage());
  
  display.hibernate(); 
  for(;;); // Don't proceed, loop forever
  }
