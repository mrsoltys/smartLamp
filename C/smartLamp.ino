/*
smartLamp.ino
Dr. Mike Soltys <soltys@colorado.edu>
University of Colorado at Boulder
Engineering+ Program

Code for bedside lamp that tells the current and forecasted temperature. 
Uses two sets of 12 LED neopixel rings attached to pin D3 for light.
    -https://www.adafruit.com/products/1643

Some code modifed from:
    -https://github.com/spark/warsting
    -https://github.com/sparkfun/Inventors_Kit_For_Photon_Experiments/tree/master/11-OLEDApps

Uses OpenWeatherMap API. Please get your own API key from
    -http://home.openweathermap.org/users/sign_up

Development environment specifics:
    Particle Build environment (https://www.particle.io/build)
    Particle Photon
    
Distributed as-is; no warranty is given. 
*/

//Include Approprate Libraries
#include <neopixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

//Set to manual for full control
SYSTEM_MODE(MANUAL);

//////////////////////////////
//    RGB LED VALUES        //
//////////////////////////////
byte rVal=0;
byte gVal=0;
byte bVal=0;

byte lampSetting=0;

//////////////////////////////
//    VARS FOR NEOPIXELS    //
//////////////////////////////
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      12
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, 3);

//////////////////////////////
//      Time Variables      //
//////////////////////////////
int curTime;
int tempTime= 32767;
int maxTemp, curTemp;

//////////////////////////////
//    VARS FOR WEBHOOKS     //
//////////////////////////////
#define FORECAST_RESP   "hook-response/wuForecast"
#define FORECAST_PUB    "wuForecast"
#define CURRENT_RESP   "hook-response/lafayetteOpenWeather"
#define CURRENT_PUB    "lafayetteOpenWeather"

void setup() {
    // Button for testing (on pin D2)
    pinMode(D2,INPUT);
    attachInterrupt(D2, debounceInterrupt, RISING,0);
    //pinMode(7,OUTPUT);        // Can use LED attached to pin 7 for debugging

    // Set Up Particle
    RGB.control(true);          // control onboard RGB
    RGB.brightness(0);          // turn off onboard RGB
    Particle.syncTime();        // sync time (Not neccissary)
    Time.zone(-6);
    if(Time.isDST())
        Time.beginDST();
    else
        Time.endDST();

    //Set Up LEDs
    pixels.begin();             // This initializes the NeoPixel library.
    dispTemp(-99);              //-99 is off. 

    //For Debugging
    Serial.begin(9600);
}

//////////////////////////////
//    Interrupt Function    //
//     (called on touch)    //
//////////////////////////////
long debouncing_time = 300; //Debouncing Time in Milliseconds
volatile unsigned long last_micros;
void debounceInterrupt() {
    if((long)(micros() - last_micros) >= debouncing_time * 1000) {
        lampState();
        last_micros = micros();
  }
}

//////////////////////////////
//   Increment Lamp Mode    //
//////////////////////////////
void lampState(){
    lampSetting++;
    if (lampSetting>3)
        lampSetting=0;
    dispLamp();
}

//////////////////////////////
//    Update Lamp Color     //
//////////////////////////////
void dispLamp(){
    if(lampSetting==1)
        dispTemp(curTemp);
    else if(lampSetting==2)
        dispTemp(maxTemp);
    else if(lampSetting==3)
        lampMode();
    else 
        dispTemp(-99);
}

void loop() {
    dispLamp();

    //Get the weather forecast
    curTime=(int)(Time.hour()*60)+(int)Time.minute();
    // currently checking every 15 minutes
    if ((curTime-tempTime)>15 || (curTime-tempTime)<0) {
       getWeather();
       last_micros = micros();
   }
}

//////////////////////////////
//        GET WEATHER       //
//////////////////////////////
void getWeather(){
    WiFi.on();
    WiFi.connect();
    if (waitForWifi(30000)) {
        Particle.connect();
        if (waitForCloud(true, 20000)){
                Particle.process();  
                int errCount = 0;
                while(!getForecast() && errCount++ < 10)
                    delay(1000);
                errCount = 0;
                while(!getCurrent() && errCount++ < 10)
                    delay(1000);
        }
        Particle.disconnect();
    }
    WiFi.disconnect();
    WiFi.off();
}

//////////////////////////////
//       get Forecast       //
//////////////////////////////
bool forecastSuccess;
bool getForecast() {
  forecastSuccess  = false;
  Particle.publish(FORECAST_PUB);
  unsigned long wait = millis();
  //wait for subscribe to kick in or 5 secs
  while (!forecastSuccess && (millis()-wait < 5000))
    //Tells the core to check for incoming messages from particle cloud
    Particle.process();  
  if (!forecastSuccess)
    Serial.println("Weather update failed");
    else
        forecastSuccess=true;
    return forecastSuccess;
}//End of getWeather function

//////////////////////////////
//        get Current       //
//////////////////////////////
bool currentSuccess;
bool getCurrent() {
  currentSuccess  = false;
  Particle.publish(CURRENT_PUB);
  unsigned long wait = millis();
  //wait for subscribe to kick in or 5 secs
  while (!currentSuccess && (millis()-wait < 5000))
    //Tells the core to check for incoming messages from particle cloud
    Particle. process();  
  if (!currentSuccess)
    Serial.println("Weather update failed");
    else
        currentSuccess=true;
    return currentSuccess;
}//End of getWeather function


//////////////////////////////
//      currentHandler      //
//////////////////////////////
void currentHandler(const char *name, const char *data){
    String str = String(data);
    curTemp=str.toFloat();
    currentSuccess = true;
}

//////////////////////////////
//     forecastHandler      //
//////////////////////////////
void forecastHandler(const char *name, const char *data) {
    String str = String(data);
    char strBuffer[125] = "";
    str.toCharArray(strBuffer, 125); // example: "\"21~99~75~0~22~98~77~20~23~97~74~10~24~94~72~10~\""
    int forecastday1 = atoi(strtok(strBuffer, "\"~"));
    int maxtempday1 = atoi(strtok(NULL, "~"));
    int mintempday1 = atoi(strtok(NULL, "~"));
    int forecastday2 = atoi(strtok(NULL, "~"));
    int maxtempday2 = atoi(strtok(NULL, "~"));
    int mintempday2 = atoi(strtok(NULL, "~"));
    int forecastday3 = atoi(strtok(NULL, "~"));
    int maxtempday3 = atoi(strtok(NULL, "~"));
    int mintempday3 = atoi(strtok(NULL, "~"));

    if (forecastday1 == Time.day())
        maxTemp=maxtempday1;
    else if (forecastday2 == Time.day())
        maxTemp=maxtempday2;
    forecastSuccess = true;
}

void fadeLEDs(){
    for(int i=0;i<NUMPIXELS;i++){
        pixels.setPixelColor(i, pixels.Color(rVal,gVal,bVal)); 
        pixels.show(); // This sends the updated pixel color to the hardware.
    }
}

void lampMode(){
    rVal=255;
    gVal=240;
    bVal=215;
    fadeLEDs();
}

void dispTemp(int temp){
    // If I send this function -99, it means turn off the LEDs
    if (temp<=-99){
       // pixels.setPixelColor(i, pixels.Color(0,150,0)); // Moderately bright green color.
        rVal=0;
        gVal=0;
        bVal=0;
        fadeLEDs();
        return;
    }
    
    // constrain temp between 0 and 100 F
    temp=constrain(temp, 0, 100);
    
    // Map Temp to Correct Temp.
    //100 - Red        255,   0,   0
    //    - Orange
    //80  - Yellow     255, 255,   0
    //60  - Green        0, 255,   0
    //40                 0, 255, 255
    //20  - Blue         0,   0, 255
    //    - Indigo
    //0   - Violet     255,   0, 255
    // 80-100 Red Full, Green 255-0
    if (temp>=80){
        rVal=255;
        gVal=map(temp,80,100,255,0);
        bVal=0;
    }
    // 60-80 Green Full, Red 0-255
    else if (temp>=60){
        rVal=map(temp,60,80,0,255);
        gVal=255;
        bVal=0;
    }
    // 40-60 Green Full, Blue 255-0
    else if (temp>=40){
        rVal=0;
        gVal=255;
        bVal=map(temp,40,60,255,0);
    }
    // 20-40 Blue Full, Green 0-255
    else if (temp>=20){
        rVal=0;
        gVal=map(temp,20,40,0,255);
        bVal=255;
    }
    else{
        rVal=map(temp,0,20,255,0);
        gVal=0;
        bVal=255;
    }
    fadeLEDs();
}

/**
 * Attempts to make a connection via WiFi.
 * @param timeout The timeout in millis
 * @return {@code true} if the wifi was connected, false if not.
 */
bool waitForWifi(unsigned timeout) {
    unsigned start = millis();
    while (WiFi.connecting() && (millis()-start)<timeout) 
        delay(100); 

    while (!WiFi.ready() && (millis()-start)<timeout) {
        Particle.process();
        delay(1000);
    }
    return WiFi.ready();
}

/**
 * Waits for the cloud connection to be in a certain state (connected or disconnected.)
 * @param state {@code true} to wait for the cloud to be connected, else wait for the cloud
 *  to be disconnected.
 * @param timeout   The timeout in millis to wait.
 * @return {@code true} if the cloud is in the state required
 */
bool waitForCloud(bool state, unsigned timeout) {
    unsigned start = millis();
    while (Particle.connected()!=state && (millis()-start)<timeout) {
        Particle.process();
        delay(100);
    }
    return Particle.connected()==state;
}