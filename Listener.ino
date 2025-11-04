// ESP32_MQTT_Listener_MSP_DualWiFi.ino
// MQTT listener that hops Wi-Fi between broker SSID and drone SSID to send MSP commands.

#include <WiFi.h>
#include <PubSubClient.h>

// ================= Wi-Fi Profiles =================
// 1) MQTT broker network (your laptop is here)
static const char* BROKER_SSID = "ESP32-Hub";
static const char* BROKER_PASS = "esp32password";

// 2) Drone AP network (Pluto)
static const char* DRONE_SSID  = "Pluto_2025_2342";
static const char* DRONE_PASS  = "4367pluto";

// ================= MQTT =================
static const char* MQTT_BROKER_IP = "192.168.4.3";
static const uint16_t MQTT_PORT   = 1883;
static const char* LISTEN_TOPIC   = "bridge/esp32/cmd";

WiFiClient espClient;
PubSubClient mqtt(espClient);

// ================= Drone (MSP over TCP) =================
const char* DRONE_HOST = "192.168.4.1";
const uint16_t DRONE_PORT = 23;


WiFiClient drone; // TCP client for MSP socket

// ================= MSP IDs =================
#define MSP_SET_RAW_RC      200
#define MSP_SET_COMMAND     217
#define MSP_ACC_CALIBRATION 205
#define MSP_MAG_CALIBRATION 206

// ================= RC =================
enum { rc_Roll=0, rc_Pitch, rc_Throttle, rc_Yaw, rcAux1, rcAux2, rcAux3, rcAux4 };
enum { RC_MAX=1900, RC_MID=1500, RC_MIN=1100 };

static uint16_t g_rc16[8] = {
  RC_MID, RC_MID, RC_MIN, RC_MID,  // roll, pitch, throttle, yaw
  RC_MAX,                          // AUX1
  RC_MID,                          // AUX2 (developer off)
  RC_MID,                          // AUX3
  RC_MID                           // AUX4 (arm mid/safe)
};

// ================= Net role =================
enum NetRole { NET_BROKER, NET_DRONE, NET_NONE };
static NetRole currentRole = NET_NONE;

// ---------- helpers ----------
bool connectWiFi(const char* ssid, const char* pass, uint32_t timeout_ms=12000) {
  if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == ssid) return true;
  WiFi.disconnect(true, true);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(ssid, pass);
  Serial.print("WiFi connecting to "); Serial.print(ssid);

  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - t0) < timeout_ms) {
    delay(300);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\nWiFi OK: "); Serial.print(ssid);
    Serial.print(" IP="); Serial.println(WiFi.localIP());
    return true;
  }
  Serial.println("\nWiFi connect FAILED");
  return false;
}

bool connectBrokerWiFi() {
  bool ok = connectWiFi(BROKER_SSID, BROKER_PASS);
  currentRole = ok ? NET_BROKER : NET_NONE;
  return ok;
}

bool connectDroneWiFi() {
  bool ok = connectWiFi(DRONE_SSID, DRONE_PASS);
  currentRole = ok ? NET_DRONE : NET_NONE;
  return ok;
}

bool ensureDroneLink() {
  if (currentRole != NET_DRONE) {
    if (!connectDroneWiFi()) return false;
  }
  if (drone.connected()) return true;
  Serial.print("Connecting to drone "); Serial.print(DRONE_HOST);
  Serial.print(":"); Serial.print(DRONE_PORT); Serial.print(" ... ");
  if (drone.connect(DRONE_HOST, DRONE_PORT)) {
    Serial.println("OK");
    return true;
  }
  Serial.println("FAILED");
  return false;
}

uint8_t mspChecksum(uint8_t len, uint8_t cmd, const uint8_t* payload) {
  uint8_t cs = len ^ cmd;
  for (uint8_t i=0; i<len; i++) cs ^= payload[i];
  return cs;
}

void mspSend(uint8_t cmd, const uint8_t* payload, uint8_t len) {
  if (!ensureDroneLink()) return;
  drone.write((uint8_t)'$');
  drone.write((uint8_t)'M');
  drone.write((uint8_t)'<');
  drone.write(len);
  drone.write(cmd);
  if (len && payload) drone.write(payload, len);
  drone.write(mspChecksum(len, cmd, payload));
}

void mspSendRawRC() {
  uint8_t rc8[16];
  for (uint8_t i=0; i<8; i++) {
    rc8[2*i]     =  g_rc16[i]       & 0xFF;
    rc8[2*i + 1] = (g_rc16[i] >> 8) & 0xFF;
  }
  mspSend(MSP_SET_RAW_RC, rc8, sizeof(rc8));
}

void pulseRawRC(uint16_t ms, uint16_t interval_ms=12) {
  uint32_t t0 = millis();
  while ((millis() - t0) < ms) {
    mspSendRawRC();
    delay(interval_ms);
  }
}

void mspCommand(uint8_t c0, uint8_t c1) {
  uint8_t p[2] = { c0, c1 };
  mspSend(MSP_SET_COMMAND, p, 2);
}

// ---------- actions (run on DRONE Wi-Fi) ----------
void doArmPulse() {
  g_rc16[rcAux4] = RC_MAX;  // ARM high
  mspSendRawRC();
  delay(50);
  g_rc16[rcAux4] = RC_MID;  // back to mid
  mspSendRawRC();
}

void action_arm() {
  Serial.println("[MSP] ARM");
  if (!ensureDroneLink()) return;
  doArmPulse();
}

void action_disarm() {
  Serial.println("[MSP] DISARM");
  if (!ensureDroneLink()) return;
  g_rc16[rcAux4] = RC_MAX;  // per original logic
  pulseRawRC(120);
  g_rc16[rcAux4] = RC_MID;
  mspSendRawRC();
}

void action_takeoff() {
  Serial.println("[MSP] TAKEOFF");
  if (!ensureDroneLink()) return;
  doArmPulse();
  delay(50);
  mspCommand(0x01, 0x00);   // TAKEOFF
  pulseRawRC(300);
}

void action_land() {
  Serial.println("[MSP] LAND");
  if (!ensureDroneLink()) return;
  mspCommand(0x02, 0x00);   // LAND
  pulseRawRC(3000);         // keep RC streaming while it settles
  g_rc16[rcAux4] = RC_MAX;  // then disarm high pulse
  pulseRawRC(120);
  g_rc16[rcAux4] = RC_MID;
  mspSendRawRC();
}

// ---------- hop back to broker Wi-Fi and MQTT ----------
void backToBrokerAndMqtt() {
  // Close drone socket to free resources
  if (drone.connected()) drone.stop();
  connectBrokerWiFi();

  // Reconnect MQTT (loop() will also call this if needed)
  if (!mqtt.connected()) {
    uint64_t chipid = ESP.getEfuseMac();
    char clientId[40];
    snprintf(clientId, sizeof(clientId), "esp32-listener-%04X%08X",
            (uint16_t)(chipid >> 32), (uint32_t)chipid);
    if (mqtt.connect(clientId)) {
      mqtt.subscribe(LISTEN_TOPIC);
      Serial.println("MQTT reconnected & subscribed.");
    } else {
      Serial.print("MQTT reconnect failed rc="); Serial.println(mqtt.state());
    }
  }
}

// ================= MQTT callback =================
void onMqtt(char* topic, byte* payload, unsigned int length) {
  String msg; msg.reserve(length);
  for (unsigned int i=0; i<length; i++) msg += (char)payload[i];

  Serial.print("MQTT: "); Serial.print(topic); Serial.print(" -> "); Serial.println(msg);

  // --- hop to drone net, act, then return ---
  if      (msg == "arm")     { if (connectDroneWiFi()) action_arm();     backToBrokerAndMqtt(); }
  else if (msg == "disarm")  { if (connectDroneWiFi()) action_disarm();  backToBrokerAndMqtt(); }
  else if (msg == "takeoff") { if (connectDroneWiFi()) action_takeoff(); backToBrokerAndMqtt(); }
  else if (msg == "land")    { if (connectDroneWiFi()) action_land();    backToBrokerAndMqtt(); }
  else                       { Serial.println("Unknown command"); }
}

// ================= MQTT keepalive =================
void ensureMqtt() {
  if (!mqtt.connected()) {
    uint64_t chipid = ESP.getEfuseMac();
    char clientId[40];
    snprintf(clientId, sizeof(clientId), "esp32-listener-%04X%08X",
            (uint16_t)(chipid >> 32), (uint32_t)chipid);
    if (mqtt.connect(clientId)) {
      mqtt.subscribe(LISTEN_TOPIC);
      Serial.println("MQTT connected & subscribed.");
    }
  }
}

// ================= Arduino setup/loop =================
void setup() {
  Serial.begin(115200);
  delay(60);

  // Default to broker Wi-Fi so we can receive commands
  connectBrokerWiFi();

  mqtt.setServer(MQTT_BROKER_IP, MQTT_PORT);
  mqtt.setCallback(onMqtt);

  // Neutral RC baseline
  g_rc16[rc_Roll]     = RC_MID;
  g_rc16[rc_Pitch]    = RC_MID;
  g_rc16[rc_Throttle] = RC_MIN;
  g_rc16[rc_Yaw]      = RC_MID;
  g_rc16[rcAux1]      = RC_MAX;
  g_rc16[rcAux2]      = RC_MID;
  g_rc16[rcAux3]      = RC_MID;
  g_rc16[rcAux4]      = RC_MID;
}

void loop() {
  // Maintain broker Wi-Fi and MQTT when idle
  if (currentRole != NET_DRONE && (WiFi.status() != WL_CONNECTED || WiFi.SSID() != BROKER_SSID)) {
    connectBrokerWiFi();
  }
  ensureMqtt();
  mqtt.loop();

  // Optional: very light RC keepalive if we happen to be on the drone net
  static uint32_t tKeep = 0;
  if (currentRole == NET_DRONE && drone.connected() && millis() - tKeep > 500) {
    mspSendRawRC();
    tKeep = millis();
  }
}
