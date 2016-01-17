//Include Approprate Libraries
#include "OpenWeatherMap.h"
#include <neopixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

//Set to manual for full control
SYSTEM_MODE(MANUAL);

int buttonPin=D4;

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
const float LATITUDE = 39.9986714; // Weather forecast location's latitude
const float LONGITUDE = -105.1054522; // Weather forecast location's longitude
// Create an OpenWeather object, giving it our API key as the parameter:
OpenWeather weather(OPENWEATHER_API_KEY);

float curTime;
float almTime=6.0; 
float almLength=1; 
float bedTime=12+9.5;
float tempTime;

int maxTemp, curTemp;
unsigned int forecastDays=1;

void ISR(){
    digitalWrite(7,HIGH);
    delay(100);
    digitalWrite(7,LOW);
}

void setup() {
    pinMode(D2,INPUT);
    //attachInterrupt(D2,ISR,FALLING);
    pinMode(7,OUTPUT);

    // Set Up Particle
    RGB.control(true);
    RGB.brightness(0);
    Time.zone(-7);
    Particle.syncTime();

    //Set Up LEDs
    pixels.begin(); // This initializes the NeoPixel library.
    dispTemp(-99); //-99 is off. 

    // OpenWeatherMap configuration: set the units to either IMPERIAL or METRIC
    // Which will set temperature values to either farenheit or celsius
    weather.setUnits(IMPERIAL); // Set temperature units to farenheit0
    getWeather();
    //set timezone -7 for MTN time
    //-8 or 6 for DST?
    
    //For Debug
    Serial.begin(9600);
}

void getWeather(){
    char publishString[4];
    //Serial.print("Getting Weather: Turning on WiFi... ");
    WiFi.on();
    WiFi.connect();
    if (waitForWifi(30000)) { 
        //Serial.print("Sucess. Connecting to Cloud...");
        Particle.connect();
        if (waitForCloud(true, 20000)){
            //Serial.println("Success.");
            int errCount = 0;
/*            while(!weather.daily(LATITUDE, LONGITUDE, forecastDays)  && errCount<10){
                delay(1000);
                errCount++;
                Serial.print(errCount);Serial.print("\t");
            }
            maxTemp=weather.maxTemperature();*/

            //Check the weather and breath between current and high temp
            //keep checking until successful. 

            /*if(errCount<10){
                sprintf(publishString,"%d",curTemp);
                Particle.publish("Current Temperature",publishString);
            }
            else {
                Particle.publish("Error Getting Current Temperature");
            }*/
            weather.connectionTest(LATITUDE, LONGITUDE);
TCPClient client;
client.connect("www.google.com", 80);
if (client.connected())
{
    IPAddress clientIP = client.remoteIP();
    Serial.println(clientIP);
    // IPAddress equals whatever www.google.com resolves to
}
client.stop();

            while(!weather.current(LATITUDE, LONGITUDE) && errCount<10) {
                delay(1000);
                errCount++;
                Serial.print(errCount);Serial.print("\t");
               }
            curTemp=weather.temperature();

client.connect("www.google.com", 80);
if (client.connected())
{
    IPAddress clientIP = client.remoteIP();
    Serial.println(clientIP);
    // IPAddress equals whatever www.google.com resolves to
}
client.stop();

            delay(1500);
            errCount=0;
            while(!weather.current(LATITUDE, LONGITUDE) && errCount<10) {
                delay(1000);
                errCount++;
                Serial.print(errCount);Serial.print("\t");
               }
            curTemp=weather.temperature();

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
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        pixels.setPixelColor(i, pixels.Color(rVal,gVal,bVal)); // Moderately bright green color.
        pixels.show(); // This sends the updated pixel color to the hardware.
        delay(50); // Delay for a period of time (in milliseconds).
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
    if (temp<=-99) {
       // pixels.setPixelColor(i, pixels.Color(0,150,0)); // Moderately bright green color.
        rVal=0;
        gVal=0;
        bVal=0;
        fadeLEDs();
        return;
    }
    
    // constrain temp between 10 and 100 F
    temp=constrain(temp, 0, 100);
    
    //If Temperature is greater than 66 map:
    // 66 R=0;G=255;
    // 100 R=255;G=0;
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