//#include <CapacitiveSensor.h>
#include <neopixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

//Set to manual for full control
SYSTEM_MODE(MANUAL);

//CapacitiveSensor   cs_4_6 = CapacitiveSensor(4,6);        // 1 megohm resistor between pins 4 & 6, pin 6 is sensor pin, add wire, foil

#define PIN            2
#define NUMPIXELS      12
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN);

bool pin8state=LOW;
bool pin9state=LOW;
bool pin7state=LOW;
int mode =0;

void setup() {
 //cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example
 Serial.begin(9600);
 pixels.begin(); // This initializes the NeoPixel library.
 pixels.setBrightness(255);
}

void loop() {
		if(mode==7)	{
            mode=0;
        }
		else {
            mode++;
        }
		writeMode();
        delay(500);			
}

void writeMode(){
int r=0;
int g=0;
int b=0;
	switch (mode) {
	  case 0:    
	  	r=0;
	  	g=0;
      	b=0;
	  case 1:    
	  	r=255;
	  	g=0;
      	b=0;
      	break;
   	  case 2:    
		r=0;
	  	g=255;
      	b=0;
      	break;        
      case 3:    
		r=0;
	  	g=0;
      	b=255;
      	break;
      case 4:    
		r=255;
	  	g=255;
      	b=0;
      	break;
      case 5:    
		r=255;
	  	g=0;
      	b=255;
      	break;   	
      case 6:    
		r=0;
	  	g=255;
      	b=255;
      	break;
      case 7:    
		r=255;
	  	g=255;
      	b=255;
      	break;   	
	}
	for(int i=0;i<NUMPIXELS;i++){
    	// pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    	pixels.setPixelColor(i, pixels.Color(r,g,b)); // Moderately bright green color.
    	pixels.show(); // This sends the updated pixel color to the hardware.
    	delay(50); // Delay for a period of time (in milliseconds).
  }
}
