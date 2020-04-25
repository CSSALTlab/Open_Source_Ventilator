**Open Source Ventilator to Address Actual/Predicted Ventilator Shortage Worldwide due to COVID19 Pandemic – Distribute Freely**

V1.3 – Sem Lampotang; March 19, 2020, Department of Anesthesiology, Center for Safety, Simulation &amp; Advanced Learning Technologies, University of Florida, Gainesville, Florida, USA

**Engineering Design Specifications**

**Overall design philosophy**

- Open source for use worldwide and contributions from others worldwide
- Adult ventilator (older adults at higher risk)
- Positive pressure volume control ventilation
- Intubated patient
- A bare bones, safe design is better than nothing and what &quot;nothing&quot; means ….
- The supplies and materials will be locally available at hardware stores
- The design(s) will be validated; validation will be documented and transparent; as an academic research lab, CSSALT can do some of the verification and validation
- The design(s) will be modular allowing different modules to be mixed and matched depending on local availability
- Dissemination via Internet [https://simulation.health.ufl.edu/technology-development/open-source-ventilator-project/](https://simulation.health.ufl.edu/technology-development/open-source-ventilator-project/) and GitHub([https://github.com/CSSALTlab/Open\_Source\_Ventilator.git](https://github.com/CSSALTlab/Open_Source_Ventilator.git))

**Assumptions:**

1. FDA will waive clearance for the bare bones design if there is a massive shortage
2. Traditional medical components and supplies used in ventilators will be in short supply
3. Transportation will be impaired/disrupted. Locally available non-medical supplies and equipment readily available at local hardware stores such as sprinkler valves and PVC tubing will be used

**Ventilator Specifications:**


| | **Range** | **Accuracy** | **Settings** |
| --- | --- | --- | --- |
| **Volume Controlled Intermittent Mechanical Ventilation** |
| Tidal volume | 145-800 ml | ± 20% | Increments of 50 ml |
| Respiratory rate | 10 – 30 bpm | ± 2 bpm | 5 fixed settings: 10, 15, 20, 25, 30 bpm |
| I:E ratio; no inspiratory pause | 1:1 or 1:2 |-| 2 fixed settings |
| PEEP | 0 – 30 cm H2O;Default setting 16 cm H2O | ± 1 cm H2O | Adjustable or fixed settings depending on design |
| Air and O2 supplied at 50 ± 5 psig (345 kPa) |
| FiO2 | 0.21 – 1.0 | ± 0.05 | 3 fixed FiO2 settings: 0.21, 0.5, 1.0 |
| Anti-asphyxia valve | Opens at –3 ± 1 cm H2O |
| Over-Pressure Valve | Opens at 30 cm H2O ± 5 cm H2O |
| **Optional** |
| Spontaneous breathing |
| Weaning |
| In-built blender | 0.5 FiO2 |
| Oxygen sensor | 0.21 – 1.0 FiO2 | ± 0.05 FiO2 |

**Modules:**

Pneumatics

Inspiratory valve

Exhalation valve

PEEP valve

Pressure Regulation (optional)

3D print

Body

Accessories

Electronics

Printed circuit board

Microcontroller

Software or Timing circuit

Power Supply

12V DC preferred

User Interface
