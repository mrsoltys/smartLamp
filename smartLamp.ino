// This #include statement was automatically added by the Particle IDE.
#include "OpenWeatherMap.h"
SYSTEM_MODE(MANUAL);

//////////////////////////////
//    RGB LED OUTPUT PINS   //
//////////////////////////////
int rPin=0;
int gPin=1;
int bPin=3;

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

void setup() {
    RGB.control(true);
    //RGB.brightness(64);

    //Set RGB pins to be outputs
    pinMode(rPin,OUTPUT);
    pinMode(gPin,OUTPUT);
    pinMode(bPin,OUTPUT);
    dispTemp(-99);

    // OpenWeatherMap configuration: set the units to either IMPERIAL or METRIC
    // Which will set temperature values to either farenheit or celsius
    weather.setUnits(IMPERIAL); // Set temperature units to farenheit0
    Time.zone(-7);
    
    getWeather();
    //Particle.syncTime();
    
    //set timezone -7 for MTN time
    //-8 or 6 for DST?
    
    
    //For Debug
    Serial.begin(9600);
}

void getWeather(){
    Serial.print("Getting Weather: Turning on WiFi... ");
    WiFi.on();
    WiFi.connect();
    if (waitForWifi(30000)) { 
        Serial.print("Yes. Connecting to Cloud...");
        Particle.connect();
        if (waitForCloud(true, 20000)){
            Serial.println("Yes.");
            int errCount = 0;
            //Check the weather and breath between current and high temp
            //keep checking until successful. 
            while(!weather.current(LATITUDE, LONGITUDE) && errCount<100) {
                 delay(500);
                errCount++;
               }
            curTemp=weather.temperature();
            //Serial.print("Current Temp: ");
            //Serial.print(curTemp);
            while(!weather.daily(LATITUDE, LONGITUDE, 2)  && errCount<100){
                delay(500);
                errCount++;
            }
            maxTemp=weather.maxTemperature();
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
    Serial.print("Current: ");Serial.println(curTime);
    Serial.print("Alarm: ");Serial.println(almTime);
    Serial.print("Temp: ");Serial.print(tempTime);
    Serial.print(" current: ");Serial.print(curTemp);
    Serial.print(" max: ");Serial.println(maxTemp);

    if(curTime<almTime || curTime>bedTime){
        dispTemp(-99);
        return;
    }
    else if(curTime>=almTime+almLength/60){
        if ((curTime-tempTime)>.25 || (curTime-tempTime)<0)
           getWeather();
        
        dispTemp(curTemp);
        for (int i=curTemp;i<=maxTemp;i++){
            dispTemp(i);
            delay(100);
        }
        delay(2000);
        dispTemp(maxTemp);
        for (int i=maxTemp;i>=curTemp;i--){
            dispTemp(i);
            delay(100);
        }
        delay(2000);
    }
    else{
        sunRise();
    }
    //API limit is 12,000 requests per minute :)
    
}

void dispTemp(int temp){
    // If I send this function -99, it means turn off the LEDs
    if (temp<=-99) {
        analogWrite(rPin, 0);
        analogWrite(gPin, 0);
        analogWrite(bPin, 0);
        return;
    }
    
    // constrain temp between 10 and 100 F
    temp=constrain(temp, 0, 100);
    
    if (temp>66){
        analogWrite(rPin, map(temp,66,100,0,255));
        analogWrite(gPin, map(temp,66,100,255,0));
        analogWrite(bPin,0);
    }
    else if (temp>33){
        analogWrite(rPin, 0);
        analogWrite(gPin, map(temp,33,66,0,255));
        analogWrite(bPin, map(temp,33,66,255,0));
    }
    else{
        analogWrite(rPin, map(temp,0,33,255,0));
        analogWrite(gPin,0);
        analogWrite(bPin,255);
    }
}

void sunRise(){
    // loosley based on charts from http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
    int delayT=(float)almLength*60*1000/999;
    
    analogWrite(rPin,0);
    analogWrite(gPin,0);
    analogWrite(bPin,0);
    
    for (int i=0;i<333;i++){
        analogWrite(rPin,127*(float)i/333);
        delay(delayT);
    }
    for (int i=0;i<333;i++){
        analogWrite(rPin,127+127*(float)i/333);
        analogWrite(bPin,127*(float)i/333);
        delay(delayT);
    }
    for (int i=0;i<333;i++){
        analogWrite(gPin,127*(float)i/333);
        delay(delayT);
    }
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