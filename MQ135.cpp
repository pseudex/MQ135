/**************************************************************************/
/*!
@file     MQ135.cpp
@author   G.Krocker (Mad Frog Labs)
@license  GNU GPLv3

First version of an Arduino Library for the MQ135 gas sensor
TODO: Review the correction factor calculation. This currently relies on
the datasheet but the information there seems to be wrong.

@section  HISTORY

v1.0 - First release
*/
/**************************************************************************/

#include "MQ135.h"

/**************************************************************************/
/*!
@brief  Default constructor

@param[in] pin  The analog input pin for the readout of the sensor
*/
/**************************************************************************/

MQ135::MQ135(uint8_t pin) {
	_pin = pin;
}


/**************************************************************************/
/*!
@brief  advanced Contructor

@param[in] pin  The analog input pin for the readout of the sensor
@param[in] in_rzero replaces the default value for RZERO from header file
@param[in] in_armCO2 replaces the defaul value for atmCO2 from header file
*/
/**************************************************************************/
MQ135::MQ135(uint8_t pin, float in_rzero, float in_atmCO2) {
	_pin = pin;
	_fRZERO = in_rzero;
	_fATMOCO2 = in_atmCO2;
}

/**************************************************************************/
/*!
@brief  advanced Contructor + ref pin

@param[in] pin  The analog input pin for the readout of the sensor
@param[in] pinRef ref pin to measure the Vcc of MQ135
@param[in] in_rzero replaces the default value for RZERO from header file
@param[in] in_armCO2 replaces the defaul value for atmCO2 from header file
*/
/**************************************************************************/
MQ135::MQ135(uint8_t pin, uint8_t pinRef, float in_rzero, float in_atmCO2) {
	_pin = pin;
	_pinRef = pinRef;
	_fRZERO = in_rzero;
	_fATMOCO2 = in_atmCO2;
}


/**************************************************************************/
/*!
@brief  Get the correction factor to correct for temperature and humidity

@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The calculated correction factor
*/
/**************************************************************************/
float MQ135::getCorrectionFactor(float t, float h) {
	// Linearization of the temperature dependency curve under and above 20 degree C
	// below 20degC: fact = a * t * t - b * t - (h - 33) * d
	// above 20degC: fact = a * t + b * h + c
	// this assumes a linear dependency on humidity
	if (t < 20) {
		return CORA * t * t - CORB * t + CORC - (h - 33.)*CORD;
	}
	else {
		return CORE * t + CORF * h + CORG;
	}
}

/**************************************************************************/
/*!
@brief  Get the resistance of the sensor, ie. the measurement value

@return The sensor resistance in kOhm
*/
/**************************************************************************/
float MQ135::getResistance() {
	int val = analogRead(_pin);	delay(5);
	val += analogRead(_pin);	delay(5);
	val += analogRead(_pin);	delay(5);
	val += analogRead(_pin);	delay(5);
	val += analogRead(_pin);	val /= 5;
	
	if (_pinRef != 255)
	{
		int valRef = analogRead(_pinRef);		
		valRef += analogRead(_pinRef); delay(5);
		valRef += analogRead(_pinRef); delay(5);
		valRef += analogRead(_pinRef); delay(5);
		valRef += analogRead(_pinRef); valRef /= 5;
		return ((float)valRef / (float)val - 1.0)*_fRLOAD;
	}

	return ((1023. / (float)val) - 1.)*_fRLOAD;
}

/**************************************************************************/
/*!
@brief  Get the resistance of the sensor, ie. the measurement value corrected
		for temp/hum

@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The corrected sensor resistance kOhm
*/
/**************************************************************************/
float MQ135::getCorrectedResistance(float t, float h) {
	return getResistance() / getCorrectionFactor(t, h);
}

/**************************************************************************/
/*!
@brief  Get the ppm of CO2 sensed (assuming only CO2 in the air)

@return The ppm of CO2 in the air
*/
/**************************************************************************/
float MQ135::getPPM() {
	return PARA * pow((getResistance() / _fRZERO), -PARB);
}

/**************************************************************************/
/*!
@brief  Get the ppm of CO2 sensed (assuming only CO2 in the air), corrected
		for temp/hum

@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The ppm of CO2 in the air
*/
/**************************************************************************/
float MQ135::getCorrectedPPM(float t, float h) {
	return PARA * pow((getCorrectedResistance(t, h) / _fRZERO), -PARB);
}

/**************************************************************************/
/*!
@brief  Get the resistance RZero of the sensor for calibration purposes

@return The sensor resistance RZero in kOhm
*/
/**************************************************************************/
float MQ135::getRZero() {
	return getResistance() * pow((_fATMOCO2 / PARA), (1. / PARB));
}

/**************************************************************************/
/*!
@brief  Get the corrected resistance RZero of the sensor for calibration
		purposes

@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The corrected sensor resistance RZero in kOhm
*/
/**************************************************************************/
float MQ135::getCorrectedRZero(float t, float h) {
	return getCorrectedResistance(t, h) * pow((_fATMOCO2 / PARA), (1. / PARB));
}


/**************************************************************************/
/*!
@brief  set new RZERO value
purposes

@param[in] newVal  new RZERO value
*/
/**************************************************************************/
void MQ135::setRZero(float newVal)
{
	_fRZERO = newVal;
}

/**************************************************************************/
/*!
@brief  set CO2 level in Atmosphere
purposes

@param[in] newVal  new CO2 Atmos level
*/
/**************************************************************************/
void MQ135::setATMCO2(float newVal)
{
	_fATMOCO2 = newVal;
}

/**************************************************************************/
/*!
@brief  set RLOAD of MQ135 board
purposes

@param[in] newVal  set new RLOAD value
*/
/**************************************************************************/
void MQ135::setRLOAD(float newVal)
{
	_fRLOAD = newVal;
}


/**************************************************************************/
/*!
@brief  set RefPin
purposes
Supply Voltage to MQ135 might not be exactly 5V which you will measure against
If you measure the supply voltage with an extra pin you can get rid of Voltages
in the equation.
(You can use the DO pin if you set the poti where it will never trigger the LED)

@param[in] pin  set refPin
*/
/**************************************************************************/
void MQ135::setRefPin(uint8_t pin)
{
	_pinRef = pin;
}