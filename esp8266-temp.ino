#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4

#include "secrets.h"
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

const char broker[] = SECRET_BROKER_HOST;
int port = SECRET_BROKER_PORT;

char nameSpace[] = "b";
char deviceClass[] = "2";
char deviceId[] = "2";
const char topic[]  = "b/2/2"; // Concat from above later

int interval = 5000; // Loop interval in milliseconds
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

WiFiClient espClient;
PubSubClient client(espClient);

void setup(void) {
  pinMode(ONE_WIRE_BUS, INPUT_PULLUP);
  sensors.begin();

  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  setup_wifi();
  client.setServer(broker, port);

  if (!client.connected()) {
    reconnect();
  }

  read_sensor_and_publish_message();

  delay(20);
  Serial.println("Going to sleep.");
  ESP.deepSleep(300e6);
}

void loop(void){}

void read_sensor_and_publish_message() {
    sensors.requestTemperatures();
    float t = sensors.getTempCByIndex(0);

    StaticJsonDocument<200> message;
    message["up"] = millis();
    message["id"] = deviceId;
  
    StaticJsonDocument<200> data;
    data["type"] = "temp";
    data["val"] = t;
    data["pin"] = ONE_WIRE_BUS;
  
    message["data"] = data;
    
    char jsonBuffer[512];
    serializeJson(message, jsonBuffer);
    client.publish(topic, jsonBuffer);

    char prettyJson[512];
    serializeJsonPretty(message, prettyJson);

    Serial.print("Publish message to topic ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(prettyJson);

    Serial.println("-------------");
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
