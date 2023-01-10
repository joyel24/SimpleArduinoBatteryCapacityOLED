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

#define NUM_OF_AVERAGE_SAMPLES 500     //Number of values picked for average adc calculation
#define MEASURED_VCC_REF 4.75           //Measured voltage accross ground and Vcc pin of your arduino (it can depends on its supply)
#define MEASURED_VCC_WITH_LOAD 4.67     //
#define SHUNT_RESISTOR_OHM 1.00         //Value of Shunt Resistor (Ohm) to measure intensity of discharge
#define PIN_RELAY_OR_MOSFET 7           //PIN used to start or stop the discharge 
#define PIN_RELAY_DEFAULT 0             //Not Used yet
#define PIN_BUTTON 5                    //Pin of the button to start/stop discharge
#define STOP_DISCHARGE_VOLTAGE 3.2     //

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_RELAY_OR_MOSFET, OUTPUT);
  digitalWrite(PIN_RELAY_OR_MOSFET, LOW);

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
  if (digitalRead(PIN_RELAY_OR_MOSFET) == LOW){
  return value / 1023 * MEASURED_VCC_REF;
  }
  else {return value / 1023 * MEASURED_VCC_WITH_LOAD;}
}

float getIntensity(int pin){
  float voltage = getVoltage(pin);
  return voltage / SHUNT_RESISTOR_OHM;
}

float getPower(){
  float voltage = getVoltage(A0);
  float intensity = getIntensity(A1);
  float power = (voltage * intensity);
  return power;
}

void DisplayRefresh(uint32_t SecondsElapsed, float mAh){   //Function to display instant voltage, intensity and power on 1st line & other function parameters
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(getVoltage(A0)); display.print("V ");     //Display Voltage
  display.print(getIntensity(A1)); display.print("A ");   //Display Intensity
  display.print(getPower()); display.print("W ");         //Display Power
  display.setCursor(0, 9);
  display.print(SecondsElapsed);; display.print("s ");    //Timer
  display.print(mAh);; display.print("mAh ");
  display.display();
}



void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(PIN_RELAY_OR_MOSFET, LOW);
  float mAs = 0;
  float mAh = mAs/3600;
  int millisecFormAsInterval = 5000;
  DisplayRefresh(0, 0);
  if (digitalRead(PIN_BUTTON) == HIGH){
    uint32_t resetTime = millis();
    uint32_t startMillisFormAs = millis();
    while (digitalRead(PIN_BUTTON) == HIGH && getVoltage(A0) >= STOP_DISCHARGE_VOLTAGE){
      digitalWrite(PIN_RELAY_OR_MOSFET, HIGH);
      uint32_t TimeElapsed = millis() - resetTime;
      if ( (millis()-startMillisFormAs) >= millisecFormAsInterval){
        mAs += (getIntensity(A1)*1000) * (millisecFormAsInterval/1000);
        startMillisFormAs = millis();
      }
      mAh = mAs/3600;
      //Serial.print(mAs);Serial.print("mas ");Serial.print(mAh);Serial.println("mah ");
      DisplayRefresh(TimeElapsed/1000, mAh);
    }
  }

}
