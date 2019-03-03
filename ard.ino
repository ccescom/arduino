#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "JU-AUDI";
const char* password =  "juaudi@12345";
const char* mqttServer = "13.233.113.49";//Broker server address
const int mqttPort = 1883;
const char* mqttUser = "deploy";
const char* mqttPassword = "sih2019";
const char* mqttMessageAction = "";
int digitalWriteValue = 0;
boolean remoteControl = false;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() 
{

  pinMode(D3, INPUT_PULLUP);   //SWITCH AS INPUT
  pinMode(D4,OUTPUT);            // RELAY AS OUTPUT
  digitalWrite(D4,HIGH);
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {

      Serial.println("connected");  

    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);

    }
  }

//  StaticJsonDocument<200> doc;
//  char json[] =
//      "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";
//
//  DeserializationError error = deserializeJson(doc, json);
//
//  if (error) {
//    Serial.print(F("deserializeJson() failed: "));
//    Serial.println(error.c_str());
//    return;
//  }
  
//  client.publish("esp/test/message", "Hello from ESP8266");
  client.subscribe("/1JAI001/1");
}

void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  StaticJsonDocument<200> message;
  char json[length];
  for (int i = 0; i < length; i++)
  {
    //Serial.print((char)payload[i]);
    json[i] = (char)payload[i];
  }
  DeserializationError jsonParseError = deserializeJson(message, json);

  if (jsonParseError) {
    remoteControl = false;
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(jsonParseError.c_str());
    return;
  }else{
    mqttMessageAction = message["action"];
    Serial.println("MEssage");
    Serial.println(mqttMessageAction);
    if(message["action"]=="on"){
      remoteControl=false;
      digitalWriteValue = digitalRead(D3)==LOW ? 1 : 0;
    }else if(message["action"]=="off"){ 
      remoteControl=true;
      digitalWriteValue = 0;
    } else if(message["action"]=="check"){
      //HeartBeat Code return active status
      StaticJsonDocument<20> doc;
      doc["status"] = "active";
      char jsonResponse[20];
      serializeJson(doc,jsonResponse);
      if(client.publish("/1JAI001/1/response",jsonResponse)==true){
        Serial.println("Successfully sent response");
      }else{
        Serial.println("Error sending response");
      }
    }
  }
  Serial.println();
  Serial.println("-----------------------");

}

void loop()
{
  client.loop();
  if(!remoteControl){
    if(digitalRead(D3)==LOW /*|| digitalWriteValue==LOW*/)
    {
      Serial.println("Switch ON");
      digitalWrite(D4,LOW);
    }
    else if(digitalRead(D3)==HIGH/* || digitalWriteValue==HIGH*/)
    {
      Serial.println("Switch OFF");
      digitalWrite(D4,HIGH);
    }
  }else{
    Serial.println("Switch based on response");
    if(digitalWriteValue==1){
      Serial.println("Switch ON");
      digitalWrite(D4,LOW);
    }else if(digitalWriteValue==0){
      Serial.println("Switch OFF");
      digitalWrite(D4,HIGH);
    }
  }
  delay(300);
}
