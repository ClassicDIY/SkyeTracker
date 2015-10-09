using System;
using System.IO.Ports;
using System.Threading;


namespace TrackerRTC
{
    public class Program
    {
        public static void Main()
        {
            

            var configuration = new Configuration();
            configuration.Load();
            var tracker = new Tracker(configuration);
            tracker.Initialize();
            //tracker.Test();
            tracker.Track();
            Thread.Sleep(Timeout.Infinite);
        }

    }

}
