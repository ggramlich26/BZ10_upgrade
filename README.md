# Bezzera BZ10 Upgrade
This Project is a replacement for the original Bezzera BZ10 electronics bringing many new features including the following:
- Temperature regulation for boiler and brewing unit
- Volumetric brewing system based on water flow instead of time only
- Standby functionality
- Automated wake up from standby, so that the machine is heated up already when you get up in the morning
- Display of various temperatures, volume distributed and distribution time
- Preinfusion (time based)
- WIFI: All parameters (temperatures, distribution volume, wake up time, etc.) can be set with a smartphone

## Building the project
## Setting up Blynk
## Code overview
The machine functionality is based on state machines. There are two state machines for boiler and brewing unit temperature control. Another state machine controls the brewing cycle and a last one is responsible for the overall machine state including standby functionality. In order to achieve
<!--stackedit_data:
eyJoaXN0b3J5IjpbLTE3NzQ3NTIxMjgsLTE2NTAxMjgwMDVdfQ
==
-->