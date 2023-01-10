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

#define NUM_OF_AVERAGE_SAMPLES 2000    //Number of values picked for average adc calculation
#define MEASURED_VCC_REF 4.75          //Measured voltage accross ground and Vcc pin of your arduino (it can depends on its supply)
#define SHUNT_RESISTOR_OHM 1.00        //Value of Shunt Resistor (Ohm) to measure intensity of discharge

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();

  // Display Welcome Message during 2sec
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Welcome");
  display.display();
  delay(2000);
}

//Function to get adc value from a desired pin with averaging samples defined by NUM_OF_AVERAGE_SAMPLES
int getadcValue(int pin){
  uint32_t adcValuesTotal = 0;
  for (int n = 0; n < NUM_OF_AVERAGE_SAMPLES; n++){ //Averaging values for n sample
    adcValuesTotal += analogRead(pin);
  }
  return adcValuesTotal / NUM_OF_AVERAGE_SAMPLES;;
}

float getVoltage(int pin){
  float value = getadcValue(pin);
  return value / 1023 * MEASURED_VCC_REF;
}

float getIntesity(int pin){
  float voltage = getVoltage(pin);
  return voltage / SHUNT_RESISTOR_OHM;
}

float getPower(){
  float voltage = getVoltage(A0);
  float intensity = getIntesity(A1);
  float power = (voltage * intensity);
  return power;
}



void loop() {
  // put your main code here, to run repeatedly:
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  //Print measured voltage from cell connected
  display.setCursor(0, 0);
  display.print(getVoltage(A0)); display.print("V ");
  display.print(getIntesity(A1)); display.print("A ");
  display.print(getPower()); display.print("W ");
  display.setCursor(0, 9);
  display.print(millis()/1000);
  display.display();
  //delay(500);

}
