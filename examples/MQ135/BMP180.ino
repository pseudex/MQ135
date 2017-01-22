#define ALTITUDE 297.0 // Höhe am Ort

void BmpSetup()
{
	if (pressure.begin())
		Serial.println(F("BMP180 init success"));
}


void getReadings(float &temp, float &press)
{
	processBmp();
	temp = (float)T;	
	press = (float)P;
}

void processBmp()
{
	char status;
	status = pressure.startTemperature();
	if (status != 0)
	{
		// Wait for the measurement to complete:
		delay(status);

		// Retrieve the completed temperature measurement:
		// Note that the measurement is stored in the variable T.
		// Function returns 1 if successful, 0 if failure.

		status = pressure.getTemperature(T);
		if (status)
		{
			// Print out the measurement:
			if (SERIAL_DEBUG)
			{
				Serial.print(F("temperature: "));
				Serial.print(T, 2);
				Serial.println(F(" deg C"));
			}

			// Start a pressure measurement:
			// The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
			// If request is successful, the number of ms to wait is returned.
			// If request is unsuccessful, 0 is returned.

			status = pressure.startPressure(3);
			if (status)
			{
				// Wait for the measurement to complete:
				delay(status);

				// Retrieve the completed pressure measurement:
				// Note that the measurement is stored in the variable P.
				// Note also that the function requires the previous temperature measurement (T).
				// (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
				// Function returns 1 if successful, 0 if failure.

				status = pressure.getPressure(P, T);
				if (status)
				{
					// Print out the measurement:
					if (SERIAL_DEBUG)
					{
						Serial.print(F("absolute pressure: "));
						Serial.print(P, 2);
						Serial.print(F(" mb, "));
						Serial.print(P * 0.0295333727, 2);
						Serial.println(F(" inHg"));
					}

					// The pressure sensor returns abolute pressure, which varies with altitude.
					// To remove the effects of altitude, use the sealevel function and your current altitude.
					// This number is commonly used in weather reports.
					// Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
					// Result: p0 = sea-level compensated pressure in mb

					p0 = pressure.sealevel(P, ALTITUDE); // we're at 1655 meters (Boulder, CO)
					if (SERIAL_DEBUG)
					{
						Serial.print(F("relative (sea-level) pressure: "));
						Serial.print(p0, 2);
						Serial.print(F(" mb, "));
						Serial.print(p0 * 0.0295333727, 2);
						Serial.println(F(" inHg"));
					}

					// On the other hand, if you want to determine your altitude from the pressure reading,
					// use the altitude function along with a baseline pressure (sea-level or other).
					// Parameters: P = absolute pressure in mb, p0 = baseline pressure in mb.
					// Result: a = altitude in m.

					a = pressure.altitude(P, p0);
					if (SERIAL_DEBUG)
					{
						Serial.print(F("computed altitude: "));
						Serial.print(a, 0);
						Serial.println(F("m"));
					}
				}
				else Serial.println(F("error retrieving pressure measurement\n"));
			}
			else Serial.println(F("error starting pressure measurement\n"));
		}
		else Serial.println(F("error retrieving temperature measurement\n"));
	}
	else Serial.println(F("error starting temperature measurement\n"));
}