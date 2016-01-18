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
#include "OpenWeatherMap.h"
#include <neopixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

//Set to manual for full control
SYSTEM_MODE(MANUAL);

//////////////////////////////
//    RGB LED VALUES        //
//////////////////////////////
int rVal=0;
int gVal=0;
int bVal=0;

int lampSetting=0;
int buttonState=0;

//////////////////////////////
//    VARS FOR NEOPIXELS    //
//////////////////////////////
// Which pin on the Arduino is connected to the NeoPixels?
#define PIN            3
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      12
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN);

//////////////////////////////
// OpenWeatherMap Variables //
//////////////////////////////
// OpenWeathermap API key. Get one (for free) by signing up here:
//                   http://home.openweathermap.org/users/sign_up
const String OPENWEATHER_API_KEY = "ab5f3051ea5ef01d114b33dadee1b99d"; 
// Forecast location declaration (currently set for Niwot, CO - SparkFun HQ!):
const float LATITUDE = 39.999; // Weather forecast location's latitude
const float LONGITUDE = -105.105; // Weather forecast location's longitude
const String location = "lat=" + String(LATITUDE) + "&lon=" + String(LONGITUDE);
const String weatherServer = "api.openweathermap.org";
// Create an OpenWeather object, giving it our API key as the parameter:
OpenWeather weather(OPENWEATHER_API_KEY);
#define forecastDays    1;
int maxTemp, curTemp;
TCPClient client;

//////////////////////////////
//      Time Variables      //
//////////////////////////////
float curTime;
float almTime=6.0; 
float almLength=1; 
float bedTime=12+9.5;
float tempTime;

void setup() {
    // Button for testing (on pin D2)
    pinMode(D2,INPUT);
    //pinMode(7,OUTPUT);        // Can use LED attached to pin 7 for debugging

    // Set Up Particle
    RGB.control(true);          // control onboard RGB
    RGB.brightness(0);          // turn off onboard RGB
    Time.zone(-7);              // set timezone to MST. Note: Unsure how to handle DST
    Particle.syncTime();        // sync time (Not neccissary)

    //Set Up LEDs
    pixels.begin();             // This initializes the NeoPixel library.
    dispTemp(-99);              //-99 is off. 

    // OpenWeatherMap configuration: set the units to either IMPERIAL or METRIC
    // Which will set temperature values to either farenheit or celsius
    weather.setUnits(IMPERIAL); // Set temperature units to farenheit0
    getWeather();

    //For Debugging
    Serial.begin(9600);
}

int getCurrentForecast(){
    String rsp;
    int timeout = 1500;

    if (client.connect(weatherServer, 80)) {
      client.print("GET /data/2.5/weather?");
      client.print(location);
      client.print("&mode=xml");
      client.print("&APPID=" + OPENWEATHER_API_KEY);
      client.print("&units=imperial"); // Change imperial to metric for celsius (delete this line for kelvin)
      client.println(" HTTP/1.1");
      client.println("Host: " + weatherServer);
      client.println("Content-length: 0");
      client.println();
    }
    else {
      Serial.println("Error Connecting to Server");
      return 0;
    }
      
    while ((!client.available()) && (timeout-- > 0))
        delay(1);
    if (timeout <= 0) {
        Serial.println("Server Timeout");
        return 0;
    }

    while (client.available()) {
        char c = client.read();
        rsp += c;
    }
    
    curTemp=parseXML(&rsp, "temperature", "value").toFloat();

    // For Debugging...
    Serial.println("Response: ");
    Serial.println(rsp);

    Serial.println(curTemp);
    client.stop();
    return 1;
}

void getWeather(){
    char publishString[4];
    WiFi.on();
    WiFi.connect();
    if (waitForWifi(30000)) {
        Particle.connect();
        if (waitForCloud(true, 20000)){
            Serial.println("Testing new code");
            int errCount = 0;
            while(!getCurrentForecast() && errCount++ < 10){
                delay(1000);
            }
            if(errCount<10){
                sprintf(publishString,"%d",curTemp);
                Particle.publish("Current Temperature",publishString);
            }
            else {
                Particle.publish("Error Getting Current Temperature");
            }

            Serial.println("Testing Old Code");

            
           /* if(errCount<10){
                sprintf(publishString,"%d",curTemp);
                Particle.publish("Max Temperature",publishString);
            }
            else {
                Particle.publish("Error Getting Max Temperature");
            }*/
            //Serial.print("Current Temp: ");
            //Serial.print(curTemp);
    
            //Serial.print("  High: ");
            //Serial.println(maxTemp);
            curTime=(float)Time.hour()+(float)Time.minute()/60;
            tempTime=curTime;
        }
        Particle.disconnect();
    }
    WiFi.disconnect();
    WiFi.off();
}

void loop() {
    //Get the weather forecast
    curTime=(float)Time.hour()+(float)Time.minute()/60;
    //Serial.print("Current: ");Serial.println(curTime);
    //Serial.print("Alarm: ");Serial.println(almTime);
    //Serial.print("Temp: ");Serial.print(tempTime);
    //Serial.print(" current: ");Serial.print(curTemp);
    //Serial.print(" max: ");Serial.println(maxTemp);

    // if(curTime<almTime || curTime>bedTime){
    //     dispTemp(-99);
    //     return;
    // }
    // else if(curTime>=almTime+almLength/60){
    if ((curTime-tempTime)>.25 || (curTime-tempTime)<0) {
       getWeather();
   }
        
    //    dispTemp(curTemp);
    //     for (int i=curTemp;i<=maxTemp;i++){
    //         dispTemp(i);
    //         delay(100);
    //     }
    //     delay(2000);
    //     dispTemp(maxTemp);
    //     for (int i=maxTemp;i>=curTemp;i--){
    //         dispTemp(i);
    //         delay(100);
    //     }
    //     delay(2000);
    // }
    // else{
    //     sunRise();
    // }
    //API limit is 12,000 requests per minute :)
   
    buttonState=digitalRead(2);
    if (buttonState==LOW){
        lampSetting++;
        if(lampSetting==0)
            dispTemp(-99);
        else if(lampSetting==1)
            dispTemp(curTemp);
        else if(lampSetting==2)
            dispTemp(maxTemp);
        else if(lampSetting==3)
            lampMode();
        else {
            dispTemp(-99);
            lampSetting=0;
        }
        delay(100);
    }
}

void fadeLEDs(){
    for(int i=0;i<NUMPIXELS;i++){
        pixels.setPixelColor(i, pixels.Color(rVal,gVal,bVal)); 
        pixels.show(); // This sends the updated pixel color to the hardware.
    }
}

void lampMode(){
    rVal=255;
    gVal=255;
    bVal=225;
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
    if (temp>66){
        rVal=map(temp,66,100,0,255);
        gVal=map(temp,66,100,255,0);
        bVal=0;
    }
    else if (temp>33){
        rVal=0;
        gVal=map(temp,33,66,0,255);
        bVal=map(temp,33,66,255,0);
    }
    else{
        rVal=map(temp,0,33,255,0);
        gVal=0;
        bVal=255;
    }
    fadeLEDs();
}

void sunRise(){
    // loosley based on charts from http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
    int delayT=(float)almLength*60*1000/999;
    
    rVal=0;
    gVal=0;
    bVal=0;
    
    // for (int i=0;i<333;i++){
    //     analogWrite(rPin,127*(float)i/333);
    //     delay(delayT);
    // }
    // for (int i=0;i<333;i++){
    //     analogWrite(rPin,127+127*(float)i/333);
    //     analogWrite(bPin,127*(float)i/333);
    //     delay(delayT);
    // }
    // for (int i=0;i<333;i++){
    //     analogWrite(gPin,127*(float)i/333);
    //     delay(delayT);
    // }
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

String parseXML(String * search, String tag, String attribute) {
    int tagStart = tagIndex(search, tag, 1);
    int tagEnd = tagIndex(search, tag, 0);
    if (tagStart >= 0){
        int attributeStart = search->indexOf(attribute, tagStart);
        if ((attributeStart >= 0) && (attributeStart < tagEnd)){ // Make sure we don't get value of another key
            attributeStart = search->indexOf("\"", attributeStart);
            if (attributeStart >= 0){
                int attributeEnd = search->indexOf("\"", attributeStart + 1);
                if (attributeEnd >= 0){
                    return search->substring(attributeStart + 1, attributeEnd);
                }
            }
        }
    }
    return NULL;
}

int tagIndex(String * xml, String tag, bool start){
    String fullTag = "<";
    if (start){
        fullTag += tag;
        fullTag += ' '; // Look for a space after the tag name
    }
    else{
        fullTag += '/';
        fullTag += tag;
        fullTag += '>';
    }
    return xml->indexOf(fullTag);
}