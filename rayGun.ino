// ************************************************************************
// **                                                                    **
// **                          RayGun 1.0                                **
// **                                                                    **
// ************************************************************************

#include <Adafruit_NeoPixel.h>




#define TRIGGER_PIN    7   // Trigger
#define SELECTOR_PIN   8   // Selector
#define PIXEL_PIN      9   // NeoPixels.
#define RUMBLE_PIN     10  // Motor.
#define BOOST_PIN      11  // Boost

#define PIXEL_COUNT 17

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

bool selectorOldState = LOW;
bool boostOldState = LOW;
bool triggerOldState = LOW;
bool shootStatus = LOW;
bool boosterStatus = LOW;

uint8_t selector;
uint8_t boost;
uint8_t trigger; 
uint8_t bootFlag = 0;
uint8_t shootType = 0;
uint8_t energyType = 0;
uint8_t wheelType = 0;
uint8_t blasterType = 0;
uint32_t shootColorA;
uint32_t shootColorB;

uint8_t shootCounter = 0;
uint8_t energyCounter = 0;
uint8_t wheelCounter = 0;
uint8_t mainCounter = 0;
uint8_t upCounter = 0;
uint8_t downCounter = 0;

uint8_t shootsFired = 0;
uint8_t energyBarLeft = 8;
uint8_t boostLaser = 0;
uint8_t boostLevel = 32;
uint8_t rumbleSpeed = 0;

long randNumber;

void setup() {
  pinMode(SELECTOR_PIN, INPUT);
  pinMode(BOOST_PIN, INPUT);
  pinMode(TRIGGER_PIN, INPUT);
  pinMode(RUMBLE_PIN, OUTPUT);
  randomSeed(analogRead(0));
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  if (bootFlag == 0 ) { bootInit(); }
  selectorStatus();
  boostStatus();
  triggerStatus();
  checkShoot(shootColorA, shootColorB);
  checkEnergy();
  blasterMode(blasterType);
  boostControl();
  rumbleControl();
  counterSystem();
  strip.show();
  delay(boostLevel);
}

void bootInit() {
  bootFlag = 1;
}

void boostControl() {
  if(boosterStatus == LOW || energyBarLeft == 0) {
   upCounter ++;
   if(upCounter >=8) {
     boostLevel ++;
     upCounter = 0;
   }
   if (boostLevel >= 32) { boostLevel = 32; }
  }
  if(boosterStatus == HIGH) {
    if (energyBarLeft != 0) {
      downCounter ++;
      if(downCounter >=8) {
        boostLevel --;
        downCounter = 0;
      }
      if (boostLevel == 0) { boostLevel = 1; }
    }
  }
  boostLaser = (64 - boostLevel) * 4;
}

void rumbleControl() {
  if(boostLevel <= 16) {
    rumbleSpeed = (16 - boostLevel) * 13;
    analogWrite(RUMBLE_PIN, rumbleSpeed);
  }
}
void checkShoot(uint32_t c, uint32_t d) {
   if(shootStatus == HIGH) {
     if (energyBarLeft != 0) {
       strip.setPixelColor(8, c);
       strip.setPixelColor(9, d);
       analogWrite(RUMBLE_PIN, 255);
       strip.show();
       delay(boostLevel);
       strip.setPixelColor(8, strip.Color(0, 0, 0));
       strip.setPixelColor(9, strip.Color(0, 0, 0));
       analogWrite(RUMBLE_PIN, 0);
       strip.show();
       shootsFired ++;
     }
   }
}

void checkEnergy() {
  if (shootsFired >= 16) {
    shootsFired = 0;
    energyBarLeft --;
    if (energyBarLeft <=0) {
      energyBarLeft = 0;
    }
  }
}
  
void selectorStatus() {
  // Get current selector button state.
  bool selectorNewState = digitalRead(SELECTOR_PIN);

  // Check if state changed from high to low (button press).
  if (selectorNewState == HIGH && selectorOldState == LOW) {
    // Short delay to debounce button.
    delay(20);
    // Check if button is still low after debounce.
    selectorNewState = digitalRead(SELECTOR_PIN);
    if (selectorNewState == HIGH) {
      if (energyBarLeft == 0) {
        energyBarLeft = 8;
      } else {
        blasterType ++;
        if(blasterType > 10) {
          blasterType = 0; }
      }
    }
  }
  // Set the last button state to the old state.
  selectorOldState = selectorNewState;
}

void boostStatus() {
  // Get current selector button state.
  bool boostNewState = digitalRead(BOOST_PIN);
  // Check if state changed from high to low (button press).
  if (boostNewState == HIGH && boostOldState == LOW) {
    // Short delay to debounce button.
    delay(20);
    // Check if button is still low after debounce.
    boostNewState = digitalRead(BOOST_PIN);
    if (boostNewState == HIGH) {
      boosterStatus = boostNewState;
    }
  }
  // Set the last button state to the old state.
  boostOldState = boostNewState;
  boosterStatus = boostNewState;
}

void triggerStatus() {
  // Get current selector button state.
  bool triggerNewState = digitalRead(TRIGGER_PIN);
  // Check if state changed from high to low (button press).
  if (triggerNewState == HIGH && triggerOldState == LOW) {
    // Short delay to debounce button.
    delay(20);
    // Check if button is still low after debounce.
    triggerNewState = digitalRead(TRIGGER_PIN);
    if (triggerNewState == HIGH) {
       shootStatus = triggerNewState;     
    }
  }
  // Set the last button state to the old state.
  triggerOldState = triggerNewState;
  shootStatus = triggerNewState;
}

void counterSystem() {
  mainCounter ++;
  if (mainCounter >= 256) { mainCounter = 0; }
  wheelCounter ++;
  if (wheelCounter > 24) { wheelCounter = 0; }
}

void blasterMode(int i) {
  switch(i){
    case 0: shootMode(0);
            energyMode(0);
            wheelMode(0);    // Purple/Red
            break;
    case 1: shootMode(1);
            energyMode(1);
            wheelMode(1);   // Orange/Green
            break;
    case 2: shootMode(2);
            energyMode(2);
            wheelMode(2);   // Light Green/Blue
            break;
    case 3: shootMode(3);
            energyMode(3);
            wheelMode(3);   // White/Purple
            break;
    case 4: shootMode(4);
            energyMode(4);
            wheelMode(4); // Red
            break;
    case 5: shootMode(5);
            energyMode(5);
            wheelMode(5);  // Green
            break;
    case 6: shootMode(6);
            energyMode(6);
            wheelMode(6);  // Blue
            break;
    case 7: shootMode(7);
            energyMode(7);
            wheelMode(7);  // Purple
            break;
    case 8: shootMode(8);
            energyMode(8);
            wheelMode(8);  // White
            break;
    case 9: shootMode(9);
            energyMode(9);
            wheelMode(9); 
            break;
    case 10: shootMode(10);
            energyMode(10);
            wheelMode(10); 
            break;
  }
}
void shootMode(int i) {
  switch(i){
    case 0: shootColorA = strip.Color(255, 0, 255);
            shootColorB = strip.Color(boostLaser, 0, 0);    // Purple/Red
            break;
    case 1: shootColorA = strip.Color(255, 255, 0);
            shootColorB = strip.Color(0, boostLaser, 0);  // Orange/Green
            break;
    case 2: shootColorA = strip.Color(0, 255, 255);
            shootColorB = strip.Color(0, 0, boostLaser);  // Light Green/Blue
            break;
    case 3: shootColorA = strip.Color(255, 255, 255);
            shootColorB = strip.Color(boostLaser, 0, boostLaser);  // White/Purple
            break;
    case 4: shootColorA = strip.Color(255, 0, 0);
            shootColorB = strip.Color(boostLaser, 0, 0); // Red
            break;
    case 5: shootColorA = strip.Color(0,   255,   0);
            shootColorB = strip.Color(0,   boostLaser,   0); // Green
            break;
    case 6: shootColorA = strip.Color(  0,   0, 255);
            shootColorB = strip.Color(  0,   0, boostLaser);  // Blue
            break;
    case 7: shootColorA = strip.Color(  255,   0, 255);
            shootColorB = strip.Color(  boostLaser,   0, boostLaser); // Purple
            break;
    case 8: shootColorA = strip.Color(  255,   255, 255);
            shootColorB = strip.Color(  boostLaser,   boostLaser, boostLaser);  // White
            break;
    case 9: shootColorA = Wheel(mainCounter & 255);
            shootColorB = Wheel(mainCounter & boostLaser);
            break;
    case 10: shootColorA = Wheel(mainCounter & 255);
             shootColorB  = Wheel( mainCounter & boostLaser);
            break;
  }
}
void energyMode(int i) {
  switch(i){
    case 0: energyBar(strip.Color(64, 0, 64));    // Purple
            break;
    case 1: energyBar(strip.Color(64, 64, 0));  // Orange
            break;
    case 2: energyBar(strip.Color(0, 64, 64));  // Light Green
            break;
    case 3: energyBar(strip.Color(64, 64, 64));  // White
            break;
    case 4: energyBar(strip.Color(127, 0, 0)); // Red
            break;
    case 5: energyBar(strip.Color(0,   127,   0)); // Green
            break;
    case 6: energyBar(strip.Color(  0,   0, 127)); // Blue
            break;
    case 7: energyBar(strip.Color(  127,   0, 127)); // Purple
            break;
    case 8: energyBar(strip.Color(  127,   127, 127)); // White
            break;
    case 9: energyBar(Wheel(mainCounter & 255));
            break;
    case 10: energyBar(Wheel(mainCounter & 255));
            break;
  }
}
void wheelMode(int i) {
  switch(i){
    case 0: wheelWipe(strip.Color(random(32,64), 0, random(32,64)),strip.Color(random(1,4), 0, 0));    // Purple/Red
            break;
    case 1: wheelWipe(strip.Color(random(32,64), random(32,64), 0),strip.Color(0, random(1,4), 0));  // Orange/Green
            break;
    case 2: wheelWipe(strip.Color(0, random(32,64), random(32,64)),strip.Color(0, 0, random(1,4)));  // Light Green/Blue
            break;
    case 3: wheelWipe(strip.Color(random(32,64), random(32,64), random(32,64)),strip.Color(random(1,4), 0, random(1,4)));  // White/Purple
            break;
    case 4: wheelChase(strip.Color(random(32,64), 0, 0)); // Red
            break;
    case 5: wheelChase(strip.Color(0,   random(32,64),   0)); // Green
            break;
    case 6: wheelChase(strip.Color(  0,   0, random(32,64))); // Blue
            break;
    case 7: wheelChase(strip.Color(  random(32,64),   0, random(32,64))); // Purple
            break;
    case 8: wheelChase(strip.Color(  random(32,64),   random(32,64), random(32,64))); // White
            break;
    case 9: wheelCycle();
            break;
    case 10: wheelChaseRainbow();
            break;
  }
}

void energyBar(uint32_t c) {
    energyCounter++;
    if(energyCounter <= energyBarLeft) {
        strip.setPixelColor(8-energyCounter, c);
    } else {
      strip.setPixelColor(8-energyCounter, strip.Color(0, 0, 0));
    }
    if(energyCounter > 8) {
      energyCounter = 0;
    }
}

// Fill the dots one after the other with a color
void wheelWipe(uint32_t c, uint32_t d) {
  if (wheelCounter < 24) {
    strip.setPixelColor((24-wheelCounter) + 9, c);
  }
  strip.setPixelColor((24-wheelCounter) + 10, d); 
}

// Slightly different, this makes the rainbow equally distributed throughout
void wheelCycle() {
      strip.setPixelColor((24-wheelCounter) + 10, Wheel(((wheelCounter * 256 / 24) + mainCounter) & 255));
}

//Theatre-style crawling lights.
void wheelChase(uint32_t c) {
    if (wheelCounter < 24) {
      strip.setPixelColor((24-wheelCounter) + 9, c);
    }
    strip.setPixelColor((24-wheelCounter) + 10, strip.Color(0, 0, 0));
}

//Theatre-style crawling lights with rainbow effect
void wheelChaseRainbow() {
    if (wheelCounter < 24) {
      strip.setPixelColor((24-wheelCounter) + 9, Wheel(((wheelCounter * 256 / 24) + mainCounter) & 255));
    }
    strip.setPixelColor((24-wheelCounter) + 10, strip.Color(0, 0, 0));
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

