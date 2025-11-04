# Multi-ESP32 MQTT Drone Control System

Communicating with a Drone using Python and MQTT

Project Information

Project Title: Multi-ESP32 MQTT Drone Control System
Developed For: Meritus.AI
Developed By: Apurva Donde
Role: Drone Systems Developer | Tools & Automation | STEM Education

 Overview

This project establishes a multi-layer MQTT communication system between a Python script, two ESP32 boards, and a Pluto Drone.
It demonstrates how commands can be transmitted over Wi-Fi using MQTT topics to control drone operations such as arm, takeoff, land, and disarm.

 Hardware Requirements
Component	Quantity	Description
ESP32	2	Wi-Fi enabled microcontrollers
Pluto Drone	1	Target drone to receive and execute commands

 System Topology
Python Script  →  ESP32 (Bridge / Hub)  →  ESP32 (Listener)  →  Pluto Drone
       (MQTT)             (Wi-Fi)                 (Serial / Command Execution)


All communication between devices is handled using the MQTT protocol.

 Components Description
1. ESP32 – MQTT Bridge (Wi-Fi Hub)

Acts as a Wi-Fi Access Point (AP).

Subscribes to topic:

pluto/cmd/#


Forwards messages to:

bridge/esp32/cmd


Displays all received messages via the Serial Monitor for debugging.

 2. ESP32 – MQTT Listener

Connects to the ESP32 Hub Wi-Fi network.

Subscribes to topic:

bridge/esp32/cmd


Receives commands and executes corresponding drone control operations:

🟢 arm

🔴 disarm

✈️ takeoff

🛬 land

💻 3. Python – Command Publisher

Script file: send_mqtt_cmd.py

Publishes MQTT messages to the following topics:

pluto/cmd/arm
pluto/cmd/disarm
pluto/cmd/takeoff
pluto/cmd/land

Run Command

To send a command from the terminal:

python3 send_mqtt_cmd.py arm


(Replace “arm” with disarm, takeoff, or land as required.)

Command Execution via Terminal

You can also send commands using mosquitto_pub:

mosquitto_pub -h 192.168.4.2 -t "pluto/cmd/arm" -m "arm"
mosquitto_pub -h 192.168.4.2 -t "pluto/cmd/takeoff" -m "takeoff"
mosquitto_pub -h 192.168.4.2 -t "pluto/cmd/land" -m "land"

 Troubleshooting & Network Utilities
 Checking Local IP Address

Use the following command (Mac/Linux) to identify your local IP:

ipconfig getifaddr en0

Common Issues & Solutions
Issue	Possible Cause	Solution
MQTT messages not received	Incorrect topic or broker IP	Verify MQTT topics and broker IP
ESP32 not connecting	Wrong SSID/password	Double-check Wi-Fi credentials
No serial output	Baud rate mismatch	Ensure Serial Monitor is set to the correct baud rate

 Future Improvements
Add telemetry feedback from the drone to the Python dashboard.

Implement command acknowledgment for improved reliability.

 Author

Apurva Donde
Drone Systems Developer | Tools & Automation | STEM Education

Project made for: Meritus.AI
