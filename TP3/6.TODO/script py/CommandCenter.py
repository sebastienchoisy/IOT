# Let's talk MQTT in Python 
# first install : https://pypi.org/project/paho-mqtt/
# Author : G.MENEZ

import paho.mqtt.client as mqtt
import time
import json


# --------------------------------------------------

def on_message(client, userdata, message):
    print("\nmessage received = {}".format(str(message.payload.decode("utf-8"))))
    print("on topic = {}".format(message.topic))
    print("with qos = {}".format(message.qos))
    print("with retain flag = {}".format(message.retain))

    if message.topic == topicFireEmergency and message.payload == "fire extinguished":
        payload = {
            "state": "normal"
        }
        msg = json.dumps(payload)
        client.publish(topicCC, payload=msg, qos=2, retain=False)
        print("rerzerzerrz")


# ==================================================

if __name__ == "__main__":
    # -------------------------------------
    clientname = "CommandCenter"
    print("Creating new Client : {}".format(clientname))
    client = mqtt.Client(clientname)  # create new instance
    client.on_message = on_message  # attach function to callback

    # -------------------------------------
    broker_address = "192.168.1.39"
    """ broker_address="iot.eclipse.org" """
    print("Connecting to broker {}".format(broker_address))
    client.connect(broker_address)  # connect to broker

    client.loop_start()  # ------------ start the loop

    # -------------------------------------
    topicFireEmergency = "emergency/fire"
    topicSensors = "esp/sensors"
    topicCC = "command/center"

    print("\nSubscribing to topics {}".format(topicSensors), "and {}".format(topicFireEmergency))
    client.subscribe([(topicSensors, 0), (topicFireEmergency, 0)])

    time.sleep(1000)  # wait in seconds before
    client.loop_stop()  # ------------ stop the loop and the script
