# ESP8266-Temp
This is a sketch that runs an ESP8266 with a single-wire temperature sensor on it.

The sketch will:
* setup:
  * connect to WiFi
  * connect to an MQTT broker
* loop:
  * take a measurement
  * publish a message

