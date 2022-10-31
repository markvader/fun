// Libraries
// MY Arduino sketch running on an Arduino Uno, hidden in a Pumpkin, with LED lights for eyes.  
// When an object moves in front of the pumpkin music or sound effects start and eyes flash for a certain length of time.
// Components used & some links
// 1. HC-SR04 ultrasonic distance sensor - https://randomnerdtutorials.com/complete-guide-for-ultrasonic-sensor-hc-sr04/
// 2. NeoPixel Jewel x 2 - https://www.adafruit.com/product/2226
// 3. DFPlayer - A Mini MP3 Player For Arduino - https://github.com/DFRobot/DFRobotDFPlayerMini/
// 4. 8 OHM (i think) speaker - Can get it quite loud.
// 5. Arduino Uno R3 Clone (Buono brand)
// 6. Proto Board & random collection of jumper cables.  If this was a permanent project I would solder to perf board
// 7. Micro USB cable and plug.  
// Could possibly have run this on battery pack but figured since I had light+music+distance sensor running, plugged in was sensible choice

#include <NewPing.h>
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <Adafruit_NeoPixel.h>

// Definitions
// HC-SR04 ultrasonic distance sensor
#define trigPin  2
#define echoPin  3
#define maxDistance 350 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
// NeoPixel Jewels
#define jewelPin 9
#define numPixels 14 // I am using two NeoPixel Jewels which have 7 leds on each.
#define brightnesslevel 50
// DFPlayer
#define dfRXPin  10
#define dfTXPin  11

// HC-SR04 ultrasonic distance sensor
NewPing sonar(trigPin, echoPin, maxDistance); // NewPing setup of pins and maximum distance.
unsigned int distance, newdistance, olddistance = 100, variance;
unsigned int pingSpeed = 50; // How frequently are we going to send out a ping (in milliseconds). 50ms would be 20 times a second.
unsigned long pingTimer;     // Holds the next ping time.

// NeoPixel Jewels
Adafruit_NeoPixel strip = Adafruit_NeoPixel(numPixels, jewelPin, NEO_GRB + NEO_KHZ800);

// DFPlayer
SoftwareSerial mySoftwareSerial(dfRXPin, dfTXPin); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);


void setup() {
  Serial.begin(9600); // Open serial monitor at 9600 baud to see ping results.
  mySoftwareSerial.begin(9600);
  
  pingTimer = millis(); // Start now.
  
  // Jewel Setup
  strip.begin();
  strip.setBrightness(brightnesslevel);
  strip.show(); // Initialize all pixels to 'off'

  // DFPlayer Setup
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  Serial.println(F("DFPlayer Mini online."));
  myDFPlayer.volume(20);  //Set volume value. From 0 to 30
  //myDFPlayer.play(1);  //Play the first mp3 on bootup  
}

void loop() {
  // Random jumble of code where i was trying to get all pixels to show same colour when not doing "theaterChaseRainbow"
  // Jewels seem to be a bit weird where sometimes three pixels out of 7 show same colour but others are random??
  // was playing with all on and all off. Dont copy me. 
  if (NUMOFPIXELS > 10) {colorWipe(strip.Color(255, 0, 0), 50); }
  else {
    for (int i=0; i<strip.numPixels(); i++){
      strip.setPixelColor(i,0);
      strip.setBrightness(brightnesslevel);
      strip.show();
    }
  }
  
  // Calculate distance from detected object. If less than 50cm play mp3 file and run theaterChaseRainbow function
  if (millis() >= pingTimer) {   // pingSpeed milliseconds since last ping, do another ping.
    pingTimer += pingSpeed;      // Set the next ping time.
    sonar.ping_timer(echoCheck); // Send out the ping, calls "echoCheck" function every 24uS where you can check the ping status.
    // Was going to filter distance values that have little variance from previous values (hence, new, old and variance variables left in code)
    newdistance = distance;
    int variance = (newdistance - olddistance);
    if (distance < 50) {
      olddistance = newdistance;
      Serial.print(distance);
      Serial.println("cm away");
      myDFPlayer.play(12);  // Play the 12th mp3 file on SD Card.
      theaterChaseRainbow(30); // Run flashing light function 
      strip.clear(); // Set all pixel colors to 'off'
      delay(15); // Give the loop a short rest before restarting
    }
    else {
      Serial.print(distance);
      Serial.println("cm, far away");
//      Serial.print("variance: ");
//      Serial.println(abs(variance));
      olddistance = newdistance;
    }
  }
}

int echoCheck() { // Timer2 interrupt calls this function every 24uS where you can check the ping status.
  // Don't do anything here!
  if (sonar.check_timer()) { // This is how you check to see if the ping was received.
    // Here's where you can add code.
    distance = sonar.ping_result / US_ROUNDTRIP_CM;
//    Serial.print("Distance: ");
//    Serial.print(distance); // Ping returned, uS result in ping_result, convert to cm with US_ROUNDTRIP_CM.
//    Serial.println("cm");
    return distance;
  }
}


//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.setBrightness(brightnesslevel);
        strip.show();
       
        delay(wait);
       
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
  }
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
  delay(wait);
  
  }
}
