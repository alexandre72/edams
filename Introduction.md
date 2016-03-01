# Introduction #

EDAMS is a Linux application to allow house domotics handling. EDAMS have nice but efficients user interfaces to improve your's domotics experience.

EDAMS support:

  * xPL protocol.
  * Data handling via Cosm.com(ex-pachube.com).
  * Voice support via voicerss.org.
  * Location handling with geolocalization service.
  * Widgets for sensor.basic show your device's data in a nice way, can send sensor.request xpl-cmnd by a simple mouse click(and get updated values by feedback)!
  * Widgets for control.basic to sens cmnd to xPL devices(like light on a lamp...).
  * Virtual widgets that don't rely to xPL service(like mail checker, clock, weather...).
  * Global view to show all your widgets grouped by location, with the ability to move them!
  * Actions(scenario-like) to perfoms some predefined actions on some condition(send a mail when temperature is >= 13, light on led bulb when night occurs...).
  * Scheduler editor. You can performs standard EDAMS actions(CMND xPL, exec external programs...). Scheduler editor uses crontab(without breaking your current crons entries).

To use EDAMS, you'll needs:

  * EFLs http://www.enlightenment.org
  * xPL HUB http://www.xpl4java.org/xPL4Linux/ or https://github.com/beanz/xpl-perl/

See your own distribution manager for these package, they should be available. Currently, EDAMS isn't available in a packaging version, you'll need to do the following step:

  * svn checkout http://edams.googlecode.com/svn/trunk/ edams-read-only
  * cd edams-read-only
  * sh autogen.sh
  * make
  * sudo make install

For hardware parts, its depends of your xPL networks. In my house, I'm using:
  * Arduino with xPL library to make your own xPL devices: http://connectingstuff.net/blog/xpl-arduino/

But you can't wathever you want, the only requirement is xPL talking capable devices.