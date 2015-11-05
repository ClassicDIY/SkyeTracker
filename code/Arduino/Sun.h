#pragma once
#include <Arduino.h>

namespace SkyeTracker
{
	class Sun
	{

	public:
		static double RadianToDegree(double angle);
		static double DegreeToRadian(double angle);

		double SunRadVector(double t);
		double SunTrueAnomaly(double t);
		double GeomMeanAnomalySun(double t);
		double EccentricityEarthOrbit(double t);
		double SunEqOfCenter(double t);
		double SunDeclination(double t);
		double SunApparentLong(double t);
		double SunTrueLong(double t);
		double GeomMeanLongSun(double t);
		double EquationOfTime(double t);
		double ObliquityCorrection(double t);
		double MeanObliquityOfEcliptic(double t);


	public:
		Sun(double latitude, double longitude, int zone);
		double azimuth();
		double elevation();
		bool ItsDark();
		void calcSun(unsigned int year, unsigned char month, unsigned char day, unsigned char hour, unsigned char minute, unsigned char second);
	};

}