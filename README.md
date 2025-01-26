This is a project to make a device that would allow multiple a2dp bluetooth sources (e.g phones) to connect to one output and cycle the active connection after every track change.

I've been working on this project occasionally for about 2 years and have gone through a few attempts to get it to function. 
The first attempt was to use mh-m18 modules controlled by a singal arduino to controll the play back status. 
Sadly the documentation for these chips is lacking and the manufacturing inconsistent on the mute and key pins.

I'm now attempting to use 3 ESP32 chips instead as they support a2dp. 
I believe this is a bit overkill but I do not currently have the skills to make my own bt receiver breakout boards/chips which seems to be
the only other way to control multiple a2dp sinks with one microcontroller.
