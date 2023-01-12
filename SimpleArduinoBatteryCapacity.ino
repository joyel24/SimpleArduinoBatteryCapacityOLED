#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include "OneButton.h"  //http://www.mathertel.de/Arduino/OneButtonLibrary.aspx

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

#define NUM_OF_AVERAGE_SAMPLES 500      //Number of values picked for average adc calculation
#define MEASURED_VCC_REF 4.95           //Measured voltage accross ground and Vcc pin of your arduino (it can depends on its supply)
#define MEASURED_VCC_WITH_LOAD 4.95     //
#define SHUNT_RESISTOR_OHM 1.00         //Value of Shunt Resistor (Ohm) to measure intensity of discharge
#define PIN_RELAY_OR_MOSFET 7           //PIN used to drive the relay to start or stop the discharge 
#define PIN_BUTTON 5                    //Pin of the button to start/stop discharge
#define STOP_DISCHARGE_VOLTAGE 0.00     //Stop discharge protection !!!!!!!!!!!!To Improove!!!!!!!!!!!
#define VOLTAGE_PIN A3                  //Battery voltage pin input
#define INTENSITY_PIN A0                //Pin connected to Shunt Resistor
#define PIN_BUTTON_FOR_MENU 16          //Pin connected to Menu/Function button

float mAsEEPROMstored;                  //Variable synced to EEPROM mas data (mAs=mah*3600)
float mAs = 0;                          //milli-Ampere-seconds : mAh=mAs/3600
uint32_t secondsElapsed;

bool menuActive = false;
bool exitMenu = false;
OneButton button(PIN_BUTTON_FOR_MENU, true);  //button definition for OneButton Library http://www.mathertel.de/Arduino/OneButtonLibrary.aspx
int selectedMenu;                             //Variable corresponding to the active item in menu
uint32_t antiMenuLoopbackMilliSeconds;        //Anti Loopback after menu exited

void setup() {
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_RELAY_OR_MOSFET, OUTPUT);
  digitalWrite(PIN_RELAY_OR_MOSFET, LOW);     //Ensure discharge relay is off
  EEPROM.get(0, mAsEEPROMstored);             //get float data stored from address 0 of EEPROM (Address 0 to 3 as float is 4bytes long)
  
  pinMode(PIN_BUTTON_FOR_MENU, INPUT_PULLUP); //
  button.attachLongPressStart(menulongPress); //Theses lines are needed for the OneButton Library
  button.attachClick(oneClick);               //Run function name beetween () when button action is detected

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

void menulongPress(){
    switch (selectedMenu){
        case 1:
            mAsEEPROMstored = 0;
            EEPROM.put(0, mAsEEPROMstored);
            display.clearDisplay();
            display.setTextSize(1);
            display.setCursor(0, 0);
            display.println("EEPROM data set to 0");
            display.display();
            delay(1500);
          break;
        case 2:
          
          break;
        case 3:
          exitMenu = true;
          menuActive = false;
          antiMenuLoopbackMilliSeconds = millis();
          break;
    }
  //}
}

void oneClick(){
  if (menuActive == false && (millis() - antiMenuLoopbackMilliSeconds > 2000) ){
    Menu();
  }
  else if (menuActive == true){
    switch (digitalRead(PIN_BUTTON)){
        case LOW:
          selectedMenu+=1;
          break;
        case HIGH:
          selectedMenu-=1;
          break;
      }
  }
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
  float voltage = getVoltage(VOLTAGE_PIN);
  float intensity = getIntensity(INTENSITY_PIN);
  float power = (voltage * intensity);
  return power;
}

void displayRefresh(uint32_t SecondsElapsed){   //Function to display instant voltage, intensity and power on 1st line & other function parameters
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(getVoltage(VOLTAGE_PIN), 3); display.print("V ");     //Display Voltage
  display.print(1000* (getIntensity(INTENSITY_PIN)), 0); display.print("mA ");   //Display Intensity
  display.print(getPower()); display.print("W ");         //Display Power
  display.setCursor(0, 9);
  display.print(SecondsElapsed);; display.print("s ");    //Timer
  display.print(mAs/3600); display.println("mAh ");
  EEPROM.get(0, mAsEEPROMstored);
  display.print(mAsEEPROMstored / 3600);display.println("mAh ");
  display.display();
}

void Menu(){
  menuActive = true;
  exitMenu = false;
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("MENU");
  display.display();
  delay(1000);
  while (exitMenu == false){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    switch (selectedMenu){
      case 1:
        display.println(">Reset EEPROM");
        display.println("2");
        display.println("Exit Menu");
        break;
      case 2:
        display.println("Reset EEPROM");
        display.println(">2");
        display.println("Exit Menu");
        break;
      case 3:
        display.println("Reset EEPROM");
        display.println("2");
        display.println(">Exit Menu");
        break;
      default:
        if (selectedMenu < 1){
          selectedMenu = 3;
          break;
        }
        else {selectedMenu = 1;}
        break;
    }
    button.tick();
    display.display();
  }
}

void loop() {
  digitalWrite(PIN_RELAY_OR_MOSFET, LOW);
  int millisecFormAsInterval = 2000;
  displayRefresh(secondsElapsed);
  button.tick();
  if (digitalRead(PIN_BUTTON) == HIGH){
    uint32_t resetTime = millis();
    uint32_t startMillisFormAs = millis();
    mAs = 0;                                //Reset mA of actual counter
    while (digitalRead(PIN_BUTTON) == HIGH && getVoltage(VOLTAGE_PIN) >= STOP_DISCHARGE_VOLTAGE){
      digitalWrite(PIN_RELAY_OR_MOSFET, HIGH);
      uint32_t TimeElapsed = millis() - resetTime;
      if ( (millis()-startMillisFormAs) >= millisecFormAsInterval){
        mAs += (getIntensity(INTENSITY_PIN)*1000) * (millisecFormAsInterval/1000);
        mAsEEPROMstored += (getIntensity(INTENSITY_PIN)*1000) * (millisecFormAsInterval/1000);
        if (EEPROM.read(0) != mAsEEPROMstored){
          EEPROM.put(0, mAsEEPROMstored);
          }
        startMillisFormAs = millis();
      }
      displayRefresh(TimeElapsed/1000);
      secondsElapsed = TimeElapsed/1000;
    }
  }

}
