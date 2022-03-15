# Let's talk MQTT in Python
# first install : https://pypi.org/project/paho-mqtt/
# Author : G.MENEZ
from types import SimpleNamespace

import paho.mqtt.client as mqtt
import time
import json
import ssl


if __name__ == "__main__":
    # -------------------------------------
    clientname = "ESP"
    print("Creating new Client : {}".format(clientname))
    client = mqtt.Client(clientname)  # create new instance

    # -------------------------------------
    broker_address = "test.mosquitto.org"
    """ broker_address="iot.eclipse.org" """
    print("Connecting to broker {}".format(broker_address))
    client.connect(broker_address)  # connect to broker
    print("connected")
    # -------------------------------------

    client.loop_start()  # ------------ start the loop

            payload = {
                "temp": 25,
                "light": 1600,  
                "localisaton": "china"
            }
            print("hello")
            msg = json.dumps(payload)
            while True:
                client.publish("IOTSHAN", payload=msg, qos=2, retain=False)
                time.sleep(1)


client.loop_stop()