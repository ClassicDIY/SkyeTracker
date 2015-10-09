namespace TrackerRTC
{
    /// <summary>
    /// Various states of the tracker
    /// </summary>
    public enum State
    {
        ClockSet,
        Starting,
        InitializingActuator,
        Standby,
        Testing,
        Tracking,
        Dark
    }
}