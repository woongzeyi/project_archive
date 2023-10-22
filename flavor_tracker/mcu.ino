#include<Wire.h>

/* ADC Module */
#include<ADS1115_WE.h> 
#define I2C_ADDRESS 0b1001000
ADS1115_WE adc = ADS1115_WE(&Wire, I2C_ADDRESS);

/* Display */
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);
  Wire.begin();

  if(!adc.init()) {
    Serial.println("ADS1115 not connected!");
    for(;;);
  }
  adc.setVoltageRange_mV(ADS1115_RANGE_0256);
  adc.setMeasureMode(ADS1115_CONTINUOUS);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
}

int current_column = 0;

void loop() {

  if (current_column >= 120) {
    display.clearDisplay();
    current_column = 0;
  }

  float voltage = 0.0;

  Serial.print("0: ");
  voltage = readChannel(ADS1115_COMP_0_1);
  Serial.print(voltage);
  display.drawPixel(current_column, 32-voltage*.16, SSD1306_WHITE);

  Serial.print(",   1: ");
  voltage = readChannel(ADS1115_COMP_0_3);
  Serial.print(voltage);
  display.drawPixel(current_column, 32-voltage*.16, SSD1306_WHITE);
  
  Serial.print(",   2: ");
  voltage = readChannel(ADS1115_COMP_1_3);
  Serial.print(voltage);
  display.drawPixel(current_column, 32-voltage*.16, SSD1306_WHITE);

  Serial.print(",   3: ");
  voltage = readChannel(ADS1115_COMP_2_3);
  Serial.println(voltage);
  display.drawPixel(current_column, 32-voltage*.16, SSD1306_WHITE);

  display.display();
  current_column++;

  delay(500);
}

float readChannel(ADS1115_MUX channel) {
  float voltage = 0.0;
  adc.setCompareChannels(channel);
  voltage = adc.getResult_mV(); 
  return voltage;
}