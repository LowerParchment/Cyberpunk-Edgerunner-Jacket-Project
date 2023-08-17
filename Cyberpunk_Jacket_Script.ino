//Dylan McClellan
//Cyberpunk Jacket LED Array Script

//This script is a reimagining of a code from an online user that was developed for a microphone to use an LED array
//My changes allow the addition of new LED modes that are user-defined with an infinite number of posibilities
//including the ability to change between profiles with a push-button

#include <arduinoFFT.h>
#include <FastLED.h>

#define SAMPLES 64        // Must be a power of 2
#define MIC_IN A0         // Use A0 for mic input
#define LED_PIN     7     // Data pin to LEDS
#define NUM_LEDS    12  
#define BRIGHTNESS  150    // LED information 
#define LED_TYPE    WS2812B
#define COLOR_ORDER RGB 
#define BUTTON_PIN 3
#define xres 4            // Total number of  columns in the display
#define yres 3            // Total number of  rows in the display

int profileIterator = 0;                    //This is going to be a counter that the button will advance
bool buttonState = false;    //variable for reading the buttons current state, false = normal, true = pushed

double vReal[SAMPLES];
double vImag[SAMPLES];

int Intensity[xres] = { }; // initialize Frequency Intensity to zero
int Displacement = 1;

CRGB leds[NUM_LEDS];            // Create LED Object
arduinoFFT FFT = arduinoFFT();  // Create FFT object

void setup() {
  pinMode(MIC_IN, INPUT);       //establish microphone as an input
  pinMode(BUTTON_PIN, INPUT);  //establish the button as an input
  
  Serial.begin(115200);         //Initialize Serial
  delay(3000);                  // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip ); //Initialize LED strips
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  const int NUMBER_OF_LED_PROFILES = 3;       //This is the number of profiles that we can switch between

  switch(profileIterator) {                   //This switch statement will pick the profile based on our iterator
    case 0:
      Visualizer();                           //Run the visualizer code profile
      break;
    case 1:
      StaticColorGreen();                     //Run the Static Green code profile
      break;
    case 2:
      StaticColorRed();                       //Run the Static Red code profile
      break;
  }

  //respond to button press and toggle between button states
  if (digitalRead(BUTTON_PIN) == true) {                                         //button pushed
    buttonState = !buttonState;                                                  //invert the button state for using it as a toggle instead of a push down only
    profileIterator = (profileIterator + 1) % NUMBER_OF_LED_PROFILES;            //increment the profile counter
  }
  while(digitalRead(BUTTON_PIN) == true) {                                       //wait for the release of the button
    delay(50);
  }
}

void StaticColorGreen(){
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(0, 255, BRIGHTNESS);    //This is a CHSV light blue color setter for the base before modifying individual leds        //final color: 0x0084ff  (209, 255, 150)
  }
  FastLED.show();
}

void StaticColorRed(){
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(354, 255, BRIGHTNESS);    //This is a CHSV light blue color setter for the base before modifying individual leds        //final color: 0x0084ff  (209, 255, 150)
  }
  FastLED.show();
}

void Visualizer(){        //Run visualizer code 
  //Collect Samples
  getSamples();
  
  //Update Display
  displayUpdate();
  
  FastLED.show();
}

void getSamples(){
  for(int i = 0; i < SAMPLES; i++){
    vReal[i] = analogRead(MIC_IN);
    Serial.println(vReal[i]);
    vImag[i] = 0;
  }

  //FFT
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

  //Update Intensity Array
  for(int i = 2; i < (xres*Displacement)+2; i+=Displacement){
    vReal[i] = constrain(vReal[i],0 ,2047);            // set max value for input data
    vReal[i] = map(vReal[i], 0, 2047, 0, yres);        // map data to fit our display

    Intensity[(i/Displacement)-2] --;                      // Decrease displayed value
    if (vReal[i] > Intensity[(i/Displacement)-2])          // Match displayed value to measured value
      Intensity[(i/Displacement)-2] = vReal[i];
  }
}

void displayUpdate(){
  int color = 193;
  for(int i = 0; i < xres; i++){
    for(int j = 0; j < yres; j++){
      if(j <= Intensity[i]){                                // Light everything within the intensity range
        if(j%2 == 0){
          leds[(xres*(j+1))-i-1] = CHSV(color, 255, BRIGHTNESS);
        }
        else{
          leds[(xres*j)+i] = CHSV(color, 255, BRIGHTNESS);
        }
      }
      else{                                                  // Everything outside the range goes dark
        if(j%2 == 0){
          leds[(xres*(j+1))-i-1] = CHSV(color, 255, 0);
        }
        else{
          leds[(xres*j)+i] = CHSV(color, 255, 0);
        }
      }
    }
    //color += 255/xres;                                      // Increment the Hue to get the Rainbow
  }
}


//This pseudocode will cycle through profiles
//start in profile 0 and store the value
//every time the button is pressed, add one to the old number, then mod by the number of profiles
//if you go past the largest profile, go back to 0
//make sure to ensure that every index is in bounds

//x = (x + 1) % numOfProfiles