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
This project is set up to be built with Eclipse Sloeber, which adds Arduino support to the Eclipse IDE. It uses the following Libraries, which can be installed through the library manager:
- Blynk (Version 0.6.1)
- TFT_eSPI (Version 1.4.16)
- Time (Version 1.5.0)

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
You will need all need the components listed below to complete the PCB. All of them can be bought from standard suppliers, including Reichelt, Farnell, Mouser etc.

**Resistors:**
|Value|Package | Quantity | Link |
|--|--|--|--
|2.2k|0603|1
|4.7k|0603|2
|10k|0603|5
|22k|0603|1
|47k|0603|2
|100k|0603|1
220k|0603|2
330k|0603|3
1M|0603|1
4.7M|0603|1

**Capacitors:**
|Value|Package | Quantity | Link |
|--|--|--|--
2.2n|0603|1
10n|0603|2
22n|0603|1
100n|0603|1


**Semiconductors:**
|Value|Package | Quantity | Link |
|--|--|--|--
| **Transistors**|
 IRLML5203|MICRO3|1|[farnell](https://de.farnell.com/infineon/irlml5203trpbf/mosfet-p-kanal-30v-3a-sot23/dp/9103511?st=irlml5203)
 BSS138BK|SOT-23|6|[farnell](https://de.farnell.com/nexperia/bss138bk-215/mosfet-n-kanal-60v-0-36a-sot23/dp/2053833?st=bss138bk)
 **Diodes**|
 BAS21|SOT-23|2|[farnell](https://de.farnell.com/diodes-inc/bas21/schaltdiode-250v-0-25w-sot23-3/dp/1902406?st=bas21)
 **ICs**|
 MCP23017SO|SO28W|1|[farnell](https://de.farnell.com/microchip/mcp23017-e-so/16bit-expander-i-o-i2c-i-f-smd/dp/1332087?st=mcp23017)
 NE555D|SO-8|1|[farnell](https://de.farnell.com/texas-instruments/ne555d/precision-timers-linear/dp/3121191?st=ne555d)
 MCP6002-E/SN|SOIC-8|1|[farnell](https://de.farnell.com/microchip/mcp6002-e-sn/op-verst-rker-1mhz-0-6v-us-4500uv/dp/1332118?st=mcp6002-e/sn)

**Boards:**
|Value|Package | Quantity | Link |
|--|--|--|--
| ESP32 nodeMCU||1|[from far east](https://www.aliexpress.com/item/32867696371.html?spm=a2g0s.9042311.0.0.27424c4dZ5DEit)|

**Connectors:**
|Value|rows|positions|m/f| Quantity | Link |
|--|--|--|--|--|--
pinheader 2,54|1|3|f|5|connecting temperature and flow sensors
pinheader 2,54|1|3|m|5|possibly soldering to temperature and flow sensor wires
pinheader 2,54|1|9|f|2|display
pinheader 2,54|1|9|m|1|display
pinheader 2,54|1|6|f|2|connecting arduino SSR board
pinheader 2,54|1|6|m|1|connecting arduino SSR board
wire to board connectors, RM5 with screws|1|2|f|13|connecting everyhing else [farnell](https://de.farnell.com/camdenboss/ctb1500-2/terminal-block-5mm-2-way/dp/3377326?st=wire%20to%20board%20rm5)


### Hardware off the PCB in the machine
- Arduino SSR board with 4 SSRs (e.g. [Arduino SSR module on Amazon](https://www.amazon.de/Ecloud-4-Channel-Trigger-SSR-Modul-Brett-Arduino-resistivem/dp/B01E6KUMTI/ref=sr_1_4?__mk_de_DE=%C3%85M%C3%85%C5%BD%C3%95%C3%91&dchild=1&keywords=arduino%20ssr%204&qid=1605773053&quartzVehicle=812-409&replacementKeywords=arduino%20ssr&sr=8-4))
- Two SSRs for 230V, switchable by 3-5VDC (called SSR-40 DA) for boiler heater and electric brewing unit heater (e.g. [SSR on Amazon](https://www.amazon.de/KKmoon-24V-380V-SSR-40-Relaismodul-Temperaturregler/dp/B010Q6898C/ref=sr_1_3?__mk_de_DE=%C3%85M%C3%85%C5%BD%C3%95%C3%91&dchild=1&keywords=ssr&qid=1605773227&sr=8-3))
- SPI TFT Display 320x240 Pixels with ILI9341 (e.g. [Display on Amazon](https://www.amazon.de/gp/product/B01E0MGZU2/ref=ppx_yo_dt_b_asin_title_o09_s00?ie=UTF8&psc=1))
- Alternative switches: Brewing switch should be replaced by a 3 fixed positions switch, indicator lights should be replaced by push buttons with LEDs, so that they can serve as indicator lights and inputs at the same time. (I used [those](https://www.aliexpress.com/item/4000454624299.html?spm=a2g0s.9042311.0.0.27424c4dZZ9bgU) instead of the indicator lights (3-6V, momentary version)
- TSIC 306 temperature sensors, 3 pieces
- Flow sensors, 2 pieces (e.g. [from far east](https://www.aliexpress.com/item/32949504762.html?spm=a2g0o.productlist.0.0.4ce346a5MgsR2I&algo_pvid=a8a64f8f-1437-4fca-a7c7-6e23d0d647a1&algo_expid=a8a64f8f-1437-4fca-a7c7-6e23d0d647a1-28&btsid=2100bdf016057823158403471e2893&ws_ab_test=searchweb0_0,searchweb201602_,searchweb201603_))
- 5V power supply
<!--stackedit_data:
eyJoaXN0b3J5IjpbLTI4MDgwMDExMiw2Mzc0NDM5OSwtMjEwMj
kyNTk1MywtNzg4NzIyOTA0LC0xNzcwNTI5NzBdfQ==
-->