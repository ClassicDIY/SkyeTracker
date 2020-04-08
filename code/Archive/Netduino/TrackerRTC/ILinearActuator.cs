namespace TrackerRTC
{
    public interface ILinearActuator
    {
        /// <summary>
        /// Fully extend and retract actuator to initialize the range
        /// </summary>
        void Initialize();

        /// <summary>
        /// Value from the actuator position sensor
        /// take the average of 10 samples, throw out the highest and lowest
        /// </summary>
        double CurrentPosition { get; }

        /// <summary>
        /// Maximum current detected during actuator moves
        /// </summary>
        double MaxCurrent { get; }

        /// <summary>
        /// The position in degrees of the azimuth or elevation when the actuator is fully retracted
        /// </summary>
        double RetractedPosition { get; set; }

        /// <summary>
        /// The position in degrees of the azimuth or elevation when the actuator is fully extended
        /// </summary>
        double ExtendedPosition { get; set; }

        /// <summary>
        /// The most recent position in degrees
        /// </summary>
        double CurrentAngle { get; set; }

        /// <summary>
        /// Calculates angle using sensor position and range
        /// </summary>
        double CalculatedAngle { get; }

        /// <summary>
        /// Move tracker to the requested angle
        /// </summary>
        /// <param name="position"></param>
        void MoveTo(double position);

        /// <summary>
        /// Move array to fully retracted position
        /// </summary>
        void Retract();

        /// <summary>
        /// Move array to fully extended position
        /// </summary>
        void Extend();
    }
}