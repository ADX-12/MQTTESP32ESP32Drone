from plutocontrol import pluto
import time
import paho.mqtt.client as mqtt

# Initialize the drone
drone = pluto()

# Connect to the drone (must be on Pluto Wi-Fi!)
print("Connecting to the Pluto drone...")
drone.connect()             
time.sleep(2)

# MQTT broker details (replace with your Mac’s IP)
BROKER = "192.168.4.2" #192.168.0.103
PORT = 1883

# Topics mapping
TOPICS = {
    "pluto/cmd/arm": "arm",
    "pluto/cmd/disarm": "disarm",
    "pluto/cmd/takeoff": "takeoff",
    "pluto/cmd/land": "land"
}

# Function to send command to Pluto
def send_to_pluto(command):
    if command == "arm":
        drone.arm()
        print(" Drone Armed")
    elif command == "disarm":
        drone.disarm()
        print("Drone Disarmed")
    elif command == "takeoff":
        drone.takeoff()
        print(" Drone Taking Off")
    elif command == "land":
        drone.land()
        print("Drone Landing")  

# MQTT callback
def on_message(client, userdata, msg):
    print(f" Received MQTT: {msg.topic} -> {msg.payload.decode()}")
    if msg.topic in TOPICS:
        send_to_pluto(TOPICS[msg.topic])


# Setup MQTT client
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)  # ensure you're using paho v2
client.on_message = on_message

# Connect and subscribe
client.connect(BROKER, PORT, 60)
for t in TOPICS.keys():
    client.subscribe(t)

print("Listening for MQTT commands...")
client.loop_forever()
