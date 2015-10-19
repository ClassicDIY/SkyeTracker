using System;

namespace TrackerRTC
{
    public struct Sun
    {
        public double azimuth;
        public double elevation;
        public double eqTime;
        public double solarDec;
        public double coszen;
        public bool dark;
    }

    public static class SunEquations
    {
        #region Calculations

        // Returns decimal latitude or longitude from deg, min, sec entered in the configuration
        public static double DegreesMinuteSecondDecimalDegrees(string latLong)
        {
            var neg = 0;
            if (latLong[0] == '-')
            {
                neg = 1;
                latLong = latLong.Substring(1);
            }
            var parts = latLong.Split('.');
            double degs = 0;
            if (parts.Length > 0)
                degs = Double.Parse(parts[0]);
            double mins = 0;
            if (parts.Length > 1)
                mins = Double.Parse(parts[1]);
            double secs = 0;
            if (parts.Length > 2)
                secs = Double.Parse(parts[2]);
            if (neg != 1)
            {
                return degs + (mins/60) + (secs/3600);
            }
            if (neg == 1)
            {
                return degs - (mins/60) - (secs/3600);
            }
            return -9999;
        }

        /// <summary>
        /// calculate solar position for the entered date, time and 
        /// location. Results are reported in azimuth and elevation 
        /// (in degrees) and cosine of solar zenith angle.
        /// </summary>
        /// <param name="latitude"></param>
        /// <param name="longitude"></param>
        /// <param name="dateTime"></param>
        /// <param name="zone">Time offset from UTC</param>
        public static Sun CalcSun(double latitude, double longitude, DateTime dateTime, int zone)
        {
            var result = new Sun();

            if ((latitude >= -90) && (latitude < -89.8))
            {
                //"All latitudes between 89.8 and 90 S\n will be set to -89.8."
                latitude = -89.8;
            }
            if ((latitude <= 90) && (latitude > 89.8))
            {
                //"All latitudes between 89.8 and 90 N\n will be set to 89.8."
                latitude = 89.8;
            }
            var jd = JD(dateTime.AddHours(zone));
            var jc = JulianCent(jd);
            var theta = SunDeclination(jc);
            var etime = EquationOfTime(jc);
            var eqTime = etime;
            var solarDec = theta; // in degrees
            result.eqTime = (Math.Floor(100*eqTime))/100;
            result.solarDec = (Math.Floor(100*(solarDec)))/100;

            //double solarTimeFix = (eqTime + 4.0 * _longitude) - 60.0 * _zone;

            var solarTimeFix = (eqTime + 4.0*longitude) - 60.0*zone;
            var trueSolarTime = dateTime.TimeOfDay.Hours*60 + dateTime.TimeOfDay.Minutes + dateTime.TimeOfDay.Seconds/60 + solarTimeFix;
            while (trueSolarTime > 1440)
            {
                trueSolarTime -= 1440;
            }
            var hourAngle = trueSolarTime/4.0 - 180.0;
            if (hourAngle < -180)
            {
                hourAngle += 360.0;
            }
            var haRad = DegreeToRadian(hourAngle);
            var csz = Math.Sin(DegreeToRadian(latitude))*Math.Sin(DegreeToRadian(solarDec)) + Math.Cos(DegreeToRadian(latitude))*Math.Cos(DegreeToRadian(solarDec))*Math.Cos(haRad);
            if (csz > 1.0)
            {
                csz = 1.0;
            }
            else if (csz < -1.0)
            {
                csz = -1.0;
            }
            var zenith = RadianToDegree(Math.Acos(csz));
            var azDenom = (Math.Cos(DegreeToRadian(latitude))*Math.Sin(DegreeToRadian(zenith)));
            double azimuth = 0;
            if (Math.Abs(azDenom) > 0.001)
            {
                var azRad = ((Math.Sin(DegreeToRadian(latitude))*Math.Cos(DegreeToRadian(zenith))) - Math.Sin(DegreeToRadian(solarDec)))/azDenom;
                if (Math.Abs(azRad) > 1.0)
                {
                    if (azRad < 0)
                    {
                        azRad = -1.0;
                    }
                    else
                    {
                        azRad = 1.0;
                    }
                }
                azimuth = 180.0 - RadianToDegree(Math.Acos(azRad));
                if (hourAngle > 0.0)
                {
                    azimuth = -azimuth;
                }
            }
            else
            {
                if (latitude > 0.0)
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
            var exoatmElevation = 90.0 - zenith;
            if (exoatmElevation > 85.0)
            {
                refractionCorrection = 0.0;
            }
            else
            {
                var te = Math.Tan(DegreeToRadian(exoatmElevation));
                if (exoatmElevation > 5.0)
                {
                    refractionCorrection = 58.1/te - 0.07/(te*te*te) + 0.000086/(te*te*te*te*te);
                }
                else if (exoatmElevation > -0.575)
                {
                    refractionCorrection = 1735.0 + exoatmElevation*(-518.2 + exoatmElevation*(103.4 + exoatmElevation*(-12.79 + exoatmElevation*0.711)));
                }
                else
                {
                    refractionCorrection = -20.774/te;
                }
                refractionCorrection = refractionCorrection/3600.0;
            }
            var solarZen = zenith - refractionCorrection;
            if (solarZen < 108.0)
            {
                // astronomical twilight
                result.azimuth = (Math.Floor(100*azimuth))/100;
                result.elevation = (Math.Floor(100*(90.0 - solarZen)))/100;
                if (solarZen < 90.0)
                {
                    result.coszen = (Math.Floor(10000.0*(Math.Cos(DegreeToRadian(solarZen)))))/10000.0;
                }
                else
                {
                    result.coszen = 0.0;
                }
            }
            else
            {
                result.dark = true;
            }
            return result;
        }

        private static double RadianToDegree(double angle)
        {
            return angle*(180.0/Math.PI);
        }

        private static double DegreeToRadian(double angle)
        {
            return Math.PI*angle/180.0;
        }

        private static double DateToJD(int year, int month, int day, int hour, int minute, int second, int millisecond)
        {
            int M = month > 2 ? month : month + 12;
            int Y = month > 2 ? year : year - 1;
            double D = day + hour/24.0 + minute/1440.0 + (second + millisecond*1000)/86400.0;
            int B = 2 - Y/100 + Y/100/4;

            return (int) (365.25*(Y + 4716)) + (int) (30.6001*(M + 1)) + D + B - 1524.5;
        }

        private static double JD(int year, int month, int day, int hour, int minute, int second, int millisecond)
        {
            return DateToJD(year, month, day, hour, minute, second, millisecond);
        }


        private static double JD(DateTime date)
        {
            return DateToJD(date.Year, date.Month, date.Day, date.Hour, date.Minute, date.Second, date.Millisecond);
        }

        private static double JulianCent(double julianDate)
        {
            var T = (julianDate - 2451545.0)/36525.0;
            return T;
        }

        /// <summary>
        /// calculate the distance to the sun in AU
        /// </summary>
        /// <param name="t">number of Julian centuries since J2000.0</param>
        /// <returns> sun radius vector in AUs</returns>
        private static double SunRadVector(double t)
        {
            var v = SunTrueAnomaly(t);
            var e = EccentricityEarthOrbit(t);
            var R = (1.000001018*(1 - e*e))/(1 + e*Math.Cos(DegreeToRadian(v)));
            return R; // in AUs
        }

        /// <summary>
        /// calculate the true anamoly of the sun
        /// </summary>
        /// <param name="t">number of Julian centuries since J2000.0</param>
        /// <returns>sun's true anamoly in degrees</returns>
        private static double SunTrueAnomaly(double t)
        {
            var m = GeomMeanAnomalySun(t);
            var c = SunEqOfCenter(t);
            var v = m + c;
            return v; // in degrees
        }

        /// <summary>
        /// calculate the Geometric Mean Anomaly of the Sun
        /// </summary>
        /// <param name="t">number of Julian centuries since J2000.0</param>
        /// <returns>the Geometric Mean Anomaly of the Sun in degrees</returns>
        private static double GeomMeanAnomalySun(double t)
        {
            var M = 357.52911 + t*(35999.05029 - 0.0001537*t);
            return M; // in degrees
        }

        /// <summary>
        /// calculate the eccentricity of earth's orbit
        /// </summary>
        /// <param name="t">number of Julian centuries since J2000.0</param>
        /// <returns>the unitless eccentricity</returns>
        private static double EccentricityEarthOrbit(double t)
        {
            var e = 0.016708634 - t*(0.000042037 + 0.0000001267*t);
            return e; // unitless
        }

        /// <summary>
        /// calculate the equation of center for the sun
        /// </summary>
        /// <param name="t">number of Julian centuries since J2000.0</param>
        /// <returns>in degrees</returns>
        private static double SunEqOfCenter(double t)
        {
            var m = GeomMeanAnomalySun(t);
            var mrad = DegreeToRadian(m);
            var sinm = Math.Sin(mrad);
            var sin2m = Math.Sin(mrad + mrad);
            var sin3m = Math.Sin(mrad + mrad + mrad);
            var C = sinm*(1.914602 - t*(0.004817 + 0.000014*t)) + sin2m*(0.019993 - 0.000101*t) + sin3m*0.000289;
            return C; // in degrees
        }

        /// <summary>
        /// calculate the declination of the sun
        /// </summary>
        /// <param name="t">number of Julian centuries since J2000.0</param>
        /// <returns>sun's declination in degrees</returns>
        private static double SunDeclination(double t)
        {
            var e = ObliquityCorrection(t);
            var lambda = SunApparentLong(t);
            var sint = Math.Sin(DegreeToRadian(e))*Math.Sin(DegreeToRadian(lambda));
            var theta = RadianToDegree(Math.Asin(sint));
            return theta; // in degrees
        }

        /// <summary>
        /// calculate the apparent longitude of the sun
        /// </summary>
        /// <param name="t">number of Julian centuries since J2000.0</param>
        /// <returns>sun's apparent longitude in degrees</returns>
        private static double SunApparentLong(double t)
        {
            var o = SunTrueLong(t);
            var omega = 125.04 - 1934.136*t;
            var lambda = o - 0.00569 - 0.00478*Math.Sin(DegreeToRadian(omega));
            return lambda; // in degrees
        }

        /// <summary>
        /// calculate the true longitude of the sun
        /// </summary>
        /// <param name="t">number of Julian centuries since J2000.0</param>
        /// <returns>sun's true longitude in degrees</returns>
        private static double SunTrueLong(double t)
        {
            var l0 = GeomMeanLongSun(t);
            var c = SunEqOfCenter(t);
            var O = l0 + c;
            return O; // in degrees
        }

        /// <summary>
        /// calculate the Geometric Mean Longitude of the Sun
        /// </summary>
        /// <param name="t">number of Julian centuries since J2000.0</param>
        /// <returns>the Geometric Mean Longitude of the Sun in degrees</returns>
        private static double GeomMeanLongSun(double t)
        {
            var L0 = 280.46646 + t*(36000.76983 + 0.0003032*t);
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
        /// calculate the difference between true solar time and mean
        /// </summary>
        /// <param name="t">number of Julian centuries since J2000.0</param>
        /// <returns>equation of time in minutes of time</returns>
        private static double EquationOfTime(double t)
        {
            var epsilon = ObliquityCorrection(t);
            var l0 = GeomMeanLongSun(t);
            var e = EccentricityEarthOrbit(t);
            var m = GeomMeanAnomalySun(t);
            var y = Math.Tan(DegreeToRadian(epsilon)/2.0);
            y *= y;
            var sin2l0 = Math.Sin(2.0*DegreeToRadian(l0));
            var sinm = Math.Sin(DegreeToRadian(m));
            var cos2l0 = Math.Cos(2.0*DegreeToRadian(l0));
            var sin4l0 = Math.Sin(4.0*DegreeToRadian(l0));
            var sin2m = Math.Sin(2.0*DegreeToRadian(m));
            var Etime = y*sin2l0 - 2.0*e*sinm + 4.0*e*y*sinm*cos2l0 - 0.5*y*y*sin4l0 - 1.25*e*e*sin2m;
            return RadianToDegree(Etime)*4.0; // in minutes of time
        }

        /// <summary>
        /// calculate the corrected obliquity of the ecliptic
        /// </summary>
        /// <param name="t">number of Julian centuries since J2000.0</param>
        /// <returns>corrected obliquity in degrees</returns>
        private static double ObliquityCorrection(double t)
        {
            var e0 = MeanObliquityOfEcliptic(t);
            var omega = 125.04 - 1934.136*t;
            var e = e0 + 0.00256*Math.Cos(DegreeToRadian(omega));
            return e; // in degrees
        }

        /// <summary>
        /// calculate the mean obliquity of the ecliptic
        /// </summary>
        /// <param name="t">number of Julian centuries since J2000.0 </param>
        /// <returns>mean obliquity in degrees</returns>
        private static double MeanObliquityOfEcliptic(double t)
        {
            var seconds = 21.448 - t*(46.8150 + t*(0.00059 - t*(0.001813)));
            var e0 = 23.0 + (26.0 + (seconds/60.0))/60.0;
            return e0; // in degrees
        }

        #endregion
    }
}