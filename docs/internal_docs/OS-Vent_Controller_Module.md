**Open Source Ventilator**

Controller Module

V1.0; Sem Lampotang, March 17, 2020; Department of Anesthesiology, Center for Safety, Simulation &amp; Advanced Learning Technologies, University of Florida, Gainesville, Florida, USA

This module is essentially a timer that turns two valves on or off. When the inspiratory valve is open, the exhalation valve is closed and vice versa.

Respiratory rate (RR) = Number of breaths per minute (bpm)

Breath duration = 60 s/RR; e.g., for 10 bpm, breath duration = 60/10 = 6 s

Inspiratory:Expiratory time ratio (I:E ratio) = 1:2 (usually)

Inspiratory time (Ti; time in seconds spent in inspiration phase) = breath duration / (1 + I:E) = 6/(1+2) = 6/3 = 2 seconds

Expiratory time (Te; time in seconds spent in exhalation phase) = breath duration – inspiratory time = 6 - 2 = = 4 seconds

Tidal volume (VT) = volume of gas (ml) delivered during inspiration

For a constant flow inspiration, the flow rate to the patient is VT/(inspiratory time)

| **Input** | **Data Output** | **Physical output** | **Status** |
| --- | --- | --- | --- |
| User selects tidal volume (see General Engineering Specifications) || Gas flows towards patient during the selected inspiratory time |
||| In constant flow design, gas flows to patient at a flow rate of VT/ |
 |
| User selects respiratory rate (see General Engineering Specifications) | Time when inspiration starts and ends | Time when gas starts flowing to patient Time when gas stops flowing to patient | Not claimed; No one working on it yet
|| Time when exhalation starts and ends | Time when gas starts flowing from patient Time when gas stops flowing from patient | No one working on it yet ; No one working on it yet |
| **Electromechanical design (no software)** |
| User selects respiratory rate (see General Engineering Specifications) | Time when inspiration starts and ends | 12 V delivered to inspiratory valve 0 V to exhalation valve | No one working on it yet ; No one working on it yet |
|
 | Time when exhalation starts and ends | 12 V delivered to exhalation valve0 V to inspiratory valve | No one working on it yet ; No one working on it yet |
|
| **Electromechanical design (software)** |
| User selection for tidal volume and respiratory rate (see General Engineering Specifications) | Variable &quot;INSPIRATORY\_VALVE\_ON&quot; is true when inspiration starts&quot;INSPIRATORY\_VALVE\_ON&quot; is false when inspiration end | 12 V delivered to inspiratory valve 0 V to exhalation valve | No one working on it yet ; No one working on it yet |
|
 || Variable &quot;EXHALATION\_VALVE\_ON&quot; is true when exhalation starts&quot;EXHALATION\_VALVE\_ON&quot; is false when exhalation ends | 12 V delivered to exhalation valve0 V to inspiratory valve | No one working on it yet ; No one working on it yet |
| **Pneumatic Design – May require specialized parts that may be out of stock** |
