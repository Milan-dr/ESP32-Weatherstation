
#include <BH1750.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>
#include "discord.h"
#include <PubSubClient.h>
#include <ESPAsyncWebServer.h>
#include <ESPDash.h>
#include <WiFi.h>
#define channelID 1967357
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  1800        /* Time ESP32 will go to sleep (in seconds) */

const int tempPin = 4;

const char mqttUserName[] = ""; 
const char clientID[] = "";
const char mqttPass[] = "";

const char * PROGMEM thingspeak_ca_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n" \
"ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n" \
"MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n" \
"LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n" \
"RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n" \
"+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n" \
"PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\n" \
"xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\n" \
"Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\n" \
"hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\n" \
"EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\n" \
"MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\n" \
"FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\n" \
"nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\n" \
"eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\n" \
"hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\n" \
"Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n" \
"vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\n" \
"+OkuE6N36B9K\n" \
"-----END CERTIFICATE-----\n";

#ifdef USESECUREMQTT
  #include <WiFiClientSecure.h>
  #define mqttPort 8883
  WiFiClientSecure client; 
#else
  #define mqttPort 1883
  WiFiClient client;
#endif

const char* Server = "mqtt3.thingspeak.com";
int status = WL_IDLE_STATUS; 
long lastPublishMillis = 0;
int connectionDelay = 1;
int updateInterval = 15;
PubSubClient mqttClient( client );

// Function to handle messages from MQTT subscription.
void mqttSubscriptionCallback( char* topic, byte* payload, unsigned int length ) {
  // Print the details of the message that was received to the serial monitor.
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Subscribe to ThingSpeak channel for updates.
void mqttSubscribe( long subChannelID ){
  String myTopic = "channels/"+String( subChannelID )+"/subscribe";
  mqttClient.subscribe(myTopic.c_str());
}

// Publish messages to a ThingSpeak channel.
void mqttPublish(long pubChannelID, String message) {
  String topicString ="channels/" + String( pubChannelID ) + "/publish";
  mqttClient.publish( topicString.c_str(), message.c_str() );
}

void mqttConnect() {
  // Loop until connected.
  while ( !mqttClient.connected() )
  {
    // Connect to the MQTT broker.
    if ( mqttClient.connect( clientID, mqttUserName, mqttPass ) ) {
      Serial.print( "MQTT to " );
      Serial.print( Server );
      Serial.print (" at port ");
      Serial.print( mqttPort );
      Serial.println( " successful." );
    } else {
      Serial.print( "MQTT connection failed, rc = " );
      // See https://pubsubclient.knolleary.net/api.html#state for the failure code explanation.
      Serial.print( mqttClient.state() );
      Serial.println( " Will try again in a few seconds" );
      delay( connectionDelay*1000 );
    }
  }
}

AsyncWebServer server(80);
ESPDash dashboard(&server); 
 
Card light(&dashboard, GENERIC_CARD, "Light", "lx");
Card temperature(&dashboard, TEMPERATURE_CARD, "Temperature", "°C");
Card pressure(&dashboard, GENERIC_CARD, "Pressure", "hPa");
Card approxaltitude(&dashboard, GENERIC_CARD, "Approx altitude", "m");

BH1750 lightMeter;
Adafruit_BMP280 bmp;
void setup() {

  Serial.begin(9600);   

  connectWIFI();

  Serial.println(WiFi.localIP());

  server.begin();

  mqttClient.setServer( Server, mqttPort ); 

  mqttClient.setCallback( mqttSubscriptionCallback );

  mqttClient.setBufferSize( 2048 );

  Wire.begin();

  lightMeter.begin();

  Serial.begin(9600);

  if (!bmp.begin(0x76)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
}

void loop() {

  if (!mqttClient.connected()) {
     mqttConnect(); 
     mqttSubscribe( channelID );
  }

  mqttClient.loop();

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  float lux = lightMeter.readLightLevel();
  float temp = bmp.readTemperature();
  float pres = (bmp.readPressure()/100);
  float alt = bmp.readAltitude(1004.40);

  Serial.println("---༺  BH1750 & BMP280 ༻---");
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");

  Serial.print(F("Temperature = "));
  Serial.print(bmp.readTemperature());
  Serial.println(" °C");

  Serial.print(F("Pressure = "));
  Serial.print(bmp.readPressure()/100);
  Serial.println(" hPa");

  Serial.print(F("Approx altitude = "));
  Serial.print(bmp.readAltitude(1004.40));
  Serial.println(" m");
  Serial.println();

  Serial.println("---༺  LM35 ༻---");
  float tempReading = analogRead(tempPin);  
  int mv = (tempReading/4095.0)*3300;
  int degreesC = mv/10;
  Serial.println("Voltage: " + String(mv) + " mv");
  Serial.println("Temprature: " + String(degreesC) + " °C"); 

  sendDiscord("------ BH1750 & BMP280 ------");
  sendDiscord("Light level: " + String(lux) + " lx");
  sendDiscord("Temprature: " + String(temp) + " °C");
  sendDiscord("Pressure: " + String(pres) + " hPa");
  sendDiscord("Approx altitude: " + String(alt) + " m");
  sendDiscord("------ LM35 ------");
  sendDiscord("Voltage: " + String(mv) + " mv");
  sendDiscord("Temprature: " + String(degreesC) + " °C");
  
  light.update(lux);
  temperature.update(temp);
  pressure.update(pres);
  approxaltitude.update(alt);
 
  dashboard.sendUpdates();

  mqttPublish( channelID, (String("field1=")+String(lux)+"&"+String("field2=")+String(temp)+"&"+String("field3=")+String(pres)+"&"+String("field4=")+String(alt)));

  Serial.println("Going to sleep now");
  Serial.flush(); 
  esp_deep_sleep_start();
  Serial.println();
}




