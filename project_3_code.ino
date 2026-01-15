#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <TFT_eSPI.h>
#include <SPI.h>
// #define targetTemp 15
#define targetTemp  15
#define pixelSize 10
#define blackColor 0x0000
#define whiteColor 0xFFFF
#define verticalTextPadding 10
#define horizentalTextPadding 10
#define sensorPixelsSize 8
#define mapTopPadding 33
#define screenWidth 240
#define screenHight 320



Adafruit_AMG88xx amg;
TFT_eSPI tft = TFT_eSPI(); 


float pixels[64];
uint16_t backfroundColor = tft.color565(157, 255, 175);
float lastPersentage = -1;

void setup() {
  Serial.begin(115200);
  Wire.begin(18, 27);   // SDA, SCL
  tft.init();
  tft.setRotation(0); 
  //175, 225, 175
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  if (!amg.begin()) {
    Serial.println("AMG8833 not detected");
    while (1);
  }

  Serial.println("AMG8833 ready!");
  // drawHeaders();
}

void loop() {
  amg.readPixels(pixels);

  int targetPixelsCount = 0;
  for(int i = 0 ; i < 64 ; i++){
    if(pixels[i] < targetTemp) {
      targetPixelsCount++;
    }
  }
  float persentage = ((float) targetPixelsCount / 64.0) * 100.0;
  drawPersentage(persentage);
  drawThermalMapSmooth(pixels, screenWidth , (screenHight - mapTopPadding));
  // delay(30);
}

void drawPersentage(float persentage) {
  if(lastPersentage == persentage) return;
  int textHight = 16;
  Serial.println(textHight);
  tft.fillRect(0 , 0 , tft.width() , (textHight + verticalTextPadding) , blackColor);
  tft.fillRect(0 , (textHight + verticalTextPadding + 5) , tft.width() , 2 , tft.color565(137, 137, 137) );
  tft.setTextSize(2.7);
  tft.setTextColor(whiteColor); 
  tft.setTextDatum(TL_DATUM);
  tft.drawString("SUBZERO", horizentalTextPadding , verticalTextPadding , 1);
  tft.setTextDatum(TR_DATUM);
  tft.drawString(String(persentage,2) + "%", (tft.width() - horizentalTextPadding) ,  verticalTextPadding, 1);
  lastPersentage = persentage;
}

float maxValue(float values[64]) {
  float max = values[0] ;
  
  for(int i =0 ; i < 64 ; i++) {
    if(max < values[i])
      max = values[i];
  }
  return max;
}

float minValue(float values[64]) {
  float min = values[0] ;
  for(int i = 0 ; i < 64 ; i++) {
    if(min > values[i])
      min = values[i];
  }
  return min;
}

void writeString(String text,int x , int y) {
  tft.fillScreen(TFT_BLACK);
  tft.drawString(text, x, y , 2);
}


void drawThermalMap(float values[64]) {
    for(int row = 0 ; row < 8 ; row++){
      for(int col = 0 ; col < 8 ; col++){
        int index = row * 8 + col;
        float temp = pixels[index];
        uint16_t color = thermalColor(temp, minValue(values), maxValue(values));
        int x = col * pixelSize;
        int y = row * pixelSize;
        // tft.fillRect(x, y, pixelSize, pixelSize, color);  
        tft.drawPixel(x,y,color);
      }
    }
}


uint16_t thermalColor(float temp, float tMin, float tMax) {
    uint8_t r, g, b;
    if(temp < targetTemp) { r=48; g=92; b= 222; } 
    else { r = 0; g = 0; b = 0; }
    return tft.color565(r,g,b);
}



void drawThermalMapSmooth(float values[64], int mapWidth, int mapHeight) {
  float tMin = minValue(values);
  float tMax = maxValue(values);
  for (int y_big = 0; y_big < mapHeight; y_big++) {
    for (int x_big = 0; x_big < mapWidth; x_big++) {
      // نحدد موقع البكسل بالنسبة للصورة الصغيرة
      float x_small = x_big * (float)(sensorPixelsSize-1) / (mapWidth-1);
      float y_small = y_big * (float)(sensorPixelsSize-1) / (mapHeight-1);

      int x0 = floor(x_small);
      int x1 = min(x0 + 1, sensorPixelsSize-1);
      int y0 = floor(y_small);
      int y1 = min(y0 + 1, sensorPixelsSize-1);

      float dx = x_small - x0;
      float dy = y_small - y0;

      // interpolationdy
      float temp = (1-dx)*(1-dy)*values[y0*sensorPixelsSize + x0] +
                   dx*(1-dy)*values[y0*sensorPixelsSize + x1] +
                   (1-dx)*dy*values[y1*sensorPixelsSize + x0] +
                   dx*dy*values[y1*sensorPixelsSize + x1];

      uint16_t color = thermalColor(temp, tMin, tMax);
      tft.drawPixel(x_big, (y_big + mapTopPadding), color); // رسم كل بكسل
    }
  }
}




