Simple Battery Capacity Tester using Sparkfun Arduino Pro Micro and 32x128 SSD1306 OLED screen

PINS connections :
  SSD1306 Oled Display
    SDA -> PIN2
    SCK -> PIN3

You can access the menu when discharge is off by clicking the pushbutton.
Navigate in menu using push button (up or down depending on the On/Off state, longpress to select item in menu)

To Do :
- [OK] Add feature to store data in EEPROM to keep data even after reset or unwanted poweroff
- [OK] Add Menu
- [OK] Erase EEPROM via menu caapability
- [OK] Try to implement battery internal Resistance calculator
- [OK] fix time shifting toooo much
- [OK] Make readable schematics
- Add Wh counter
- Add Wh counter to EEPROM
- Add seconds elapsed to EEPROM
- Add max intensity parameter to prevent from short circuit or load resistance too low
- Add History function
- Connect & control Buck/boost to charge ?
- Add Voltage divider feature in code to allow multi-cells accus
- Add Low Voltage cut-off parameter in menu and store it into the EEPROM
- Add an avenced Menu to modify parameters such as : shunt resistor, pin difinitions, stop discharge voltage etc...

Exemple result on screen with Ni-MH 1.2V Cell :
![Image](https://github.com/joyel24/SimpleArduinoBatteryCapacityOLED/blob/main/.readme/BetterPictureComing.jpg?raw=true)

My actual prototype looks like :
![Image](https://github.com/joyel24/SimpleArduinoBatteryCapacityOLED/blob/main/.readme/Actual_Prototype.jpg?raw=true)

