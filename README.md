# IOT
TP IOT


### POUR TP3 : Certificat ESP

## BROKER MQTT

### Génération de la clé privée pour le broker MQTT
```
openssl genrsa -out mosquitto.key 2048
```
### Génération de la requête de certificat 
```
openssl req -out mosquitto.csr -key mosquitto.key -new -subj '//CN=MettreIPduBroker(Adresse IP du PC si local)'
```

### Génération du certificat pour le broket MQTT
```
openssl x509 -req -in mosquitto.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out mosquitto.crt -days 365 -passin pass:1234
```

## ESP

Génération de la clé privée pour l'ESP
```
openssl genrsa -out esp.key 2048
```
#Génération de la requête de certificat
```
openssl req -out esp.csr -key esp.key -new 
```
Renseigner les différentes informations
=> Laisser le common name vide

 Country Name (2 letter code) [AU]: FR  
 State or Province Name (full name) [Some-State]: PACA  
 Locality Name (eg, city) []:NICE  
 Organization Name (eg, company) [Internet Widgits Pty Ltd]:MIAGE  
 Organizational Unit Name (eg, section) []:M1  
 Common Name (e.g. server FQDN or YOUR name) []:  
 Email Address []:choisysebastien@gmail.com  

 Please enter the following 'extra' attributes  
 to be sent with your certificate request  
 A challenge password []:  
 An optional company name []:  

### Génération du certificat pour l'ESP
```
openssl x509 -req -in esp.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out esp.crt -days 365 -passin pass:1234 
```


