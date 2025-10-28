# Communicating with a drone using python and Mqtt 
Multi-ESP32 MQTT Drone Control System
 Overview

This project establishes a multi-layer MQTT communication system between a Python script, two ESP32 boards, and a Pluto Drone.
It demonstrates how commands can be transmitted over Wi-Fi using MQTT topics to control drone operations such as arm, takeoff, land, and disarm.

⚙️ Hardware Requirements
Component	Quantity	Description
ESP32	2	Wi-Fi enabled microcontrollers
Pluto Drone	1	Target drone to receive and execute commands
 System Topology
Python Script  →  ESP32 (Bridge / Hub)  →  ESP32 (Listener)  →  Pluto Drone
        (MQTT)             (Wi-Fi)                 (Serial / Command Execution)


Communication across all devices is handled using MQTT protocol.

 Components Description
1. ESP32 – MQTT Bridge (Wi-Fi Hub)

Acts as a Wi-Fi Access Point (AP).

Subscribes to topic:

pluto/cmd/#


Forwards all received messages to:

bridge/esp8266/cmd


Displays logs and messages via the Serial Monitor.

2. ESP32 – MQTT Listener

Connects to the ESP32-Hub Wi-Fi network.

Subscribes to topic:

bridge/esp32/cmd


Receives messages and executes corresponding drone commands:

arm

disarm

takeoff

land

3. Python – Command Publisher

Script File: send_mqtt_cmd.py

Publishes MQTT messages to the drone topics:

pluto/cmd/arm
pluto/cmd/disarm
pluto/cmd/takeoff
pluto/cmd/land

 Run Command
python3 send_mqtt_cmd.py arm


(Replace “arm” with desired command: disarm, takeoff, or land)

Command Execution via Terminal

Once all connections are established, commands can be sent using mosquitto_pub:

mosquitto_pub -h 192.168.4.2 -t "pluto/cmd/arm" -m "arm"
mosquitto_pub -h 192.168.4.2 -t "pluto/cmd/takeoff" -m "takeoff"
mosquitto_pub -h 192.168.4.2 -t "pluto/cmd/land" -m "land"

Troubleshooting & Network Utilities
 Checking Local IP Address

Use the following command to identify your local IP (Mac/Linux):

ipconfig getifaddr en0

Common Issues
Issue	Possible Cause	Solution
MQTT messages not received	Incorrect topic or broker IP	Verify MQTT topics and broker IP
ESP32 not connecting	Wrong SSID/password	Double-check Wi-Fi credentials
No serial output	Baud rate mismatch	Ensure Serial Monitor is set to correct baud rate
 Future Improvements

Add telemetry feedback from the drone to the Python dashboard.

Integrate with FlytBase API or ROS2 for advanced multi-drone coordination.

Add command acknowledgement for improved reliability.

 Author

Apurva Donde
Drone Systems Developer | Tools & Automation | STEM Education
