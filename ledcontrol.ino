/*************************************************** 
  This is an example for our Adafruit 16-channel PWM & Servo driver
  GPIO test - this will set a pin high/low

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/815

  These drivers use I2C to communicate, 2 pins are required to  
  interface.

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h> 
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
//#include <WiFiManager.h>

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>


ESP8266WebServer server(80);
// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);
// you can also call it with a different address and I2C interface
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40, Wire);

//const char* ssid   = "Univision_7079";
//const char* password = "6c8d679cd1ba";

/*
const char* ssid   = "UA_24";
const char* password = "#UAlliance2020";
#define STATICIP 1
*/
const char* ssid   = "AndroidAP";
const char* password = "ganzorig";
int timer_start=0;
int timer_value=0;
int i;

void setup() {
  Serial.begin(9600);
  delay(10);   
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  //WiFi.begin(ssid, NULL);
  #ifdef STATICIP
    IPAddress ip(192,168,10,230);  
    IPAddress gateway(192,168,10,1);
    IPAddress subnet(255,255,255,0);
    WiFi.config(ip,gateway,subnet);
  #endif
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  pwm.begin();
  pwm.setPWMFreq(500);  // Set to whatever you like, we don't use it in this demo!

  // if you want to really speed stuff up, you can go into 'fast 400khz I2C' mode
  // some i2c devices dont like this so much so if you're sharing the bus, watch
  // out for this!
  Wire.setClock(400000);

  server.on("/", HTTP_GET, handleRoot);     // Call the 'handleRoot' function when a client requests URI "/"
  server.on("/led", HTTP_POST, handleLed);  // Call the 'handleLED' function when a POST request is made to URI "/LED"
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"

  server.begin();                           // Actually start the server


  /*
  for (i=0;i<15;i++)
  {
    pwm.setPWM(i,0,4090);  
  } 
  */  
  
}
int brightness=0;

int count=0;
int state=0, togglestate=0;
int tbrightness=0;
int tcnt=0;
int toggleTimes=0;
int ledvalue = 0;

void loop() {
  server.handleClient(); 
  if (timer_start==1)
  {
    led_toggle();        
    if(toggleTimes>5)
    {
       timer_start=0;
       // ALL led on
       /*for (i=0;i<15;i++)
       {
          pwm.setPWM(i,0,4090);  
       } 
       */
    }
  }
  else
  {
    led_animation();
  }
  
}


int animation_state=0;

int cycle_index=0;
int cycle_brightness=4090;
int cycle_cnt=0;
int cycle_end=0;


void led_animation()
{
  switch(animation_state)
  {
    case 0:
           if (cycle_end>1)
           {
              cycle_end=0;
              animation_state=1;
           }
           else
           {
              led_cycle_on();              
           }
           break;
    case 1:
           if (cycle_end>5)
           {
              cycle_end=0;
              animation_state=2;
           }
           else
           {
              led_fadeinout();              
           }
           break;
    case 2:
           if (cycle_end>4)
           {
              cycle_end=0;
              animation_state=0;
           }
           else
           {
              one_led_at_time();              
           }
           break;                     
  }
}



void led_light(int val,int leds)
{
  int led2on = leds;
  for (i=0;i<15;i++)
  {
    if (led2on&(0x01))
    {
      pwm.setPWM(i,0,val);
    }
    else
    {
      pwm.setPWM(i,0,0);
    }  
    led2on = led2on>>1;
  }
}

  

void led_toggle()
{
    switch(togglestate)
    {
      case 0: // brightness up
            if(tbrightness<4095)
            {    
              delay(1);
              tcnt++;
              if (tcnt>5)
              {
                led_light(tbrightness,ledvalue);                              
                tbrightness+=20;
                tcnt = 0;
              }    
            }
            else
            {
              tcnt = 0;              
              togglestate=1;
              tbrightness = 4090;
            }
            break;
      case 1: // stay at full brightness
            if (tcnt<1000)
            {
              delay(1);
              tcnt++;               
            }
            else
            {
              tcnt = 0;              
              togglestate=2;
            }
            break;
      case 2: // brightness down
            if(tbrightness>0)
            {
              delay(1);
              tcnt++;
              if (tcnt>5)
              {
                led_light(tbrightness,ledvalue);                              
                tbrightness-=20;
                tcnt = 0;                
              }   
            }
            else
            {
              tcnt = 0;              
              togglestate=0;
              tbrightness = 0;
              for (i=0;i<15;i++)
              {
                pwm.setPWM(i,0,tbrightness);  
              }  
            }
            toggleTimes++;     
            break;     
    }
      
}

void led_cycle_on()
{
  if (cycle_index<16)
  { 
    delay(1);
    cycle_cnt++;
    if (cycle_cnt>500)
    {
      pwm.setPWM(cycle_index,0,cycle_brightness);  
      cycle_index++;
    }
  }
  else
  {
    cycle_index=0;
    cycle_end++;
  }  
}


void one_led_at_time()
{
  if (cycle_index<16)
  { 
    delay(1);
    cycle_cnt++;
    if (cycle_cnt>200)
    {
      pwm.setPWM(cycle_index,0,cycle_brightness);  //turn on led
      pwm.setPWM((cycle_index-1)&0x0F,0,0);        //turn off previous led
      cycle_index++;
    }
  }
  else
  {
    cycle_index=0;
    cycle_end++;
  } 
}



void led_fadeinout()
{
  switch(state){
    case 0:  
            if(brightness<4095)
            {    
              delay(1);
              count++;
              if (count>5)
              {
                for (i=0;i<15;i++)
                {
                  pwm.setPWM(i,0,brightness);  
                }                               
                brightness+=20;
                count = 0;                
              }    
            }
            else
            {
              count = 0;              
              state=1;
              brightness = 4090;
            }
            break;
     case 1:
            if (count<2000)
            {
              delay(1);
              count++;               
            }
            else
            {
              count = 0;              
              state=2;
            }
            break;
     case 2:                        
            if(brightness>0)
            {
              delay(1);
              count++;
              if (count>10)
              {
                for (i=0;i<15;i++)
                {
                  pwm.setPWM(i,0,brightness);  
                }  
                brightness-=20;
                count = 0;                
              }   
            }
            else
            {
              count = 0;              
              state=0;
              brightness = 0;
              for (i=0;i<15;i++)
              {
                pwm.setPWM(i,0,brightness);  
              }
              cycle_end++;  
            }            
            break;
  }    
}



void handleRoot() {                         // When URI / is requested, send a web page with a button to toggle the LED
  server.send(200, "text/html", "<form action=\"/LED\" method=\"POST\"><input type=\"submit\" value=\"Toggle LED\"></form>");
}


void handleLed() {                          // If a POST request is made to URI /LED
 if (server.hasArg("plain")== false){ //Check if body received
            server.send(200, "text/plain", "Body not received");
            return;
 
      }
    Serial.println("new request");  
    Serial.println(server.arg("plain"));
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    if (error)
    return;
    ledvalue = doc["ledvalue"]; // 5   

    timer_start=1;
    toggleTimes=0;
   
    server.send(200, "text/plain", "request complete");
    //pwm.setPWM(0,pwmvalue,0);
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
