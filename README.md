# ESP32 MQTT Relay Control

This project demonstrates how to use an ESP32 to connect to a WiFi network and an MQTT broker, listen to control messages, and control relays connected to the ESP32's GPIO pins. The relays can be controlled by sending specific messages to an MQTT topic.

## Hardware Required

- ESP32 development board
- Relays (6 units)
- Connecting wires
- Power supply

## Pin Configuration

The relays are connected to the following GPIO pins on the ESP32:

- Relay 1: GPIO 04
- Relay 2: GPIO 12
- Relay 3: GPIO 13
- Relay 4: GPIO 32
- Relay 5: GPIO 32
- Relay 6: GPIO 25

## Software Setup

### Libraries Required

This project uses the following libraries:

- `WiFi.h`: To connect to a WiFi network.
- `PubSubClient.h`: To connect to and communicate with an MQTT broker.

### Installation

1. Ensure you have the Arduino IDE installed on your computer.
2. Install the ESP32 board package in the Arduino IDE.
3. Install the `PubSubClient` library via the Arduino Library Manager.

### Configuration

Update the following placeholders in the code with your actual credentials and settings:

- `your_SSID`: Your WiFi network name (SSID).
- `your_PASSWORD`: Your WiFi network password.
- `your_mqtt_server`: The MQTT broker's server address.
- `your_mqtt_user`: Your MQTT username.
- `your_mqtt_password`: Your MQTT password.

### Code

```cpp
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
```

### Usage

1. Open the Arduino IDE and upload the code to your ESP32 board.
2. Open the Serial Monitor to view the connection status and debug messages.
3. Use an MQTT client (like MQTT.fx, Mosquitto, etc.) to send control messages to the `control/relay` topic in the format `relay_index.on` or `relay_index.off`.
4. The ESP32 will control the specified relay and send a response to the `control/relay/response` topic with the current state of the relay.

### Example

To turn on the relay connected to GPIO 17 (relay index 0), send the message:

```
0.on
```

To turn off the relay connected to GPIO 17, send the message:

```
0.off
```

You will receive a response message indicating the current state of the relay, for example:

```
Relay 0 is now ON
```

### Node-RED

Open the "Import" tab from the Node-RED menu and paste the JSON code in.
Edit the MQTT broker information.

```
[{"id":"59e5672e3ccb5717","type":"mqtt in","z":"e3873bd17493ca05","name":"","topic":"control/relay/response","qos":"2","datatype":"auto-detect","broker":"","nl":false,"rap":true,"rh":0,"inputs":0,"x":570,"y":300,"wires":[["eaf69504412a2ba4"]]},{"id":"53fb24c7175d5801","type":"mqtt out","z":"e3873bd17493ca05","name":"","topic":"control/relay","qos":"","retain":"","respTopic":"","contentType":"","userProps":"","correl":"","expiry":"","broker":"","x":700,"y":140,"wires":[]},{"id":"eaf69504412a2ba4","type":"debug","z":"e3873bd17493ca05","name":"debug 40","active":true,"tosidebar":true,"console":false,"tostatus":false,"complete":"false","statusVal":"","statusType":"auto","x":745,"y":300,"wires":[],"l":false},{"id":"98715cd16bc684fa","type":"ui-switch","z":"e3873bd17493ca05","name":"","label":"switch","group":"c356ef39718bc72d","order":1,"width":0,"height":0,"passthru":false,"decouple":false,"topic":"topic","topicType":"msg","style":"","className":"","layout":"row-spread","clickableArea":"switch","onvalue":"3.on","onvalueType":"str","onicon":"","oncolor":"","offvalue":"3.off","offvalueType":"str","officon":"","offcolor":"","x":495,"y":140,"wires":[["53fb24c7175d5801"]],"l":false},{"id":"c356ef39718bc72d","type":"ui-group","name":"Switch Relay","page":"4f325fe094285d99","width":6,"height":1,"order":1,"showTitle":true,"className":"","visible":"true","disabled":"false","groupType":"default"},{"id":"4f325fe094285d99","type":"ui-page","name":"Relay test","ui":"a85527ea6a7aeda3","path":"/relaytest","icon":"home","layout":"grid","theme":"09222e4dff225a54","breakpoints":[{"name":"Default","px":"0","cols":"3"},{"name":"Tablet","px":"576","cols":"6"},{"name":"Small Desktop","px":"768","cols":"9"},{"name":"Desktop","px":"1024","cols":"12"}],"order":12,"className":"","visible":true,"disabled":"false"},{"id":"a85527ea6a7aeda3","type":"ui-base","name":"Lakeview IoT (CM4)","path":"/dashboard","appIcon":"","includeClientData":true,"acceptsClientConfig":["ui-notification","ui-control"],"showPathInSidebar":false,"headerContent":"page","navigationStyle":"icon","titleBarStyle":"hidden","showReconnectNotification":false,"notificationDisplayTime":1,"showDisconnectNotification":false},{"id":"09222e4dff225a54","type":"ui-theme","name":"Alpha Bits - Lakeview","colors":{"surface":"#ffffff","primary":"#0088cc","bgPage":"#ffffff","groupBg":"#ffffff","groupOutline":"#cccccc"},"sizes":{"density":"default","pagePadding":"16px","groupGap":"16px","groupBorderRadius":"8px","widgetGap":"12px"}}]
```

