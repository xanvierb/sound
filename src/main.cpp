#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define NUMBER_OF_MEASUREMENTS 1000000
#define NUMBER_OF_SUMS 3000

////// WIFI settings //////
const char* ssid = "";     // your network SSID (name of wifi network)
const char* password = ""; // your network password

////// MQTT Settings //////
//const char* mqtt_server = "10.151.236.151"; // dev
//const char* mqtt_server = "10.151.236.156"; // acc
const char* mqtt_server = "";                 // acc secure
const int mqtt_port = 8883;

// const char* mqtt_user = "urn:dev:DVNUUID:60d74832-65d8-4804-8c76-b6d44d3b6474:"; // acc omgeving
// const char* mqtt_user = "urn:dev:DVNUUID:13d0f3a6-9307-4da2-8704-e6e2cc8d8f6f:"; // dev omgeving
// const char* mqtt_user = "urn:dev:DVNUUID:633220eb-0af3-4123-84f1-1d0f65783bd3:"; // hackathon omgeving
const char* mqtt_user = "urn:dev:DVNUUID:97b1d52c-14cf-4aa4-8297-26aeb6dab69c:";    // mqtts acc omgeving
const char* mqtt_password = "";
const char* mqtt_topic = "outTopic";

////// Do not edit below //////
int countPin = D0;
int sendCount = 0;

int countValue = 0;
int measurements = 0;

// WiFiClient espClient;
// PubSubClient client(espClient);
WiFiClientSecure secureClient;
PubSubClient client(secureClient);

void setup_wifi() {
  delay(10);
  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(9600);
  pinMode(countPin, INPUT);  
  setup_wifi();
  Serial.println("\n=========\nStarting...\n=========\n");
}

char * generatePayload(double measure) {
  char measureStr[20];
  dtostrf(measure, 6, 2, measureStr); 
  char *payload = (char*) malloc(sizeof(char) * 200); // Replace 200 with size that's right for you
  
  if(payload == NULL) {
    return NULL;
  }

  sprintf(payload, "%d,%s,%s", sendCount++, mqtt_user, measureStr);
  return payload;
}

int countNonBusyWaiting(){
  if (measurements < NUMBER_OF_MEASUREMENTS) {
    countValue = countValue + digitalRead(countPin);
    measurements++;
  } else {
    measurements = 0;
    int retval = countValue;
    countValue = 0;
    return retval;
  }
  return -1;  
}

void loop (){
  int count = countNonBusyWaiting();
  if (count != -1) {
    char *payload = generatePayload(count);
    client.setServer(mqtt_server, mqtt_port);
    if (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect;
      if (client.connect("1", mqtt_user, mqtt_password)) {
        Serial.println("connected");
        // Once connected, publish a measurement...
        Serial.print("Sending payload: ");
        Serial.println(payload);
        bool result = client.publish(mqtt_topic, payload);
        if(result) {
          Serial.println("Message published successfully");
        } else {
          Serial.println("Message publishing failed");
        }
        free(payload); // Free the allocated memory after using it
        client.disconnect();
      } else {
        Serial.print("failed, rc=");
        Serial.println(client.state());
      }
    }
    count = -1;
  } 
}
