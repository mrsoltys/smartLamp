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

TCPClient client;

//////////////////////////////
//      Time Variables      //
//////////////////////////////
int curTime;
int tempTime= 32767;
int maxTemp, curTemp;

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

    //For Debugging
    Serial.begin(9600);
}

void loop() {
    //Get the weather forecast
    curTime=(int)(Time.hour()*60)+(int)Time.minute();
    // currently checking every 9 minutes
    if ((curTime-tempTime)>9 || (curTime-tempTime)<0) {
       getWeather();
   }
   
    byte buttonState=0;
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
        delay(500);
    }
}

int getCurrentMaxTemp(){
    client.flush();
    String rsp;
    
    client.print("GET /data/2.5/weather?");
    client.print("lat=39.999&lon=-105.105");
    client.print("&mode=xml");
    client.print("&APPID=ab5f3051ea5ef01d114b33dadee1b99d");
    client.print("&units=imperial"); // Change imperial to metric for celsius (delete this line for kelvin)
    client.println(" HTTP/1.1");
    client.println("Host: api.openweathermap.org");
    client.println("Content-length: 0");
    client.println();

    unsigned long timeout=millis(); 
    while ((!client.available()) && (millis()-timeout < 5000)) {}
    if (!client.available()) {
        //Serial.println("Server Timeout");
        return 0;
    }
    while (client.available()) {
        char c = client.read();
        rsp += c;
    }
    curTemp=parseXML(&rsp, "temperature", "value").toInt();
    //Serial.print("curTemp: ");Serial.println(curTemp);
    // For Debugging...
    //Serial.println("Response: ");
    //Serial.println(rsp);

    rsp=NULL;
    client.print("GET /data/2.5/forecast/daily?");
    client.print("lat=39.999&lon=-105.105");
    client.print("&cnt=" + String(0));
    client.print("&mode=xml");
    client.print("&APPID=ab5f3051ea5ef01d114b33dadee1b99d");
    client.print("&units=imperial"); // Change imperial to metric for celsius (delete this line for kelvin)
    client.println(" HTTP/1.1");
    client.println("Host: api.openweathermap.org");
    client.println("Content-length: 0");
    client.println();
  
    ///////////////////////////////////////
    // Wait for Response, Store in Array //
    ///////////////////////////////////////
    timeout=millis(); 
    while ((!client.available()) && (millis()-timeout < 5000)) {}
    if (!client.available()) {
        //Serial.println("Server Timeout");
        return 0; 
    }
    timeout=millis();
    while (client.available() || (millis()-timeout < 500)) {
        if(client.available()) {
            rsp += (char)client.read();
        }
    }

    //Serial.println("Response: ");
    Serial.println(rsp);
  
    maxTemp=parseXML(&rsp, "temperature", "max").toInt();
    Serial.print("maxTemp: ");Serial.println(maxTemp);
    curTime=(int)(Time.hour()*60)+(int)Time.minute();
    tempTime=curTime;
    Serial.print("tempTime: ");Serial.println(tempTime);
    return 1;
}

void getWeather(){
    Serial.print("curTime: ");Serial.println(curTime);
    char publishString[4];
    WiFi.on();
    WiFi.connect();
    if (waitForWifi(30000)) {
        Particle.connect();
        if (waitForCloud(true, 20000)){
            if (client.connect("api.openweathermap.org", 80)) {
                int errCount = 0;
                while(!getCurrentMaxTemp() && errCount++ < 10){
                    delay(1000);
                }
            }
            else {
                //Serial.println("Error Connecting to Server");
            }
            client.stop();
        }
        Particle.disconnect();
    }
    WiFi.disconnect();
    WiFi.off();
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