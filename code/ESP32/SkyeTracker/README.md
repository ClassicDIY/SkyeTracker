# ESP32 version of SkyeTracker 

<p align="center">
<img src=https://github.com/ClassicDIY/SkyeTracker/blob/master/Pictures/ESP32-pinout-mapping.png width=500>
</p>

<p>
Please refer to the <a href="https://github.com/ClassicDIY/SkyeTracker/wiki">SkyeTracker wiki</a> for more information.
</p>

# Programming setup for the ESP32 version of SkyeTracker

Once you download extract the ZIP file for this project
the code for the ESP32 implementation is available in the `SkyeTracker-master\code\ESP32\SkyeTracker` folder.

This project uses Visual Studio CODE with the PlatformIO extension.
Here is a video tutorial for the VSCode & PlatformIO setup;
[PlatformIO for ESP32 Tutorial](https://www.youtube.com/watch?v=0poh_2rBq7E)

Once setup with the PlatformIO extension, use File->Open Folder to open the project selecting the `SkyeTracker-master\code\ESP32\SkyeTracker` folder.

The Log level can be set in the platform.ini file by setting `APP_LOG_LEVEL` to one of the following;

* #define ARDUHAL_LOG_LEVEL_NONE       (0)
* #define ARDUHAL_LOG_LEVEL_ERROR      (1)
* #define ARDUHAL_LOG_LEVEL_WARN       (2)
* #define ARDUHAL_LOG_LEVEL_INFO       (3)
* #define ARDUHAL_LOG_LEVEL_DEBUG      (4)
* #define ARDUHAL_LOG_LEVEL_VERBOSE    (5)
