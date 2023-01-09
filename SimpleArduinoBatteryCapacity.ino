#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUM_OF_AVERAGE_ATTEMPTS 200     //Number of values picked for average adc calculation
#define MEASURED_VCC_REF 4.6            //Measured voltage accross ground and Vcc pin of your arduino (it can depends on its supply)

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();

  // Display Welcome Message during 3sec
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Welcome");
  display.display();
  delay(3000);
}

int getadcValue(){
  uint32_t adcValuesTotal = 0;
  int averageValue = 0;
  for (int n = 0; n < NUM_OF_AVERAGE_ATTEMPTS; n++){ //Averaging values for n times attempts
    adcValuesTotal += analogRead(A0);
  }
  return adcValuesTotal / NUM_OF_AVERAGE_ATTEMPTS;
}

float getADCValToVolts(){
  float value = getadcValue();
  return value / 1024 * MEASURED_VCC_REF;
}

void loop() {
  // put your main code here, to run repeatedly:
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  //Print Voltage Measured from cell connected to ADC0
  display.setCursor(0, 0);
  display.print(getADCValToVolts()); display.print("V");
  display.display();
  //delay(500);

}
