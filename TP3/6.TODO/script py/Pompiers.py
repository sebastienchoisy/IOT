# Let's talk MQTT in Python
# first install : https://pypi.org/project/paho-mqtt/
# Author : G.MENEZ

import paho.mqtt.client as mqtt
import time
import json


# --------------------------------------------------
def on_message(client, userdata, message):
    global state
    print("\nmessage received = {}".format(str(message.payload.decode("utf-8"))))
    print("on topic = {}".format(message.topic))
    print("with qos = {}".format(message.qos))
    print("with retain flag = {}".format(message.retain))

    if message.topic == topicFireEmergency and message.payload.decode("utf-8") == "FIRE EMERGENCY":
        state = "en intervention"


# ==================================================

if __name__ == "__main__":
    # -------------------------------------
    clientname = "Pompier"
    state = "en attente"
    print("Creating new Client : {}".format(clientname))
    client = mqtt.Client(clientname)  # create new instance
    client.on_message = on_message  # attach function to callback

    # -------------------------------------
    broker_address = "192.168.1.39"
    """ broker_address="iot.eclipse.org" """
    print("Connecting to broker {}".format(broker_address))
    client.connect(broker_address)  # connect to broker
    # -------------------------------------
    topicFireEmergency = "emergency/fire"
    topicFireFightersCommunication = "emergency/firefighters"

    client.loop_start()  # ------------ start the loop

    print("\nSubscribing to topic {}".format(topicFireEmergency))
    client.subscribe(topicFireEmergency)

    while True:
        if state == "en intervention":
            payload = {
                "status": "en route",
                "state": "fire"
            }
            msg = json.dumps(payload)
            client.publish(topicFireFightersCommunication, payload=msg, qos=2, retain=False)
            time.sleep(5)
            payload = {
                "status": "sur place",
                "state": "fire"
            }
            msg = json.dumps(payload)
            client.publish(topicFireFightersCommunication, payload=msg, qos=2, retain=False)
            time.sleep(10)
            payload = {
                "status": "retour en caserne",
                "state": "normal"
            }
            msg = json.dumps(payload)
            client.publish(topicFireFightersCommunication, payload=msg, qos=2, retain=False)
            state = "en attente"


