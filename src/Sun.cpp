#define PI 3.14159265359

#include <math.h>
#include "Sun.h"

namespace SkyeTracker
{
	float fltMins;
	float fltHrs;
	float fltDayDecimal;
	long centuries;
	long leaps;
	long leapDays;
	long yearDays;
	long monthDays;
	double JulianResult;
	double JulianDay;
	double JulianCentury;
	double _latitude;
	double _longitude;

	int _zone;
	double _azimuth;
	double _elevation;
	double _eqTime;
	double _solarDec;
	double _coszen;
	bool _dark;


	Sun::Sun(double latitude, double longitude, int zone)
	{
		_latitude = latitude;
		_longitude = longitude;

		_zone = ~zone;

		if ((_latitude >= -90) && (_latitude < -89.8))
		{
			//"All latitudes between 89.8 and 90 S\n will be set to -89.8."
			_latitude = -89.8;
		}
		if ((_latitude <= 90) && (_latitude > 89.8))
		{
			//"All latitudes between 89.8 and 90 N\n will be set to 89.8."
			_latitude = 89.8;
		}
	}

	double Sun::azimuth()
	{
		return _azimuth;
	}

	double Sun::elevation()
	{
		return _elevation;
	}

	bool Sun::ItsDark()
	{
		return _dark;
	}


	/// <summary>
	/// calculate solar position for the entered date, time and 
	/// location. Results are reported in azimuth and elevation 
	/// (in degrees) and cosine of solar zenith angle.
	/// </summary>
	/// <param name="dateTime"></param>
	void Sun::calcSun(unsigned int year, unsigned char month, unsigned char day, unsigned char hour, unsigned char minute, unsigned char second)
	{

		int intYear = year;
		centuries = intYear / 100;
		leaps = centuries / 4;
		leapDays = 2 - centuries + leaps;         // note is negative!!
		yearDays = 365.25 * (intYear + 4716);     // days until 1 jan this year
		monthDays = 30.6001* (month + 1);    // days until 1st month
		JulianResult = leapDays + day + monthDays + yearDays - 1524.5;

		fltMins = ((float)minute / 60) / 24;
		fltHrs = (float)hour / 24;
		fltDayDecimal = fltHrs + fltMins;
		JulianDay = (double)JulianResult + fltDayDecimal;
		JulianCentury = (JulianDay - 2451545) / 36525;

		//double jd = JD(dateTime.AddHours(zone));
		//double jc = JulianCent(jd);
		double theta = SunDeclination(JulianCentury);
		double etime = EquationOfTime(JulianCentury);
		double eqTime = etime;
		double solarDec = theta; // in degrees
		_eqTime = (floor(100 * eqTime)) / 100;
		_solarDec = (floor(100 * (solarDec))) / 100;
		double solarTimeFix = eqTime - 4.0*_longitude + 60.0*_zone;
		double trueSolarTime = hour * 60 + minute + second / 60 + solarTimeFix;
		while (trueSolarTime > 1440)
		{
			trueSolarTime -= 1440;
		}
		double hourAngle = trueSolarTime / 4.0 - 180.0;
		if (hourAngle < -180)
		{
			hourAngle += 360.0;
		}
		double haRad = DegreeToRadian(hourAngle);
		double csz = sin(DegreeToRadian(_latitude))*sin(DegreeToRadian(solarDec)) + cos(DegreeToRadian(_latitude))*cos(DegreeToRadian(solarDec))*cos(haRad);
		if (csz > 1.0)
		{
			csz = 1.0;
		}
		else if (csz < -1.0)
		{
			csz = -1.0;
		}
		double zenith = RadianToDegree(acos(csz));
		double azDenom = (cos(DegreeToRadian(_latitude))*sin(DegreeToRadian(zenith)));
		double azimuth = 0;
		if (azDenom > 0.001 || azDenom < -0.001)
		{
			double azRad = ((sin(DegreeToRadian(_latitude))*cos(DegreeToRadian(zenith))) - sin(DegreeToRadian(solarDec))) / azDenom;
			if (azRad > 1.0)
			{
				azRad = 1.0;
			}
			else if (azRad < -1.0)
			{
				azRad = -1.0;
			}
			azimuth = 180.0 - RadianToDegree(acos(azRad));
			if (hourAngle > 0.0)
			{
				azimuth = -azimuth;
			}
		}
		else
		{
			if (_latitude > 0.0)
			{
				azimuth = 180.0;
			}
			else
			{
				azimuth = 0.0;
			}
		}
		if (azimuth < 0.0)
		{
			azimuth += 360.0;
		}
		double refractionCorrection;
		double exoatmElevation = 90.0 - zenith;
		if (exoatmElevation > 85.0)
		{
			refractionCorrection = 0.0;
		}
		else
		{
			double te = tan(DegreeToRadian(exoatmElevation));
			if (exoatmElevation > 5.0)
			{
				refractionCorrection = 58.1 / te - 0.07 / (te*te*te) + 0.000086 / (te*te*te*te*te);
			}
			else if (exoatmElevation > -0.575)
			{
				refractionCorrection = 1735.0 + exoatmElevation*(-518.2 + exoatmElevation*(103.4 + exoatmElevation*(-12.79 + exoatmElevation*0.711)));
			}
			else
			{
				refractionCorrection = -20.774 / te;
			}
			refractionCorrection = refractionCorrection / 3600.0;
		}
		double solarZen = zenith - refractionCorrection;
		if (solarZen < 108.0)
		{
			_dark = false;
			// astronomical twilight
			_azimuth = (floor(100 * azimuth)) / 100;
			_elevation = (floor(100 * (90.0 - solarZen))) / 100;
			if (solarZen < 90.0)
			{
				_coszen = (floor(10000.0*(cos(DegreeToRadian(solarZen))))) / 10000.0;
			}
			else
			{
				_coszen = 0.0;
			}
		}
		else
		{
			_dark = true;
		}
		return;
	}

	double Sun::RadianToDegree(double angle)
	{
		return angle*(180.0 / PI);
	}

	double Sun::DegreeToRadian(double angle)
	{
		return PI*angle / 180.0;
	}


	/// <summary>
	/// calculate the Geometric Mean Longitude of the Sun
	/// </summary>
	/// <param name="t">number of Julian centuries since J2000.0</param>
	/// <returns>the Geometric Mean Longitude of the Sun in degrees</returns>
	double Sun::GeomMeanLongSun(double t)
	{
		double L0 = 280.46646 + t*(36000.76983 + 0.0003032*t);
		while (L0 > 360.0)
		{
			L0 -= 360.0;
		}
		while (L0 < 0.0)
		{
			L0 += 360.0;
		}
		return L0; // in degrees
	}


	/// <summary>
	/// calculate the Geometric Mean Anomaly of the Sun
	/// </summary>
	/// <param name="t">number of Julian centuries since J2000.0</param>
	/// <returns>the Geometric Mean Anomaly of the Sun in degrees</returns>
	double Sun::GeomMeanAnomalySun(double t)
	{
		return 357.52911 + t*(35999.05029 - 0.0001537*t);
	}

	/// <summary>
	/// calculate the equation of center for the sun
	/// </summary>
	/// <param name="t">number of Julian centuries since J2000.0</param>
	/// <returns>in degrees</returns>
	double Sun::SunEqOfCenter(double t)
	{
		double m = GeomMeanAnomalySun(t);
		double mrad = DegreeToRadian(m);
		double sinm = sin(mrad);
		double sin2m = sin(mrad + mrad);
		double sin3m = sin(mrad + mrad + mrad);
		double C = sinm*(1.914602 - t*(0.004817 + 0.000014*t)) + sin2m*(0.019993 - 0.000101*t) + sin3m*0.000289;
		return C; // in degrees
	}

	/// <summary>
	/// calculate the true anamoly of the sun
	/// </summary>
	/// <param name="t">number of Julian centuries since J2000.0</param>
	/// <returns>sun's true anamoly in degrees</returns>
	double Sun::SunTrueAnomaly(double t)
	{
		double m = GeomMeanAnomalySun(t);
		double c = SunEqOfCenter(t);
		double v = m + c;
		return v; // in degrees
	}


	/// <summary>
	/// calculate the eccentricity of earth's orbit
	/// </summary>
	/// <param name="t">number of Julian centuries since J2000.0</param>
	/// <returns>the unitless eccentricity</returns>
	double Sun::EccentricityEarthOrbit(double t)
	{
		double e = 0.016708634 - t*(0.000042037 + 0.0000001267*t);
		return e; // unitless
	}

	/// <summary>
	/// calculate the distance to the sun in AU
	/// </summary>
	/// <param name="t">number of Julian centuries since J2000.0</param>
	/// <returns> sun radius vector in AUs</returns>
	double Sun::SunRadVector(double t)
	{
		double v = SunTrueAnomaly(t);
		double e = EccentricityEarthOrbit(t);
		double R = (1.000001018*(1 - e*e)) / (1 + e*cos(DegreeToRadian(v)));
		return R; // in AUs
	}


	/// <summary>
	/// calculate the true longitude of the sun
	/// </summary>
	/// <param name="t">number of Julian centuries since J2000.0</param>
	/// <returns>sun's true longitude in degrees</returns>
	double Sun::SunTrueLong(double t)
	{
		double l0 = GeomMeanLongSun(t);
		double c = SunEqOfCenter(t);
		double O = l0 + c;
		return O; // in degrees
	}

	/// <summary>
	/// calculate the apparent longitude of the sun
	/// </summary>
	/// <param name="t">number of Julian centuries since J2000.0</param>
	/// <returns>sun's apparent longitude in degrees</returns>
	double Sun::SunApparentLong(double t)
	{
		double o = SunTrueLong(t);
		double omega = 125.04 - 1934.136*t;
		double lambda = o - 0.00569 - 0.00478*sin(DegreeToRadian(omega));
		return lambda; // in degrees
	}



	/// <summary>
	/// calculate the mean obliquity of the ecliptic
	/// </summary>
	/// <param name="t">number of Julian centuries since J2000.0 </param>
	/// <returns>mean obliquity in degrees</returns>

	double Sun::MeanObliquityOfEcliptic(double t)
	{
		double seconds = 21.448 - t*(46.8150 + t*(0.00059 - t*(0.001813)));
		double e0 = 23.0 + (26.0 + (seconds / 60.0)) / 60.0;
		return e0; // in degrees
	}


	/// <summary>
	/// calculate the corrected obliquity of the ecliptic
	/// </summary>
	/// <param name="t">number of Julian centuries since J2000.0</param>
	/// <returns>corrected obliquity in degrees</returns>
	double Sun::ObliquityCorrection(double t)
	{
		double e0 = MeanObliquityOfEcliptic(t);
		double omega = 125.04 - 1934.136*t;
		double e = e0 + 0.00256*cos(DegreeToRadian(omega));
		return e; // in degrees
	}

	/// <summary>
	/// calculate the declination of the sun
	/// </summary>
	/// <param name="t">number of Julian centuries since J2000.0</param>
	/// <returns>sun's declination in degrees</returns>
	double Sun::SunDeclination(double t)
	{
		double e = ObliquityCorrection(t);
		double lambda = SunApparentLong(t);
		double sint = sin(DegreeToRadian(e))*sin(DegreeToRadian(lambda));
		double theta = RadianToDegree(asin(sint));
		return theta; // in degrees
	}



	/// <summary>
	/// calculate the difference between true solar time and mean
	/// </summary>
	/// <param name="t">number of Julian centuries since J2000.0</param>
	/// <returns>equation of time in minutes of time</returns>
	double Sun::EquationOfTime(double t)
	{
		double epsilon = ObliquityCorrection(t);
		double l0 = GeomMeanLongSun(t);
		double e = EccentricityEarthOrbit(t);
		double m = GeomMeanAnomalySun(t);
		double y = tan(DegreeToRadian(epsilon) / 2.0);
		y *= y;
		double sin2l0 = sin(2.0*DegreeToRadian(l0));
		double sinm = sin(DegreeToRadian(m));
		double cos2l0 = cos(2.0*DegreeToRadian(l0));
		double sin4l0 = sin(4.0*DegreeToRadian(l0));
		double sin2m = sin(2.0*DegreeToRadian(m));
		double Etime = y*sin2l0 - 2.0*e*sinm + 4.0*e*y*sinm*cos2l0 - 0.5*y*y*sin4l0 - 1.25*e*e*sin2m;
		return RadianToDegree(Etime)*4.0; // in minutes of time
	}


}