# Open Source Ventilator

An open-source ventilator project designed for worldwide use and community-driven development. Contributions from developers, engineers, researchers, and healthcare technology enthusiasts are welcome.

## Documentation

For detailed project documentation, setup instructions, and hardware information, visit:

https://cssaltlab.github.io/Open_Source_Ventilator/

---

## Prerequisites

Before compiling and uploading the ventilator code, ensure that all required libraries are installed in your Arduino IDE.

### Required Libraries

The project depends on the following external libraries, which are not included with the Arduino IDE or this repository:

| Library               | Source                                                   |
| --------------------- | -------------------------------------------------------- |
| AllSensors_DLHR       | https://github.com/jeremycole/AllSensors_DLHR            |
| BMP280 Sensor Library | https://itbrainpower.net/downloadables/sSense-BMx280.zip |

---

## Library Installation

### Installing AllSensors_DLHR

1. Download the ZIP file from the AllSensors_DLHR repository.

2. Open the Arduino IDE.

3. Navigate to:

   `Sketch → Include Library → Add .ZIP Library...`

4. Select the downloaded ZIP file.

5. Wait for the installation to complete.

### Installing BMP280 Sensor Library

1. Download the `sSense-BMx280.zip` file.

2. Open the Arduino IDE.

3. Navigate to:

   `Sketch → Include Library → Add .ZIP Library...`

4. Select the downloaded ZIP file.

5. Wait for the installation to complete.

---

## Verifying Installation

After installing both libraries:

1. Restart the Arduino IDE.

2. Open the ventilator project.

3. Click **Verify** (✓) or select:

   `Sketch → Verify/Compile`

4. Ensure that the code compiles without errors.

---

## Common Compilation Errors

### Error: AllSensors_DLHR.h: No such file or directory

**Cause:** The AllSensors_DLHR library is not installed.

**Solution:** Install the library using the steps described above.

---

### Error: BMP280.h: No such file or directory

**Cause:** The BMP280 library is missing.

**Solution:** Install the BMP280 Sensor Library and restart the Arduino IDE.

---

### Error: Multiple Libraries Found

**Cause:** Multiple versions of the same library exist in your Arduino libraries folder.

**Solution:** Remove duplicate library installations and keep only one version.

---

### Error: Board Not Supported

**Cause:** Incorrect Arduino board selected.

**Solution:** Select the appropriate board from:

`Tools → Board`

---

## Contributing

We welcome contributions from the community. Areas where contributors can help include:

* Documentation improvements
* Hardware setup guides
* Wiring diagrams
* Bug fixes
* Code optimization
* Testing and validation
* Feature enhancements

Please fork the repository, create a feature branch, and submit a pull request with a clear description of your changes.

---

## License

Please refer to the repository license for usage and distribution information.

