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
This project is set up to be built with Eclipse Sloeber, which adds Arduino support to the Eclipse IDE.
## Setting your WiFi credentials
## Setting up Blynk
You can use the following virtual pins for communication with your espresso machine:
- V1: Boiler temperature in °C (display)
- V2: BU temperature in °C (display)
- V3: Tube temperature in °C (display)
- V4: Boiler target temperature in °C (Numeric Input widget, values 0-120, step size 0.5)
- V5: Brewing unit target temperature in °C (Numeric Input widget, values 0-110, step size 0.5)
- V6: Distribution volume in ml (Numeric Input widget, values 1-200, step size 1, only integers allowed)
- V7: Volume offset in ml. This makes up for the water going through the hx but not into the cup (Numeric Input widget, values 0-100, step size 1, only integers allowed)
- V8: Boiler heater controller P parameter (Numeric Input widget, values 0.01-100, step size 0.05)
- V9: BU heater controller P parameter (Numeric Input widget, values 0.01-100, step size 0.05)
- V10: Preinfusion buildup time in s (Numeric Input widget, values 0-20, step size 1, fractions would be ok)
- V11: Preinfusion wait time in s (Numeric Input widget, values 0-20, step size 1, fractions would be ok)
- V12: Standby wakup timer: in s after midnight (use Blynk time widget, reset widget or set to 0:00 to disable)
- V13: Standby start time: time in s after which the machine goes into standby mode if no user interaction occurs (us Blynk time widget, reset widget or set to 0:00 to disable)
You can use a SuperChart Widget for V1-V3
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

## The Hardware
There are eagle layout and gerber files available for a PCB designed with this project. You can simply send those files to a manufacturer such as JLCPCB or Seeedstudio. At the moment I also have a few available from my order, that I could have sent to you for very little money. In case you have trouble soldering SMD (surface mounted device) components, we could also discuss having an assembled PCB shipped to you.
### Hardware on the PCB
You will need the following components to assemble the PCB:
Resistors, all 0603:
- 2.2k, 1 piece
- 4.7k, 2 pieces
- 10k, 5 pieces
- 22k, 1 piece
- 47k, 2 pieces
- 100k, 1 piece
- 220k, 2 pieces
- 330k, 3 pieces
- 1M, 1 piece
- 4.7M, 1 piece

Capacitors, all 0603:
- 2.2 nF, 1 piece
- 10 nF, 2 pieces
- 22 nF, 1 piece
- 100 nF 1 piece

Transistors:
- IRLML5203, 1 piece
- BSS138BK, 6 pieces, SOT-23 package

ICs:
- MCP23017SO, 1 piece
- NE555D, 1 piece, SO-8 package

Connectors:
- pinheader 1 row, 3 positions, 5 male and 5 female pieces (for temperature and flow sensors)
- pinheader (m+f) 1 row, 9 positions, 1 male and 2 female piece (for display)
- pinheader (m+f) 1 row, 6 positions, 1 piece (for SSR board)
- connectors RM 5, 1 row, 2 positions, 13 pieces
### Hardware off the PCB in the machine
<!--stackedit_data:
eyJoaXN0b3J5IjpbMTUzODU2NjkxNSwtNTM5ODE0ODAzLDI3OT
g4MDM4MSwtMTcxMjU0Nzg0LDIxMTQyNjI3NTAsLTE2NTAxMjgw
MDVdfQ==
-->