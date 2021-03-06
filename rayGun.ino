// ************************************************************************
// **                                                                    **
// **                          RayGun 1.2                                **
// **                          © Pepe Fernández, May 2017                **
// **                          www.geekmatic.com                         **
// **                                                                    **
// ************************************************************************

#include <Adafruit_NeoPixel.h>
#include "OneButton.h"

#define TRIGGER_PIN    7   // Trigger
#define SELECTOR_PIN   8   // Selector
#define PIXEL_PIN      9   // NeoPixels
#define RUMBLE_PIN     10  // Motor
#define BOOST_PIN      11  // Boost

#define PIXEL_COUNT 17

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

OneButton selector(SELECTOR_PIN, false);
OneButton boost(BOOST_PIN, false);
OneButton trigger(TRIGGER_PIN, false);

bool shootStatus = false;
bool boosterStatus = false;
bool rumbleToggle = false;
bool fireInTheHole = false;
bool repeatFire = false;

uint8_t bootFlag = 0;
uint8_t blasterType = 0;
uint32_t mainFire;
uint32_t secondaryFire;

uint8_t shootTimer = 0;
uint8_t energyCounter = 0;
uint8_t wheelCounter = 0;
uint8_t mainCounter = 0;
uint8_t upCounter = 0;
uint8_t downCounter = 0;
uint8_t rumbleCounter = 0;

uint8_t shootsFired = 0;
uint8_t energyBarLeft = 8;
uint8_t boostLaser = 0;
uint8_t boostLevel = 32;
uint8_t rumbleSpeed = 0;

void setup() {
  pinMode(RUMBLE_PIN, OUTPUT);
  randomSeed(analogRead(0)); // Initialize random number generator
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  // OneButton Init
  selector.attachLongPressStart(startSELECT);
  boost.attachLongPressStart(startBOOST);
  boost.attachDuringLongPress(longPressBOOST);
  boost.attachLongPressStop(stopBOOST);
  trigger.attachLongPressStart(startTRG);
  trigger.attachDuringLongPress(longPressTRG);
  trigger.attachLongPressStop(stopTRG);
}

void loop() {
  if (bootFlag == 0 ) { bootInit(); }
  checkButtons();
  checkEnergy();
  boostControl();
  rumbleControl();
  checkShoot();
  shootMode(blasterType);
  energyMode(blasterType);
  wheelMode(blasterType);
  counterSystem();
  strip.show();
  delay(boostLevel);
}

void bootInit() {
  bootFlag = 1;
}

void checkButtons() {
  selector.tick();
  boost.tick();
  trigger.tick();
}

void startSELECT() {
   if (!energyBarLeft) {
    energyBarLeft = 8;
    energyCounter = 0;
  } else {
    blasterType ++;
    energyCounter = 0;
    wheelCounter = 0;
    if(blasterType > 10) { blasterType = 0; }
  }
}

void startBOOST() {
  boosterStatus = true;
}

void longPressBOOST() {
  boosterStatus = true;
}

void stopBOOST() {
  boosterStatus = false;
}

void startTRG() {
  shootStatus = true;
}

void longPressTRG() {
  repeatFire = true;
}

void stopTRG() {
  shootStatus = false;
  repeatFire = false;
}

void checkShoot() {
  if(shootStatus) { 
    fireInTheHole = true;
  }
  if(repeatFire) {
    shootStatus = false;
    if(boostLevel <= 16) { fireInTheHole = true; }
  }
  if(energyBarLeft) {
    if(fireInTheHole) {
      fire(mainFire, secondaryFire, 255);
      shootTimer ++;
      if(shootTimer >= 6) { 
        fire(0, 0, 0);
        shootTimer = 0;
        fireInTheHole = false;
        shootsFired ++;
      }
    }
  }
  if(!energyBarLeft) {
    fire(0, 0, 0);
    shootTimer = 0;
    fireInTheHole = false;
  }
}

void fire(uint32_t main, uint32_t secondary, uint8_t rumble) {
  strip.setPixelColor(8, main);
  strip.setPixelColor(9, secondary);
  analogWrite(RUMBLE_PIN, rumble);
}

void checkEnergy() {
  if (shootsFired >= 6) {
    shootsFired = 0;
    energyBarLeft --;
    if (energyBarLeft <=0) {
      energyBarLeft = 0;
    }
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

void boostControl() {
  if(!boosterStatus || !energyBarLeft) {
   upCounter ++;
   if(upCounter >=8) {
     boostLevel ++;
     upCounter = 0;
   }
   if (boostLevel >= 32) { boostLevel = 32; }
  }
  if(boosterStatus) {
    if (energyBarLeft != 0) {
      downCounter ++;
      if(downCounter >=8) {
        boostLevel --;
        downCounter = 0;
      }
      if (boostLevel < 6) { boostLevel = 6; }
    }
  }
  boostLaser = (64 - boostLevel) * 4;
}

void rumbleControl() {
  if(boostLevel <= 20) {
    rumbleSpeed = (21 - boostLevel) * 14;
    rumbleCounter ++;
    if(rumbleCounter>=12) { 
      rumbleToggle = !rumbleToggle;
      rumbleCounter = 0;
    }
    if(rumbleToggle) { analogWrite(RUMBLE_PIN, rumbleSpeed); }
    if(!rumbleToggle) { analogWrite(RUMBLE_PIN, 0); }
  }
  if(boostLevel > 20 && rumbleToggle) {
    rumbleToggle = !rumbleToggle;
    analogWrite(RUMBLE_PIN, 0);
  }
}

void counterSystem() {
  mainCounter ++;
  if (mainCounter >= 256) { mainCounter = 0; }
  wheelCounter ++;
  if (wheelCounter > 24) { wheelCounter = 0; }
}

void shootMode(int i) {
  switch(i){
    case 0: mainFire = strip.Color(255, 0, 255);
            secondaryFire = strip.Color(boostLaser, 0, 0);    // Purple/Red
            break;
    case 1: mainFire = strip.Color(255, 255, 0);
            secondaryFire = strip.Color(0, boostLaser, 0);  // Orange/Green
            break;
    case 2: mainFire = strip.Color(0, 255, 255);
            secondaryFire = strip.Color(0, 0, boostLaser);  // Light Green/Blue
            break;
    case 3: mainFire = strip.Color(255, 255, 255);
            secondaryFire = strip.Color(boostLaser, 0, boostLaser);  // White/Purple
            break;
    case 4: mainFire = strip.Color(255, 0, 0);
            secondaryFire = strip.Color(boostLaser, 0, 0); // Red
            break;
    case 5: mainFire = strip.Color(0,   255,   0);
            secondaryFire = strip.Color(0,   boostLaser,   0); // Green
            break;
    case 6: mainFire = strip.Color(  0,   0, 255);
            secondaryFire = strip.Color(  0,   0, boostLaser);  // Blue
            break;
    case 7: mainFire = strip.Color(  255,   0, 255);
            secondaryFire = strip.Color(  boostLaser,   0, boostLaser); // Purple
            break;
    case 8: mainFire = strip.Color(  255,   255, 255);
            secondaryFire = strip.Color(  boostLaser,   boostLaser, boostLaser);  // White
            break;
    case 9: mainFire = Wheel(mainCounter & 255);
            secondaryFire = Wheel(mainCounter & boostLaser);
            break;
    case 10: mainFire = Wheel(mainCounter & 255);
             secondaryFire  = Wheel( mainCounter & boostLaser);
            break;
  }
}

void energyMode(int i) {
  switch(i){
    case 0: energyBar(Wheel(((energyCounter*8) + 191)& 255));    // Purple-Red
            break;
    case 1: energyBar(Wheel(((energyCounter*8))& 255));   // Red-Green
            break;
    case 2: energyBar(Wheel(((energyCounter*8) + 64)& 255));   // Yellow-Light Blue
            break;
    case 3: energyBar(strip.Color((energyCounter+1) * 8, (energyCounter+1) * 8, (energyCounter+1) * 8));  // White
            break;
    case 4: energyBar(strip.Color((energyCounter+1) * 16, 0, 0)); // Red
            break;
    case 5: energyBar(strip.Color(0,   (energyCounter+1) * 16,   0)); // Green
            break;
    case 6: energyBar(strip.Color(  0,   0, (energyCounter+1) * 16)); // Blue
            break;
    case 7: energyBar(strip.Color(  (energyCounter+1) * 16,   0, (energyCounter+1) * 16)); // Purple
            break;
    case 8: energyBar(strip.Color(  (energyCounter+1) * 16,   (energyCounter+1) * 16, (energyCounter+1) * 16)); // White
            break;
    case 9: energyBar(Wheel(((wheelCounter * 256 / 8) + mainCounter) & 255));
            break;
    case 10: energyBar(Wheel(((wheelCounter * 256 / 8) + mainCounter) & 255));
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
    case 4: wheelWipe(strip.Color(random(32,64), 0, 0),strip.Color(random(1,2), 0, 0)); // Red
            break;
    case 5: wheelWipe(strip.Color(0,   random(32,64),   0),strip.Color(0,   random(1,2),   0)); // Green
            break;
    case 6: wheelWipe(strip.Color(  0,   0, random(32,64)),strip.Color(  0,   0, random(1,2))); // Blue
            break;
    case 7: wheelWipe(strip.Color(  random(32,64),   0, random(32,64)),strip.Color(  random(1,2),   0, random(1,2))); // Purple
            break;
    case 8: wheelWipe(strip.Color(  random(32,64),   random(32,64), random(32,64)),strip.Color(  random(1,2),   random(1,2), random(1,2))); // White
            break;
    case 9: wheelCycle();
            break;
    case 10: wheelChaseRainbow();
            break;
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

