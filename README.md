MQ135 GAS SENSOR
=====

Arduino library for the MQ135 gas sensor

## Datasheet
Can be found [here](https://www.olimex.com/Products/Components/Sensors/SNS-MQ135/resources/SNS-MQ135.pdf)

## Application
They are used in air quality control equipments for buildings/offices, are suitable for detecting of NH3, NOx, alcohol, Benzene, smoke, CO2, etc

## advanced Features compared to base
  Not every device is the same. Changing the Header files is not a good way to program for a variety of devices
  - Set RZero and ATMCO2 directly in Constructor -> MQ135 gasSensor = MQ135(AIR_PIN, 52.7, 404.48);
  - Added a RefPin which is supposed to measure the Vcc of the MQ135. Constructor -> MQ135 gasSensor = MQ135(AIR_PIN,AIR_PIN_REF, 52.7, 404.48);
  - RZERO, ATOMCO2, RefPin, RLoad can be set by functions

## Features
This library has:
 - Corrections for temperature and humidity
 - Measurements:
    - getResistance
    - getCorrectedResistance
    - getPPM
    - getCorrectedPPM
    - getRZero
    - getCorrectedRZero
	- setRZero(float newVal);
    - setATMCO2(float newVal);
    - setRLOAD(float newVal);
    - setRefPin(uint8_t pin);

## Example
The example provided shows how to implement the Sensor with MySensors. For temperature I use a barometric pressure sensor.
Humidity is received from my MySensor network (don't have a spare DHT sensor)
RL[kOhm] I measure with a Voltmeter between GND and Vcc on the MQ135.
	
More Info
=====

https://hackaday.io/project/3475-sniffing-trinket/log/12363-mq135-arduino-library
