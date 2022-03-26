# Let's talk MQTT in Python 
# first install : https://pypi.org/project/paho-mqtt/
# Author : G.MENEZ

import paho.mqtt.client as mqtt
import time
import json
import random

#--------------------------------------------------

def on_message(client, userdata, message):
    print("\nmessage received = {}".format(str(message.payload.decode("utf-8"))))
    print("on topic = {}".format(message.topic))
    print("with qos = {}".format(message.qos))
    print("with retain flag = {}".format(message.retain))

#==================================================

if __name__=="__main__":

    #-------------------------------------
    clientname = "P1"
    print("Creating new Client : {}".format(clientname))
    client = mqtt.Client(clientname) # create new instance
    client.on_message=on_message     # attach function to callback

    #-------------------------------------
    broker_address="test.mosquitto.org"
    """ broker_address="iot.eclipse.org" """
    print("Connecting to broker {}".format(broker_address))
    client.connect(broker_address) # connect to broker

    client.loop_start()     #------------ start the loop

    #-------------------------------------
    topicLight = "IOTSHAN/light"
    topicTemp = "IOTSHAN/temp"

    """
    payload ="{"
    payload+="\"Temperature\":10"
    payload+=","
    payload+="\"Humidity\":50"
    payload+="}"

    payload = {
        "Temperature":10,
        "Humidity":50 }
    msg = json.dumps(payload)
    
    """


    
    while True:
       payloadTemp={}
       temperature={}
       info={}
       loc={}
       temperature["temperature"]= random.randint(12,30)
       payloadTemp["status"]= temperature
       loc["longitude"]= -122.4194
       loc["latitude"]= 37.7749
       info["loc"]=loc
       info["user"]= "espSeb"
       info["ident"]= "MAC1"
       info["ip"]= "192.168.0.5"
       payloadTemp["info"]= info

       msgTemp = json.dumps(payloadTemp)
       print("\nPublishing message {} to topic {}".format(msgTemp, topicTemp))
       client.publish(topicTemp, payload=msgTemp, qos=2, retain=False)
       time.sleep(60)
    
    time.sleep(100)         # wait in seconds before 
    client.loop_stop()      #------------ stop the loop and the script
