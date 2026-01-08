#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <TFT_eSPI.h>
#include <SPI.h>

Adafruit_AMG88xx amg;
TFT_eSPI tft = TFT_eSPI(); 


float pixels[64];
int pixelSize = 10;
uint16_t backfroundColor = tft.color565(157, 255, 175);

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

  for (int i = 0; i < 64; i++) {
    Serial.print(pixels[i]);
    Serial.print(" , ");
    if ((i + 1) % 8 == 0) Serial.println();
  }

  // String text = "Max: ";
  // text += String(maxValue(pixels), 1); // رقم عشري واحد
  // text += "  , ";
  // text += "Min: ";
  // text += String(minValue(pixels), 1);

  // writeString(text, 10, 10);
  // drawThermalMap(pixels);
  // 30
  drawThermalMapSmooth(pixels, 240 , 320);
  // delay(500);
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
    float ratio = (temp - tMin) / (tMax - tMin);
    ratio = constrain(ratio, 0, 1);

    uint8_t r, g, b;

    if (ratio <= 0.25) { r=0; g=4*ratio*255; b=255; }               // أزرق → سماوي
    else if (ratio <= 0.5) { r=0; g=255; b=255 - 4*(ratio-0.25)*255;} // سماوي → أخضر
    else if (ratio <= 0.75) { r=4*(ratio-0.5)*255; g=255; b=0;}     // أخضر → أصفر
    else { r=255; g=255 - 4*(ratio-0.75)*255; b=0;}                 // أصفر → أحمر

    return tft.color565(r,g,b);
}

void drawThermalMapSmooth(float values[64], int mapWidth, int mapHeight) {
  int smallSize = 8; // المصفوفة الأصلية 8×8
  float tMin = minValue(values);
  float tMax = maxValue(values);
  for (int y_big = 0; y_big < mapHeight; y_big++) {
    for (int x_big = 0; x_big < mapWidth; x_big++) {
      // نحدد موقع البكسل بالنسبة للصورة الصغيرة
      float x_small = x_big * (float)(smallSize-1) / (mapWidth-1);
      float y_small = y_big * (float)(smallSize-1) / (mapHeight-1);

      int x0 = floor(x_small);
      int x1 = min(x0 + 1, smallSize-1);
      int y0 = floor(y_small);
      int y1 = min(y0 + 1, smallSize-1);

      float dx = x_small - x0;
      float dy = y_small - y0;

      // interpolationdy
      float temp = (1-dx)*(1-dy)*values[y0*smallSize + x0] +
                   dx*(1-dy)*values[y0*smallSize + x1] +
                   (1-dx)*dy*values[y1*smallSize + x0] +
                   dx*dy*values[y1*smallSize + x1];

      uint16_t color = thermalColor(temp, tMin, tMax);
      tft.drawPixel(x_big, y_big, color); // رسم كل بكسل

    }
  }
}




