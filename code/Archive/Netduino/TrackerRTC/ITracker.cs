namespace TrackerRTC
{
    public interface ITracker
    {
        /// <summary>
        /// Perform a tracker test by stepping vertical and horizontal positions in 10 degree increments
        /// </summary>
        void Test();

        /// <summary>
        /// Perform active tracking of the sun azimuth and elevation
        /// </summary>
        void Track();

        /// <summary>
        /// The location and calibration of the tracker
        /// </summary>
        Configuration Configuration { get; }

        /// <summary>
        /// State of the tracker as defined by the State enumeration
        /// </summary>
        State TrackerState { get; set; }

        /// <summary>
        /// Current Azimuth of the array
        /// </summary>
        double ArrayAzimuth { get; set; }

        /// <summary>
        /// Current Azimuth of the sun
        /// </summary>
        double SunAzimuth { get; set; }

        /// <summary>
        /// Current Elevation of the array
        /// </summary>
        double ArrayElevation { get; set; }

        /// <summary>
        /// Current Elevation of the sun
        /// </summary>
        double SunElevation { get; set; }

        /// <summary>
        /// Initialize the actuator to determin range
        /// </summary>
        /// <returns></returns>
        bool Initialize();
    }
}