#include<WiFi.h>
#include<PubSubClient.h>
int lm35 = 34;
int  GSR = 35;
float l,t1;
int sensorValue;
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
MAX30105 particleSensor;
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;

WiFiClient espClient;
PubSubClient client (espClient);

const char* ssid = "test";
const char* password = "12345678";
const char* mqtt_server = "broker.mqtt-dashboard.com";
long lasMsg =0;
char msg[50];

int value=0;
String message="";
char val[50];
String top;

void connectwifi() {
  
   delay(10);
  Serial.print("CONNECTING TO");
  Serial.println(ssid);

  WiFi.begin(ssid,password);
  while (WiFi.status() !=WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  
  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP ADDRESS:");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length)
{
  top = topic;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  message += '\0';
  Serial.println(message);
  Serial.println();
}

void reconnect() {
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("node");
    }
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void mqtt()
{
   client.setCallback(callback);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
 
}
void setup()
{
  Serial.begin(9600);
  connectwifi();
  client.setServer(mqtt_server,1883);
  pinMode(lm35,INPUT);
  Serial.println("Initializing...");
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); 
  particleSensor.setPulseAmplitudeGreen(0); 
}
void loop()
{
  mqtt();
  l = analogRead(lm35);
  t1 = l/10;
  int temp;
  float conductivevoltage;
  sensorValue=analogRead(GSR);
  conductivevoltage = sensorValue*(5.0/1023.0);
  Serial.println("GSR="+String (conductivevoltage));
  Serial.println("TEMP ="+String (t1));
  delay(500);
  long irValue = particleSensor.getIR();
  if (checkForBeat(irValue) == true)
  {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);
  delay(500);

  if (irValue < 50000)
  {
    Serial.print(" No finger?");
    Serial.println();
  }
  
  if(t1 >=33)
  {
    client.publish("WOMENSAFETY","ABORMAL TEMP");
    Serial.println("ABORMAL TEMP");
  }
  if (conductivevoltage >=7 && conductivevoltage <=9)
  {
    client.publish("WOMENSAFETY","ABORMAL GSR");
    Serial.println(" ABNORMAL GSRVALUE");
  }
 
}
