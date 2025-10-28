// ESP32_MQTT_Bridge_AP.ino 

#include <WiFi.h> 

#include <PubSubClient.h> 

 

// ---------- SoftAP (ESP32 creates Wi-Fi) ---------- 

const char* AP_SSID = "ESP32-Hub"; 

const char* AP_PASS = "esp32password"; // >= 8 chars 

 

// ---------- MQTT (broker runs on your laptop) ---------- 

const char* MQTT_BROKER_IP = "192.168.4.3"; // <-- laptop IP on ESP32 AP //192.168.4.2 

const uint16_t MQTT_PORT = 1883; 

 

WiFiClient espClient; 

PubSubClient mqtt(espClient); 

 

// Topics 

const char* SUB_BASE = "pluto/cmd/#"; // Python publishes here 

const char* FWD_TOPIC = "bridge/esp32/cmd"; // ESP32 forwards here 

 

void ensureAp() { 

WiFi.mode(WIFI_AP); 

bool ok = WiFi.softAP(AP_SSID, AP_PASS); 

Serial.println(ok ? "SoftAP started" : "SoftAP failed!"); 

Serial.print("AP SSID: "); Serial.println(AP_SSID); 

Serial.print("AP IP : "); Serial.println(WiFi.softAPIP()); // expect 192.168.4.1 

} 

 

void onMqtt(char* topic, byte* payload, unsigned int length) { 

String msg; msg.reserve(length); 

for (unsigned int i = 0; i < length; i++) msg += (char)payload[i]; 

 

Serial.print("ESP32 got MQTT: "); Serial.print(topic); 

Serial.print(" -> "); Serial.println(msg); 

 

// forward unchanged payload to the 8266 topic 

mqtt.publish(FWD_TOPIC, msg.c_str()); 

Serial.print("Forwarded to "); Serial.println(FWD_TOPIC); 

} 

 

void ensureMqtt() { 

while (!mqtt.connected()) { 

Serial.print("Connecting MQTT..."); 

if (mqtt.connect("esp32-bridge")) { 

Serial.println("connected"); 

mqtt.subscribe(SUB_BASE); 

Serial.print("Subscribed: "); Serial.println(SUB_BASE); 

} else { 

Serial.print("failed, rc="); Serial.print(mqtt.state()); 

Serial.println(" retry in 2s"); 

delay(2000); 

} 

} 

} 

 

void setup() { 

Serial.begin(115200); 

ensureAp(); 

mqtt.setServer(MQTT_BROKER_IP, MQTT_PORT); 

mqtt.setCallback(onMqtt); 

} 

 

void loop() { 

// AP stays up automatically 

if (!mqtt.connected()) ensureMqtt(); 

mqtt.loop(); 

} 

 