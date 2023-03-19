# Bezzera BZ10 Upgrade
This Project is a replacement for the original Bezzera BZ10 electronics bringing many new features including the following:
- Temperature regulation for boiler and brewing unit
- Volumetric brewing system based on water flow instead of time only
- Standby functionality
- Automated wake up from standby, so that the machine is heated up already when you get up in the morning
- Display of various temperatures, volume distributed and distribution time
- Preinfusion (time based)
- WIFI: All parameters (temperatures, distribution volume, wake up time, etc.) can be set with a smartphone or computer
- Automated cleaning cycle: Ten cycles of switching distribution on and off for 20 seconds each by pressing only one button

## Building the project
This project is set up to be built with Eclipse Sloeber, which adds Arduino support to the Eclipse IDE. It uses the following Libraries, which can be installed through the library manager:
- TFT_eSPI (Version 1.4.16)
- Time (Version 1.5.0)
- MCP23017 (Version	1.1.2), not Adafruit MCP23017!
- PID (Version 1.2.0)
- ADS115_WE (Version 1.3.6)
- ArduinoJson (Version 6.19.4)
- ESPmDNS
- SPIFFS

Some libraries are also added as source code. If you use the Arduino IDE, please move all files from the libraries/library name/src/ directly into your project directory. If you use Sloeber, you might have to add the library paths in the project settings: Right click your project/Properties/"C/C++ Build"/Settings. In the Tool Settings Tab, expand the compiler and click Include Folders. Add the following paths ```"${workspace_loc:/BZ10_upgrade/libraries/AsyncTCP/src}"``` and ```"${workspace_loc:/BZ10_upgrade/libraries/ESPAsyncWebServer/src}"``` under each of the three compilers.

**Board settings:** You need to [install the ESP32 board for Sloeber](http://wiki.lauerbach.de/wiki/view.do?page=ESP32Entwicklungsumgebungen). Select NodeMCU-32S as board, Upload Protocol: Default, Flash Frequency: 40MHz, Upload Speed: 115200

***Windows:*** Under windows you need to make sure you have make command installed (e.g. through MinGW64: select mingw32-make package). You might also have to [install CP2102 drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) if you get a timeout error when trying to upload your sketch.

**Before building** 
You will additionally need to modify User_Setup.h in the TFT_eSPI library folder. Uncomment `#define ILI9341_DRIVER` (line 39) and make sure all other lines in this block are commented out. Then search for the uncommented pin definitions and change them to the following:
```C++
#define TFT_DC 		23
#define TFT_CS 		17
#define	TFT_MOSI	32
#define	TFT_SCLK	33
#define	TFT_MISO	14
#define TFT_RST		16
```

## Setting your WiFi credentials
The machine can be configured to connect to your own home WiFi network via the web interface:
 1. Turn the machine off.
 2. Set the brewing switch to manual mode, hold both buttons pressed (This step resets the wifi configuration. You can omit it if you have a fresh setup).
 3. Turn the machine on while holding the buttons pressed. You can release them once the screen is ready.
 4. Wait for 30 seconds
 5. Connect with any device to the WiFi network "BZ10" with no password required.
 6. Open your web browser and type 192.168.4.1 into address bar, hit enter.
 7. Now you will be presented a prompt to enter the new SSID (your WiFi name) and password. You can also set your machines Bonjour name. Finish by hitting the "Set" button.

## Connecting to the machine
You can connect to the machine with a browser on any device. You can use your computer, smartphone or tablet, for example. Please note: The machine currently **only supports http and not https**. If you cannot connect, you might have to change the line ine your browser to http://your-ip. There are multiple ways to connect:
### Access point mode
When the machine cannot establish a connection to a wifi network, it automatically creates a hotspot after 30 seconds. Simply connect to the network BZ10 with your device. In the browser type in 192.168.4.1 to go to your machine and change the settings.
### Local WiFi
If the machine is connected to your local WiFi network (little wifi icon shown in the bottom), you can reach it by typing its IP in your browser (use your router configuration website to find out what that is and to make it always the same). Alternatively, you can find it with Bonjour. This works out of the box on Apple and Linux machines and requires an extra [installation from the apple resources](https://support.apple.com/downloads/bonjour-for-windows) on windows. You can then reach the machine by typing bz10.local into your browser. If you want to change the address to something else than bz10, you can modify the Bonjour name on the machine website.
## Usage
Your new espresso machine has a lot more functions than before. This is how you can use them
- **Standard manual distribution:** Flip your distribution switch in the manual distribution position for manual dosing and cooling flush
- **True volumetric distribution:** Flip your distribution switch in the volumetric distribution position for automatic dosing. You can set the desired volume via the website. You will also need to set an offset volume that compensates for the water being moved into the brewing unit but not going through your coffee powder. The best way to do so is to measure the difference between set and actual distribution volume and entering this value via Blynk.
- **Preinfusion:** If you set a preinfusion buildup and wait time on the website, volumetric distribution will come with a preinfusion. This is achieved by turning on the pump for the preinfusion buildup time and then waiting for the preinfusion wait time before starting the actual extraction.
- **Cooling flush:** Since the BZ10 is a HX machine, you will still need to perform a cooling flush just like before the upgrade. But in order to make this process more repeatable, you get a timer showing the time passed after your cooling flush. In order to get this functionality, short press the right button before starting your cooling flush. You will see, that the timer starts counting up once you finished your cooling flush.
- **Standby:** Press the left button shortly to enter standby mode. You can also define a period of time after which the machine will automatically enter standby if it has not been used via Blynk.
- **Automated wake up from standby:** Tired of having to wait for half an hour after getting up until you can finally brew your coffee? Just turn your espresso machine on and press the standby button before going to bed. Now you can set a wake up time via the website. If you set it to half an hour before your alarm goes off, your machine will be all ready for your coffee first thing in the morning. This feature only works if wifi is enabled and the machine has connection to the internet, since it needs to get the current time from a remote server.
- **Automated cleaning cycle:** Cleaning is boring and time consuming task. But now you can save a lot of time. Pressing the right button for 3s or more will start an automated cleaning cycle: Your machine will build up pressure for 20s and then release it for another 20s. This process is repeated 10 times.
- **Disable WiFi:** You got everything set up and don't want your machine to be in the internet any more? Nothing easier than that. Just press the left button for 3s or longer. Now you can see the wifi symbol disappearing from the display and once you restart your machine it won't even connect to your WiFi network any more. To enable WiFi again, just press the left button for 3s or longer again. 
- **Display:** Pretty obvious but you now have a display showing various temperatures, the volume distributed and the time your distribution took (preinfusion time not included). The red circle shows the boiler temperature, the orange circle on the right shows the brewing head temperature and the blue circle in the top shows one additional temperature. I attached this sensor to the tube leading from the heat exchanger to the brewing head.

## Code overview
The machine functionality is based on state machines. There are two state machines for boiler and brewing unit temperature control. Another state machine controls the brewing cycle, one is used for the display, and a last one is responsible for the overall machine state including standby functionality. All state machines are visually shown under `/State machines/State machines.odg`. 
In order to make the program more flexible for other hardware, a hardware abstraction class has been written (`dev.c`). Adapting this class will allow the project to run on a different hardware as well.
Besides machine logic, a faster library for TSIC temperature sensors has been developed.
### Brewing state machine
### Boiler heater state machine
### Brewing unit heater state machine
### Display state machine
### Machine status state machine


## The Hardware
There are eagle layout and gerber files available for a PCB designed with this project. You can simply send those files to a manufacturer such as JLCPCB or Seeedstudio. At the moment I also have a few available from my order, that I could have sent to you for very little money. In case you have trouble soldering SMD (surface mounted device) components, we could also discuss having an assembled PCB shipped to you.
### Hardware on the PCB
You will need all need the components listed below to complete the PCB. All of them can be bought from standard suppliers, including Reichelt, Farnell, Mouser etc.

**Resistors:**
|Value|Package | Quantity | Link |
|--|--|--|--
|2.2k|0603|1
|4.7k|0603|2
|10k|0603|2
|22k|0603|3
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
100n|0603|2


**Semiconductors:**
|Name|Package | Quantity | Link |
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
  ADS1115|MSOP10|1|[e.g. from far east](https://www.aliexpress.com/item/32809831068.html?spm=a2g0s.9042311.0.0.27424c4daHnzqu)

**Boards:**
|Name|Package | Quantity | Link |
|--|--|--|--
| ESP32 nodeMCU with external antenna||1|[from far east](https://www.aliexpress.com/item/4000155919030.html?spm=a2g0s.9042311.0.0.27424c4d4ySog2)|

**Power supply:**
|Name|Package | Quantity | Link |
|--|--|--|--
| HLK-PM01 (5V power supply)||1|[from far east](https://www.aliexpress.com/item/4000534910455.html?spm=a2g0s.9042311.0.0.2f024c4dAwDw9T)|

**Connectors:**
|Name|rows|positions|m/f| Quantity | Comment |
|--|--|--|--|--|--
pinheader 2.54mm|1|2|f+m|4|boiler SSR, boiler probe, NTC, tank switch
pinheader 2.54mm|1|3|f+m|5|connecting TSIC temperature and flow sensors
pinheader 2.54mm|1|6|f+m|2|connecting arduino SSR board
pinheader 2.54mm|1|8|f+m|2|connecting switches and LEDs
pinheader 2.54mm|1|9|f+m|2|display
wire to board connectors, RM5 with screws|1|2|f|1|230V connector [farnell](https://de.farnell.com/camdenboss/ctb1500-2/terminal-block-5mm-2-way/dp/3377326?st=wire%20to%20board%20rm5)


### Hardware off the PCB in the machine
- Arduino SSR board with 4 SSRs (e.g. [Arduino SSR module on Amazon](https://www.amazon.de/Ecloud-4-Channel-Trigger-SSR-Modul-Brett-Arduino-resistivem/dp/B01E6KUMTI/ref=sr_1_4?__mk_de_DE=%C3%85M%C3%85%C5%BD%C3%95%C3%91&dchild=1&keywords=arduino%20ssr%204&qid=1605773053&quartzVehicle=812-409&replacementKeywords=arduino%20ssr&sr=8-4))
- One SSRs for 230V, switchable by 3-5VDC (called SSR-40 DA) for boiler heater and electric brewing unit heater (e.g. [SSR on Amazon](https://www.amazon.de/KKmoon-24V-380V-SSR-40-Relaismodul-Temperaturregler/dp/B010Q6898C/ref=sr_1_3?__mk_de_DE=%C3%85M%C3%85%C5%BD%C3%95%C3%91&dchild=1&keywords=ssr&qid=1605773227&sr=8-3))
- SPI TFT Display 320x240 Pixels with ILI9341 (e.g. [Display on Amazon](https://www.amazon.de/gp/product/B01E0MGZU2/ref=ppx_yo_dt_b_asin_title_o09_s00?ie=UTF8&psc=1))
- Alternative switches: Brewing switch should be replaced by a 3 fixed positions switch, indicator lights should be replaced by push buttons with LEDs, so that they can serve as indicator lights and inputs at the same time. (I used [those](https://www.aliexpress.com/item/4000454624299.html?spm=a2g0s.9042311.0.0.27424c4dZZ9bgU) instead of the indicator lights (3-6V, momentary version) and [those](https://www.aliexpress.com/item/4000028451024.html?spm=a2g0s.9042311.0.0.27424c4dZZ9bgU) for the original two push buttons (5V, latching fixation, blue LED, 2 positions for on/off, 3 positions for brewing switch). If you want to use those switches for on/off, you will need to add a relay in the power supply chain because they are not strong enough for the entire espresso machine power!
- TSIC 306 temperature sensors, 3 pieces
- Flow sensors, 2 pieces (e.g. [from far east](https://www.aliexpress.com/item/32949504762.html?spm=a2g0o.productlist.0.0.4ce346a5MgsR2I&algo_pvid=a8a64f8f-1437-4fca-a7c7-6e23d0d647a1&algo_expid=a8a64f8f-1437-4fca-a7c7-6e23d0d647a1-28&btsid=2100bdf016057823158403471e2893&ws_ab_test=searchweb0_0,searchweb201602_,searchweb201603_))
- Quite some wiring. I chose Oelflex Heat for its heat resistance up to 180°C (purchased from Conrad electronics)

