#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"


/************************ wifi setup ************************/
#define WIFI_SSID       "Saad El Desoky Office"
#define WIFI_PASSWORD   "!#SAAD@saad@12345#!"

/************************ MQTT setup ************************/
#define IO_SERVER       "io.adafruit.com"
#define IO_SERVERPORT   1883
#define IO_USERNAME     "AbdelrahmanEldesouky"
#define IO_KEY          "aio_gVsC40dHezneCN3UspjPLa8h16u1"

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, IO_SERVER, IO_SERVERPORT, IO_USERNAME, IO_KEY);
Adafruit_MQTT_Publish TMP = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/TMP");
Adafruit_MQTT_Publish HUM = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/HUM");
Adafruit_MQTT_Publish PIR = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/PIR");
Adafruit_MQTT_Publish LDR = Adafruit_MQTT_Publish(&mqtt, IO_USERNAME "/feeds/LDR");
Adafruit_MQTT_Subscribe AC1 = Adafruit_MQTT_Subscribe(&mqtt, IO_USERNAME "/feeds/AC1", MQTT_QOS_1);
Adafruit_MQTT_Subscribe AC2 = Adafruit_MQTT_Subscribe(&mqtt, IO_USERNAME "/feeds/AC2", MQTT_QOS_1);
Adafruit_MQTT_Subscribe AC3 = Adafruit_MQTT_Subscribe(&mqtt, IO_USERNAME "/feeds/AC3", MQTT_QOS_1);
Adafruit_MQTT_Subscribe AC4 = Adafruit_MQTT_Subscribe(&mqtt, IO_USERNAME "/feeds/AC4", MQTT_QOS_1);


/************************ pin connection ************************/
#define MCU_PIR_EN 23
#define MCU_PIR_S 22
#define MCU_LDR_EN 32
#define MCU_LDR_S 33
#define MCU_TMP_EN 27
#define MCU_TMP_S 13
#define MCU_AC1 19
#define MCU_AC2 18
#define MCU_AC3 25
#define MCU_AC4 26


#define TMP_TYPE DHT11
DHT dht(MCU_TMP_S, TMP_TYPE);

bool isConnected{false};
bool isMoved{false};
bool isCallBack {false} ; 

long int NOW_t ; 
long int LDR_t ;
long int TMP_t ; 
long int PIR_t ; 

// connect to adafruit io via MQTT
void connect() {
  Serial.print(F("Connecting to Adafruit IO... "));
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) 
  {
    switch (ret) 
    {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(10000);
  }
  Serial.println(F("Adafruit IO Connected!"));
}

void read_TMP_HUM()
{
  if ((millis() - TMP_t) > 10000)
  {
    /************************ TMP Sensor ************************/
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) 
    {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
    
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C "));
    
    if (! TMP.publish(t)) //Publish to Adafruit
    {                     
      Serial.println(F("Failed"));
    } 
    if (! HUM.publish(h)) //Publish to Adafruit
    {                     
      Serial.println(F("Failed"));
    }
    else 
    {
      Serial.println(F("Sent!"));
    }

    TMP_t = millis() ;
  }
}
void read_LDR()
{
  /************************ Light Sensor ************************/
  if ((millis() - LDR_t) > 10000)
  {
    // Reading Light Sensor
    double light = ((double)analogRead(MCU_LDR_S)/ 4096.0) * 100.0 ; 
  
    Serial.print(F("Light Percentage: "));
    Serial.print(light);
    Serial.print(F("%  "));
  
    if (! LDR.publish(light)) //Publish to Adafruit
    {                     
      Serial.println(F("Failed"));
    } 
    else 
    {
      Serial.println(F("Sent!"));
    }

    LDR_t = millis() ;
  }
}
void callBack_PIR()
{
  isMoved = true ; 
  isCallBack = true ; 
  PIR_t = millis() ; 
}

void read_PIR()
{
  /************************ Motion Sensor ************************/
  
  if (((millis() - PIR_t) > 3000) && isCallBack)
  {
    Serial.print(F("Motion detected!  "));
    
    if (! PIR.publish(1)) //Publish to Adafruit
    {                     
      Serial.println(F("Failed"));
    } 
    else 
    {
      Serial.println(F("Sent!"));
    }
    
    isCallBack = false ;
  }
  else if (((millis() - PIR_t) > 10000) && isMoved)
  {
    Serial.print(F("Motion stoped...  "));

    if (! PIR.publish(0)) //Publish to Adafruit
    {                     
      Serial.println(F("Failed"));
    } 
    else 
    {
      Serial.println(F("Sent!"));
    }
    
    isMoved = false ;
  }
}

void AC1_callBack(char *data, uint16_t len)
{
  Serial.print("Hey we're in a AC1 callback, the button value is: ");
  Serial.println(data);

  if (*data == '1')
  {
    digitalWrite(MCU_AC1, HIGH); 
  }
  else 
  {
    digitalWrite(MCU_AC1, LOW); 
  }
}

void AC2_callBack(char *data, uint16_t len)
{
  Serial.print("Hey we're in a AC2 callback, the button value is: ");
  Serial.println(data);

  if (*data == '1')
  {
    digitalWrite(MCU_AC2, HIGH); 
  }
  else 
  {
    digitalWrite(MCU_AC2, LOW); 
  }
}

void AC3_callBack(char *data, uint16_t len)
{
  Serial.print("Hey we're in a AC3 callback, the button value is: ");
  Serial.println(data);

  if (*data == '1')
  {
    digitalWrite(MCU_AC3, HIGH); 
  }
  else 
  {
    digitalWrite(MCU_AC3, LOW); 
  }
}

void AC4_callBack(char *data, uint16_t len)
{
  Serial.print("Hey we're in a AC4 callback, the button value is: ");
  Serial.println(data);

  if (*data == '1')
  {
    digitalWrite(MCU_AC4, HIGH); 
  }
  else 
  {
    digitalWrite(MCU_AC4, LOW); 
  }
}

void setup() 
{
  Serial.begin(9600);
  
  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("starting");

  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.println(".");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(1000);
    isConnected = false;
  }

  pinMode(MCU_TMP_EN, OUTPUT) ;
  digitalWrite(MCU_TMP_EN, HIGH) ; 
  pinMode(MCU_LDR_EN, OUTPUT) ;
  digitalWrite(MCU_LDR_EN, HIGH) ; 
  pinMode(MCU_PIR_EN, OUTPUT) ;
  digitalWrite(MCU_PIR_EN, HIGH) ;
  pinMode(MCU_PIR_S, INPUT);
  pinMode(MCU_AC1, OUTPUT) ;
  pinMode(MCU_AC2, OUTPUT) ;
  pinMode(MCU_AC3, OUTPUT) ;
  pinMode(MCU_AC4, OUTPUT) ;
  delay (1000);

  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(MCU_PIR_S), callBack_PIR, RISING);

  AC1.setCallback(AC1_callBack);
  mqtt.subscribe(&AC1);

  AC2.setCallback(AC2_callBack);
  mqtt.subscribe(&AC2);

  AC3.setCallback(AC3_callBack);
  mqtt.subscribe(&AC3);

  AC4.setCallback(AC4_callBack);
  mqtt.subscribe(&AC4);

  dht.begin();

  LDR_t = millis() ;
  TMP_t = millis() ;  
}

void loop() 
{
  /************************ wifi connection ************************/
  if ((WiFi.status() == WL_CONNECTED) && !isConnected) 
  {
    Serial.println("Connected");
    digitalWrite(LED_BUILTIN, HIGH);
    isConnected = true;

    connect();
  }
  else if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("reconected...");
    while (WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      Serial.println(".");
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(1000);
    }
   isConnected = false;
  }

  /************************ mqtt connection ************************/
  if(! mqtt.ping(3)) 
  {
    // reconnect to adafruit io
    if(! mqtt.connected())
    connect();
  }

  read_TMP_HUM() ;
  read_LDR() ;
  read_PIR() ;
  mqtt.processPackets(1000);
}
