# Executing commands through MQTT ( Subscriber & listener ) model for controlling a drone 

⚙️ Execution Steps

1️⃣ Flash Codes to ESP32 Boards

Flash the Subscriber code to the 1st ESP32 (Hub)

Flash the Listener code to the 2nd ESP32 (Command Receiver)

2️⃣ Download the Python Wrapper
Clone the Python wrapper repository:

git clone https://github.com/DronaAviation/PROJECTS_WITH_PYTHON.git


3️⃣ Add Drone Control Script

Copy the takeoff_land.py file from this repo into the cloned folder
(path: PROJECTS_WITH_PYTHON/Takeoff_land.py)
OR

Create a new file (e.g., mydrone.py) and paste your Python MQTT command code inside.

4️⃣ Connect to ESP32 Hub Wi-Fi
Run the command below to get your system’s IP address:

ipconfig getifaddr en0


👉 Note down this IP and update it in your Python script, Subscriber, and Listener code files.

5️⃣ Build and Flash
Rebuild and flash both ESP32 boards again with the updated IP address.

6️⃣ Start MQTT Broker
Open a terminal and start the Mosquitto service:

sudo service mosquitto start


7️⃣ Send Commands to the Drone
Use the following command to send MQTT messages:

mosquitto_pub -h 192.168.4.2 -t "pluto/cmd/arm" -m "arm"


💡 Replace the IP address with your actual Hub IP.
