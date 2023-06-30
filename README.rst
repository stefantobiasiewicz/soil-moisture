#Soil moisture ble sensor

**GATT Service UUID:** 0000AAAA-0000-1000-8000-00805F9B34FB

1. **Characteristic "Calibration"**
   - UUID: 0000AAAA-0001-1000-8000-00805F9B34FB
   - Description: This characteristic is used for sending calibration commands from the base station to the device to calibrate soil moisture measurements.
   - Value: Single-byte value representing the calibration command.
     - CAL_START: 0x01 (00000001) - Initiates the calibration process.
     - CAL_LOW: 0x02 (00000010) - Sets the low calibration point.
     - CAL_HIGH: 0x04 (00000100) - Sets the high calibration point.
     - CAL_END: 0x08 (00001000) - Finishes the calibration process.
   - Functions: Write

2. **Characteristic "Time Interval"**
   - UUID: 0000AAAA-0002-1000-8000-00805F9B34FB
   - Description: This characteristic allows setting the time interval at which soil moisture measurement results are transmitted from the device to the base station.
   - Value: Time interval in seconds, represented as an integer.
     - Value range in decimal: 0-65535
     - Value range in hexadecimal: 0x0000-0xFFFF
   - Functions: Write, Read

3. **Characteristic "Soil Moisture Measurements"**
   - UUID: 0000AAAA-0003-1000-8000-00805F9B34FB
   - Description: This characteristic is used for transmitting current soil moisture measurements from the device to the base station.
   - Value: Soil moisture measurement represented as an integer (2 bytes).
     - Value range in decimal: 0-1000 (the value divided by 10 gives the percentage value)
     - Value range in hexadecimal: 0x0000-0x03E8
   - Functions: Read, Notify

4. **Characteristic "Battery Level"**
   - UUID: 0000AAAA-0004-1000-8000-00805F9B34FB
   - Description: This characteristic allows reading the battery level of the device.
   - Value: Battery level represented as an integer (2 bytes).
     - Value range in decimal: 0-1000 (the value divided by 10 gives the percentage value)
     - Value range in hexadecimal: 0x0000-0x03E8
   - Functions: Read
