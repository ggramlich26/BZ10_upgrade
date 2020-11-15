# Bezzera BZ10 Upgrade
This Project is a replacement for the original Bezzera BZ10 electronics bringing many new features including the following:
- Temperature regulation for boiler and brewing unit
- Volumetric brewing system based on water flow instead of time only
- Standby functionality
- Automated wake up from standby, so that the machine is heated up already when you get up in the morning
- Display of various temperatures, volume distributed and distribution time
- Preinfusion (time based)
- WIFI: All parameters (temperatures, distribution volume, wake up time, etc.) can be set with a smartphone
- Automated cleaning cycle: Ten cycles of switching distribution on and off for 20 seconds each by pressing only one button

## Building the project
## Setting up Blynk
## Code overview
The machine functionality is based on state machines. There are two state machines for boiler and brewing unit temperature control. Another state machine controls the brewing cycle, one is used for the display, and a last one is responsible for the overall machine state including standby functionality. In order to make the program more flexible for other hardware, a hardware abstraction class has been written (`dev.c`). Adapting this class will allow the project to run on a different hardware as well.
Besides machine logic, a faster library for TSIC temperature sensors has been developed.
### Brewing state machine
```mermaid
graph LR
idle(IDLE)
refill_boiler(Refill Boiler)
preinfusion_buildup(Preinfusion <br> Buildup)
preinfusion_wait(Preinfusion <br> Wait)
vol_dist(Volumetric <br> Distribution)
man_dist(Manual <br> Distribution)
vol_dist_finished(Volumetric <br> Distribution <br> Finished)
error_wat
```
### Boiler heater state machine
```mermaid
graph LR
enabled(Enabled)
disabled(Disabled)
start(Start)
enabled -- Water level low OR standby OR <br> boiler temperature sensor error OR <br>boiler fill probe error--> disabled
disabled -- Water level sufficiently high OR <br> !standby OR <br> !boiler temperature sensor error OR <br> !boiler fill probe error-->enabled
start --> enabled
```
### Brewing unit heater state machine
```mermaid
graph LR
start(Start)
enabled(Enabled)
disabled(Disabled)
enabled --"standbye OR <br> BU temperature sensor error"-->disabled
disabled --"!standbye AND !BU temperature sensor error"-->enabled
start-->enabled
```
### Display state machine
### Machine status state machine
```mermaid
graph LR
start(Start)
running(Running)
standby(Standby)
running --"standby pressed OR <br> no input for too long"-->standby
standby--"any button pressed OR <br> wakeup time reached"-->running
start-->running
```
<!--stackedit_data:
eyJoaXN0b3J5IjpbLTM0ODI2NjAzOSwtMTY1MDEyODAwNV19
-->