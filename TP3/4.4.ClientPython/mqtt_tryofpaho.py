# Let's talk MQTT in Python 
# first install : https://pypi.org/project/paho-mqtt/
# Author : G.MENEZ

import paho.mqtt.client as mqtt # pip install paho-mqtt pour installer la lib
import time
import json

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
    broker_address="192.168.1.39"
    """ broker_address="iot.eclipse.org" """
    print("Connecting to broker {}".format(broker_address))
    client.connect(broker_address) # connect to broker

    client.loop_start()     #------------ start the loop

    #-------------------------------------
    topicLed = "my/sensors/led"
    topicTemp = "my/sensors/temp"
    
    print("\nSubscribing to topic {}".format(topicLed))
    client.subscribe([(topicLed,0),(topicTemp,0)])

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
    
    msg = "off"
    for i in range(3) : # On publie 3 messages "off"
        print("\nPublishing message {} to topic {}".format(msg, topicLed))
        client.publish(topicLed, payload=msg, qos=2, retain=False)
        time.sleep(5)
    
    time.sleep(100)         # wait in seconds before 
    client.loop_stop()      #------------ stop the loop and the script
