# Controller Source Code Projects

* **ArduinoVent** The final target code for the Ventilator Controller

* **VentSim** The VentSim purpose is to focus on UI and high level control of the Ventilator. Some particular files will be port to the real Ventilator project and others emulates the native Arduino libraries.

### Very first Prototype (before CSSALT PBC)
![UI](Docs/proto01.JPG)


### CSSALT PCB Schematic (thanks to Ashhar):
[Click here to see the schematic](Docs/schematic.pdf)

### Concept Video
[click here for Concept Video](https://youtu.be/OPbaNoG-sxQ)

### UI Update
[click here for UI Update 1 Video](https://youtu.be/SgtlYgqE2HE)

### UI Update
[click here for UI Update 2 Video](https://youtu.be/RzNPGvQ2CpA)

## Controller
At this point the team building the Ventilator only needs a timer to control breath rate via two valves: inhalation/exhalation valves. Later on we will add a step motor to drive a machanism to squeese a rubber bad/bottle in order to provide positive pressure.

## UI Options
Whoever is producing the Ventilator can opt to use LCD's: 16X2, 20X2 or 20X4. Interface can be I2C or Parallel. 
## UI Initial concept/proposal

### LCD/Buttons Interface
VentSim simulates the LCD and buttons. The LCD displays the following rows:

![UI](Docs/Ventilator_UI_sample.png)

Where the rows represent the following tokes:

 * **First Row**: St (Idle, On, or Error). 
   The last 6 characters on the right displays the breath progress, where "||||||" means lung full and 6 spaces empty lung. 

 * **The other rows** are scrollable and show parameters. In "Normal" mode the **"-"** and **"+"** buttons are use to scroll the parameter list. Pressing and hold the **"-"** button scrolls the list up to the top.
 To change a parameter the user needs to press and hold the **"Set"** button for half+ second; it enters in "Enter" mode; then the selected row starts flashing indicating that the parameter can be changed; the The **"-"** and **"+"** buttons increment or decrement the value; Pressing the **"Set"** button commits the change (and so if times-out) and return the unit to "Normal" mode. 

### Android UI
TBD

