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

#define NUM_OF_AVERAGE_SAMPLES 800      //Number of values picked for average adc calculation
#define MEASURED_VCC_REF 4.966           //Measured voltage accross ground and Vcc pin of your arduino (it can depends on its supply)
#define MEASURED_VCC_WITH_LOAD 4.966     //In case of powered by usb, you need to check VCC voltage to stay relative for ADC convertion
#define SHUNT_RESISTOR_OHM 1.00         //Value of Shunt Resistor (Ohm) to measure intensity of discharge
#define PIN_RELAY_OR_MOSFET 7           //PIN used to drive the relay to start or stop the discharge 
#define PIN_BUTTON 5                    //Pin of the button to start/stop discharge
#define STOP_DISCHARGE_VOLTAGE 0.85     //Stop discharge protection !!!!!!!!!!!!To Improove!!!!!!!!!!!
#define VOLTAGE_PIN A3                  //Battery voltage pin input
#define INTENSITY_PIN A0                //Pin connected to Shunt Resistor
#define PIN_BUTTON_FOR_MENU 16          //Pin connected to Menu/Function button
#define NUM_ATTEMPTS_INTERNAL_RES 5     //Number of attempts to calculate internal resistance
#define INTERNAL_RESITANCE_DURATION 100 //Duration of each internal resistance test
#define MEASURED_LOAD_RESISTANCE 17.175   //Measured load resistance (I recommend using OhmMeter in the battery socket with discharge ON with #define STOP_DISCHARGE_VOLTAGE 0.00)
#define ADC_INTENSITY_OFFSET_MINIMUM (13.20-4.8387)

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
    if (menuActive == false && (millis() - antiMenuLoopbackMilliSeconds > 2000) ){
      Menu();
    }
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
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.print(getInternalRes(MEASURED_LOAD_RESISTANCE, NUM_ATTEMPTS_INTERNAL_RES, INTERNAL_RESITANCE_DURATION), 0);
          display.print("mOhm");
          display.display();
          delay(5000);
          break;
        case 3:
          exitMenu = true;
          menuActive = false;
          antiMenuLoopbackMilliSeconds = millis();
          break;
    }
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
int getadcValue(int pin, int numSamples){
  uint32_t adcValuesTotal = 0;
  for (int n=0; n<numSamples; n++){ //Averaging values for n sample
    adcValuesTotal += analogRead(pin);
  }
  return adcValuesTotal / numSamples;;
}

float getVoltage(int pin){
  float value = getadcValue(pin, NUM_OF_AVERAGE_SAMPLES);
  if (digitalRead(PIN_RELAY_OR_MOSFET) == LOW){
  return value / 1023 * MEASURED_VCC_REF;
  }
  else {return value / 1023 * MEASURED_VCC_WITH_LOAD;}
}

float getIntensity(int pin){
  float voltage = getVoltage(pin);
  if (voltage == 0){
    return voltage / SHUNT_RESISTOR_OHM;
  }
  else return (-10/1000+(voltage / SHUNT_RESISTOR_OHM)+(ADC_INTENSITY_OFFSET_MINIMUM/1000));
}

float getPower(){
  float voltage = getVoltage(VOLTAGE_PIN);
  float intensity = getIntensity(INTENSITY_PIN);
  float power = (voltage * intensity);
  return power;
}

float getInternalRes(float loadRes, int attempts, int msDelay){
  display.clearDisplay();
  float accumulatedInternalResForAverage=0;
  for (int n=0; n<attempts; n++){
    float CalculatedInternalRes=0;
    float deltaVoltage=0;
    float ratioVoltage=0;
    float initialVoltage = getVoltage(VOLTAGE_PIN);
    uint32_t StartMillis = millis();
    digitalWrite(PIN_RELAY_OR_MOSFET, HIGH);
    while (millis() - StartMillis <= msDelay){
    }
    float droppedVoltage = getVoltage(VOLTAGE_PIN);
    deltaVoltage = initialVoltage - droppedVoltage;
    ratioVoltage = droppedVoltage/initialVoltage;
    CalculatedInternalRes = 1000*(-MEASURED_LOAD_RESISTANCE+MEASURED_LOAD_RESISTANCE*(1/ratioVoltage));
    accumulatedInternalResForAverage += CalculatedInternalRes;
    /*Serial.print(initialVoltage, 3);Serial.println("V init");
    Serial.print(n);Serial.print("n ");Serial.println(droppedVoltage, 3);
    Serial.print("deltaVoltage : ");Serial.println(deltaVoltage, 3);
    Serial.print("ratioVoltage");Serial.println(ratioVoltage, 3);
    Serial.print("CalculatedInternalRes : ");Serial.println(CalculatedInternalRes);
    Serial.print("DeltaVMoyenne : ");Serial.println((accumulatedDeltaVoltageForAverage / (n+1)), 3);Serial.println();
    */
    digitalWrite(PIN_RELAY_OR_MOSFET, LOW);
    delay(msDelay);
    //Serial.print("CalculatedInternalRes : ");Serial.println(CalculatedInternalRes);
    //Serial.print("Moy : ");Serial.println(accumulatedInternalResForAverage / (n+1));
  }
  //Serial.println(accumulatedInternalResForAverage / attempts);
  return accumulatedInternalResForAverage / attempts;
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
  EEPROM.get(0, mAsEEPROMstored);           //get float data stored from address 0 of EEPROM (Address 0 to 3 as float is 4bytes long)
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
  delay(500);
  while (exitMenu == false){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    switch (selectedMenu){
      case 1:
        display.println(">Reset EEPROM");
        display.println("Internal Resistance");
        display.println("Exit Menu");
        break;
      case 2:
        display.println("Reset EEPROM");
        display.println(">Internal Resistance");
        display.println("Exit Menu");
        break;
      case 3:
        display.println("Reset EEPROM");
        display.println("Internal Resistance");
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
  uint32_t xxx=0;
  uint32_t yyy=0;
  bool STOP=false;
  digitalWrite(PIN_RELAY_OR_MOSFET, LOW);
  int millisecFormAsInterval = 2000;
  float mAsTempStoredForEEPROMdelay=0;
  displayRefresh(secondsElapsed);
  button.tick();
  while (digitalRead(PIN_BUTTON) == HIGH){
    mAsTempStoredForEEPROMdelay = mAsEEPROMstored;
    uint32_t resetTime = millis();
    uint32_t startMillisFormAs = millis();
    mAs = 0;                                //Reset mA of actual counter
    if (STOP==false){
      while (STOP==false && (getVoltage(VOLTAGE_PIN) >= STOP_DISCHARGE_VOLTAGE)){
        if (digitalRead(PIN_BUTTON) == LOW){
          break;
        }
        digitalWrite(PIN_RELAY_OR_MOSFET, HIGH);
        uint32_t TimeElapsed = millis() - resetTime;
        //if ( (millis()-startMillisFormAs) >= millisecFormAsInterval){
        if ( (TimeElapsed/millisecFormAsInterval) > xxx/(millisecFormAsInterval/1000) ) {
          mAs += (getIntensity(INTENSITY_PIN)*1000) * (millisecFormAsInterval/1000);
          mAsEEPROMstored += (getIntensity(INTENSITY_PIN)*1000) * (millisecFormAsInterval/1000);
          mAsTempStoredForEEPROMdelay += (getIntensity(INTENSITY_PIN)*1000) * (millisecFormAsInterval/1000);
          xxx+=2;
          //Serial.print(yyy);Serial.print(" ");Serial.print(TimeElapsed/1000);Serial.print(" ");Serial.println(xxx);
          if ( (EEPROM.read(0)!=mAsEEPROMstored) && ((TimeElapsed/1000)>(yyy*2)) ){
            //EEPROM.put(0, mAsEEPROMstored);
            EEPROM.put(0, mAsTempStoredForEEPROMdelay);
            //display.setTextSize(1);
            //display.setTextColor(WHITE);
            //display.setCursor(64, 20);
            
            yyy+=5;
            //display.print(xxx);
            //display.setCursor(84, 20);
            //display.print(millis()-startMillisFormAs);
            //display.display();
            }
          startMillisFormAs = millis();
        }
        displayRefresh(TimeElapsed/1000);
        secondsElapsed = TimeElapsed/1000;
      }
      digitalWrite(PIN_RELAY_OR_MOSFET, LOW);
      STOP=true;
    }
    EEPROM.put(0, mAsTempStoredForEEPROMdelay);
  }

}
