# canbus_autopilot
this is a canbus-connected marine autopilot, composed of 3 main parts:
* an IMU, implementing a MARG algorithm and transmitting attitude and rate of turn PGNs
* a display unit, allowing the user to display and change parameters of the autopilot
* a command unit, which is the autopilot itself. It drives the rudder's motor (in my case, and old Autohelm 2000 tiller).

These parts are built around a pic18f26j50, for its low power and native can support. The CAN protocol is based on J1939 and reuses NMEA2000 PGNs when possible.

There is also wxpilot, a wxwidget/socketcan utility which allows to show and change some of the autopilot's parameters from a unix computer (see my marine_charplotter repository)

The display unit can also decode and display navigation PGNs sent by my nmea2000_pi OpenCPN plugin.

I use this hardware for sailing since several years now
