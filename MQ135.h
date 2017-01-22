/**************************************************************************/
/*!
@file     MQ135.h
@author   G.Krocker (Mad Frog Labs)
@license  GNU GPLv3

First version of an Arduino Library for the MQ135 gas sensor
TODO: Review the correction factor calculation. This currently relies on
the datasheet but the information there seems to be wrong.

@section  HISTORY

v1.0 - First release
*/
/**************************************************************************/
#ifndef MQ135_H
#define MQ135_H
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif



/// The load resistance on the board
#define RLOAD 1.0
/// Calibration resistance at atmospheric CO2 level
#define RZERO 76.63

/// Parameters for calculating ppm of CO2 from sensor resistance
#define PARA 116.6020682
#define PARB 2.769034857

/// Parameters to model temperature and humidity dependence
#define CORA 0.00035
#define CORB 0.02718
#define CORC 1.39538
#define CORD 0.0018
#define CORE -0.003333333
#define CORF -0.001923077
#define CORG 1.130128205

/// Atmospheric CO2 level for calibration purposes
#define ATMOCO2 404.48



class MQ135 {
 private:
  uint8_t _pin;
  uint8_t _pinRef = 255;
  float _fRZERO = RZERO;
  float _fATMOCO2 = ATMOCO2;
  float _fRLOAD = RLOAD;
 public:
  MQ135(uint8_t pin);
  MQ135(uint8_t pin, float in_rzero,float in_atmCO2);
  MQ135(uint8_t pin, uint8_t pinRef, float in_rzero, float in_atmCO2);
  float getCorrectionFactor(float t, float h);
  float getResistance();
  float getCorrectedResistance(float t, float h);
  float getPPM();
  float getCorrectedPPM(float t, float h);
  float getRZero();
  float getCorrectedRZero(float t, float h);
  void setRZero(float newVal);
  void setATMCO2(float newVal);
  void setRLOAD(float newVal);
  void setRefPin(uint8_t pin);
};
#endif
