/*********************************************************************
 * 
 * This example is for a 128x64 size display using I2C to communicate
 * 3 pins are required to interface (2 I2C and one reset)
 * 
 *********************************************************************/

#include <MenuSystem.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


// Encoder Pins

const int                                 PinCLK   = 2;     // Used for generating interrupts using CLK signal
const int                                 PinDT    = 4;     // Used for reading DT signal
const int                                 PinSW    = 8;     // Used for the push button switch


volatile int                              virtualPosition = 0;

// -----------------------------------------------------------------------------
// Interrupt service routine is executed when a HIGH to LOW transition is detected on CLK

void isr ()  {

  static unsigned long  lastInterruptTime = 0;

  unsigned long   interruptTime = millis();

  // If interrupts come faster than 5ms, assume it's a bounce and ignore
  if (interruptTime - lastInterruptTime > 5) {
    if (!digitalRead(PinDT))
      virtualPosition = virtualPosition + 1;
    else
      virtualPosition = virtualPosition - 1;
  }
  lastInterruptTime = interruptTime;
} // ISR



#define SELECTED_DISPLAY_DELAY 1500

// Menu variables
MenuSystem ms;
Menu mm("Main Menu");
MenuItem mm_mi1("Read A0 PIN");
MenuItem mm_mi2("Blink led");

Menu mu1("Date & Time");
MenuItem mu1_mi1("Show current");
//MenuItem mu1_mi2("Set");
MenuItem mu1_mi3("Go back");


boolean menuSelected = false; // no menu item has been selected
boolean commandReceived = false;// no command has been received (Seria, button, etc)
enum setMenuSelected_Type { 
  DATETIME, OTHER }; // this enum is in case we want to expand this example
setMenuSelected_Type setMenu;

byte cursorPosition;


int led = 13; // connect a led+resistor on pin 41 or change this number to 13 to use default arduino led on board
int ledState = LOW;
String dateTime = "14.08.2015 16:00";

// Menu callback functions
void on_item1_selected(MenuItem* p_menu_item) {
  display.clearDisplay();

  display.setCursor(0,1);
    display.setTextSize(1);
  display.print("Analog A0: ");

    display.setCursor(0,3);
    display.setTextSize(2);

  display.print(analogRead(A0));
  display.display();
    delay(SELECTED_DISPLAY_DELAY);

}

void on_item2_selected(MenuItem* p_menu_item) {
  ledState = !ledState;
  digitalWrite(led,ledState);
  display.setCursor(0,1);
  display.setTextSize(2);

  if(ledState)
  {
    display.clearDisplay();
    display.print("Led: ON ");
  }
  else
  {
    display.clearDisplay();
    display.print("Led: OFF ");

  }
  display.display();
  delay(SELECTED_DISPLAY_DELAY);

}

// on_back_selected is usefull if you don't have a button to make the back function
void on_back_selected(MenuItem* p_menu_item) {
  ms.back();
}

void on_time_show_selected(MenuItem* p_menu_item) {
  display.clearDisplay();
  display.setCursor(0,1);
    display.setTextSize(2);

  display.print(dateTime);
  delay(SELECTED_DISPLAY_DELAY);
  display.display();

}

void setup()   {  

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  Serial.begin(9600);

  pinMode(PinCLK,INPUT);
  pinMode(PinDT, INPUT);
  pinMode(PinSW, INPUT);

  attachInterrupt(0, isr, FALLING);   // interrupt 0 is always connected to pin 2 on Arduino UNO

  mm.add_item(&mm_mi1, &on_item1_selected);
  mm.add_item(&mm_mi2, &on_item2_selected);
  mm.add_menu(&mu1);
  mu1.add_item(&mu1_mi1, &on_time_show_selected);
  // mu1.add_item(&mu1_mi2, &on_time_set_selected);
  mu1.add_item(&mu1_mi3, &on_back_selected);
  ms.set_root_menu(&mm);
  displayMenu();

}


void loop() {


  display.setTextSize(1);
  display.setTextColor(WHITE);  

  int lastCount = 0;

  displayMenu();

  while (true) {

    if (!(digitalRead(PinSW))) {        // check if pushbutton is pressed
      virtualPosition = 0;            // if YES, then reset counter to ZERO
      while (!digitalRead(PinSW)) {
      }  // wait til switch is released
      if(!menuSelected)
        ms.select();
      else if(cursorPosition<15)
        display.setCursor(++cursorPosition,1);
      display.display();


    }   



    if (virtualPosition != lastCount) {
      if(lastCount < virtualPosition)
      {
        ms.next();
        displayMenu();

        display.display();

      }
      if(lastCount > virtualPosition)
      {
        ms.prev();
        displayMenu();

        display.display();

      }
      lastCount = virtualPosition;


    } 
  }
}







void displayMenu() {
  display.clearDisplay();
  display.setCursor(0,0);
    display.setTextSize(1);

  // Display the menu
  Menu const* cp_menu = ms.get_current_menu();

  display.println(cp_menu->get_selected()->get_name());

  display.display();
}






