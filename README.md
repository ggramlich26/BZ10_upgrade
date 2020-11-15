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
You can use the following virtual pins for communication with your espresso machine:
- V1: Boiler temperature in °C (display)
- V2: BU temperature in °C (display)
- V3: Tube temperature in °C (display)
- V4: Boiler target temperature in °C (input)
- V5: Brewing unit target temperature in °C (input)
- V6: Distribution volume in ml (input, integers only)
- V7: Volume offset in ml. This makes up for the water going through the hx 
## Code overview
The machine functionality is based on state machines. There are two state machines for boiler and brewing unit temperature control. Another state machine controls the brewing cycle, one is used for the display, and a last one is responsible for the overall machine state including standby functionality. All state machines are visually shown under `/State machines/State machines.odg`. 
In order to make the program more flexible for other hardware, a hardware abstraction class has been written (`dev.c`). Adapting this class will allow the project to run on a different hardware as well.
Besides machine logic, a faster library for TSIC temperature sensors has been developed.
### Brewing state machine
**not complete yet**
```mermaid
graph LR
idle(IDLE)
refill_boiler(Refill Boiler)
preinfusion_buildup(Preinfusion <br> Buildup)
preinfusion_wait(Preinfusion <br> Wait)
vol_dist(Volumetric <br> Distribution)
man_dist(Manual <br> Distribution)
vol_dist_finished(Volumetric <br> Distribution <br> Finished)
error_tank("Error: <br> H2O Tank Empty")
error_boiler("Error: <br> Boiler Fill Probe")

idle--"boiler empty"-->refill_boiler
refill_boiler--"!boiler empty OR <br> standby"-->idle
idle--"dist_vol AND <br>preinf. wait > 0"-->preinfusion_buildup
preinfusion_buildup--"Preinfusion buildup <br> time reached"-->preinfusion_wait
preinfusion_wait-->vol_dist
idle-->vol_dist
idle-->man_dist
vol_dist-->vol_dist_finished
refill_boiler-->error_boiler
preinfusion_buildup-->error_tank
vol_dist-->error_tank
man_dist-->error_tank

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
eyJoaXN0b3J5IjpbLTE5NzExMjM3MTEsLTE3MTI1NDc4NCwyMT
E0MjYyNzUwLC0xNjUwMTI4MDA1XX0=
-->