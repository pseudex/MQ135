// Enable debug prints to serial monitor
//#define MY_DEBUG
#define SERIAL_DEBUG 0
// Enable and select radio type attached

#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

#include <MySensors.h>
#include <MQ135.h>
#include <SFE_BMP180.h>
#include <Wire.h>

SFE_BMP180 pressure;
double T, P, p0, a;

#define AIR_PIN A0
#define AIR_PIN_REF A1
#define SWITCH_PIN 5
#define SET_CALIB 6
#define STAY_AWAKE 7

// START MESSAGES

// --> CO2

#define CHILD_ID_RZERO 0
#define CHILD_ID_CO2 1
MyMessage msgRZero(CHILD_ID_RZERO, V_VAR1);
MyMessage msgRZeroMin(CHILD_ID_RZERO, V_VAR2);
MyMessage msgRZeroMax(CHILD_ID_RZERO, V_VAR3);
MyMessage msgRs(CHILD_ID_RZERO, V_VAR4);
MyMessage msgCO2(CHILD_ID_CO2, V_VAR1);
MyMessage msgCO2Min(CHILD_ID_CO2, V_VAR2);
MyMessage msgCO2Max(CHILD_ID_CO2, V_VAR3);

// --> Temp and Pressure

#define CHILD_ID_TEMP 2
#define CHILD_ID_PRESS 3
float temp, press;
float lastTemp, lastPress;
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgPress(CHILD_ID_PRESS, V_PRESSURE);

// --> Humidity

#define CHILD_ID_HUM 4
float humidity = 65.0;
bool reqHum = 1;
MyMessage msgHum(CHILD_ID_HUM, V_HUM);

// END MESSAGES


float Co2ppmMin;
float Co2ppmMax;
float lastCo2ppm;

float rzeroMin;
float rzeroMax;
float lastRzero = 0.0;
float lastRs = 0.0;
bool bRZeroRcvd = 0;
MQ135 gasSensor = MQ135(AIR_PIN,AIR_PIN_REF, 52.7, 404.48);

const unsigned long SLEEP_TIME = 30000UL;

void before()
{
	Co2ppmMin = 10000;
	Co2ppmMax = 0;
	rzeroMin = 10000;
	rzeroMax = 0;

}

void presentation()
{
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("CO2 Sensor", "1.1");

	// Register all sensors to gateway (they will be created as child devices)
	present(CHILD_ID_RZERO, S_CUSTOM);
	present(CHILD_ID_CO2, S_CUSTOM);
	present(CHILD_ID_TEMP, S_TEMP);
	present(CHILD_ID_PRESS, S_BARO);
	present(CHILD_ID_HUM, S_HUM);
}

void setup()
{

	/* add setup code here */
	pinMode(SWITCH_PIN, INPUT);
	pinMode(SET_CALIB, INPUT);
	BmpSetup();
	//gasSensor.setRefPin(AIR_PIN_REF);
}

void receive(const MyMessage &message)
{
	if (SERIAL_DEBUG)
	{
		Serial.println(F("message received"));
		Serial.print(F("Message Command: "));
		Serial.print(message.getCommand());
		Serial.print(F(" Type: "));
		Serial.println(message.type);
	}
	// Zähler für Watth empfangen
	if (message.sensor == CHILD_ID_HUM && message.type == V_HUM)
	{
		humidity = message.getFloat();
		reqHum = 0;
		if (SERIAL_DEBUG)
		{
			Serial.print(F("Humidity received: "));
			Serial.println(humidity);
		}
	}
	if (message.sensor == CHILD_ID_RZERO && message.type == V_VAR1)
	{
		float value = message.getFloat();
		gasSensor.setRZero(value);
		bRZeroRcvd = 1;
		if (SERIAL_DEBUG)
		{
			Serial.print(F("RZero received: "));
			Serial.println(value);
		}
	}
}


float Co2ppm = 0;
uint8_t cycle = 0;
void loop()
{

	if (cycle % 20 == 0)
		reqHum = 1;
	if (reqHum)
	{
		request(CHILD_ID_HUM, V_HUM);
		wait(1000, 1, V_HUM);
	}
	if (!bRZeroRcvd)
	{
		request(CHILD_ID_RZERO, msgRZero.type);
		wait(1000, 1, msgRZero.type);
	}
	loop_bmp(); //Read and send Pressure/temp sensor

	bool bMode = digitalRead(SWITCH_PIN); // 1:=Calibration, 0:=measure
	if (bMode)
	{// Calibration
		float rZero = calibrate();
		if (abs((int)(rZero - lastRzero)) > 0.5)
		{
			if (send(msgRZero.set(rZero, 1)))
				lastRzero = rZero;
			if ((int)rZero == (int)rzeroMin) // sent only if it (may) has been newly set
			{
				wait(75);
				if (!send(msgRZeroMin.set(rzeroMin, 0)))
					rzeroMin = 10000.0; // reset if send not successful
			}
			if ((int)rZero == (int)rzeroMax)
			{
				wait(75);
				if (!send(msgRZeroMax.set(rzeroMax, 0)))
					rzeroMax = 0.0;
			}
		}
	}
	else
	{// Measure
		float Co2ppmAct = measureCo2();
		if (Co2ppm == 0.0)
		{
			Co2ppm = Co2ppmAct;
		}
		else
		{
			Co2ppm += (Co2ppmAct - Co2ppm) / 5.0; // average value by 5 samples
		}
		if (abs(Co2ppm - lastCo2ppm) > 1.0)
		{

			if (send(msgCO2.set(Co2ppm, 1)))
				lastCo2ppm = Co2ppm;
			if ((int)Co2ppmAct == (int)Co2ppmMin) // sent only if it (may) has been newly set
			{
				wait(75);
				if (!send(msgCO2Min.set(Co2ppmMin, 0)))
					Co2ppmMin = 10000.0; // reset if send not successful
			}
			if ((int)Co2ppmAct == (int)Co2ppmMax)
			{
				wait(75);
				if (!send(msgCO2Max.set(Co2ppmMax, 0)))
					Co2ppmMax = 0.0;
			}
		}

		float Rs = gasSensor.getCorrectedResistance(temp, humidity);
		if (abs(Rs - lastRs) > 1.0)
		{
			if (send(msgRs.set(Rs, 1)))
				lastRs = Rs;
		}

	}
	bool calib = digitalRead(SET_CALIB);
	if (calib)
	{
		request(CHILD_ID_RZERO, msgRZero.type);
		wait(1000, 1, msgRZero.type);
	}
	bool stay_awake = digitalRead(STAY_AWAKE);
	if (stay_awake)
		wait(SLEEP_TIME);
	else
		sleep(SLEEP_TIME); //sleep for: sleepTime

	++cycle;
}

void loop_bmp()
{
	getReadings(temp, press);
	if (abs(temp - lastTemp) > 1.0)
	{
		send(msgTemp.set(temp, 1));
		lastTemp = temp;
	}

	if (abs(press - lastPress) > 1.0)
	{
		send(msgPress.set(press, 0));
		lastPress = press;
	}
}

float calibrate()
{
	float rzero1 = gasSensor.getCorrectedRZero(temp, humidity);

	if (rzero1 > rzeroMax)
	{
		rzeroMax = rzero1;
	}

	if (rzero1 < rzeroMin)
	{
		rzeroMin = rzero1;
	}
	return rzero1;
}

float measureCo2()
{
	float Co2ppm = gasSensor.getCorrectedPPM(temp, humidity);
	if (Co2ppm > Co2ppmMax)
	{
		Co2ppmMax = Co2ppm;
	}
	if (Co2ppm < Co2ppmMin)
	{
		Co2ppmMin = Co2ppm;
	}
	return Co2ppm;
}