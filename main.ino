#include <WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// MQTT broker credentials
const char* mqtt_server = "your_mqtt_server";
const char* mqtt_user = "your_mqtt_user";
const char* mqtt_password = "your_mqtt_password";

// MQTT topics
const char* control_topic = "control/relay";
const char* response_topic = "control/relay/response";

// Relay pins
const int relay_pins[] = {4, 12, 13, 32, 33, 25};
const int num_relays = sizeof(relay_pins) / sizeof(relay_pins[0]);

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(message);

  // Parse the message for relay control
  int separatorIndex = message.indexOf('.');
  if (separatorIndex != -1) {
    int relay_index = message.substring(0, separatorIndex).toInt();
    String command = message.substring(separatorIndex + 1);

    if (relay_index >= 0 && relay_index < num_relays) {
      int relay_pin = relay_pins[relay_index];
      if (command == "on") {
        digitalWrite(relay_pin, HIGH);
      } else if (command == "off") {
        digitalWrite(relay_pin, LOW);
      }
      
      // Send response
      String response = "Relay " + String(relay_index) + " is now " + (digitalRead(relay_pin) ? "ON" : "OFF");
      client.publish(response_topic, response.c_str());
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(control_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Initialize relay pins
  for (int i = 0; i < num_relays; i++) {
    pinMode(relay_pins[i], OUTPUT);
    digitalWrite(relay_pins[i], LOW);
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
