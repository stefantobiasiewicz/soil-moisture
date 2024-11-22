#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "logic_control.h"

/*
   Voltage divider with an NTC resistor:

                 +V_ref
                   |
                  [R_f]
                   |
                   +------- V_out (U_adc)
                   |
                  [NTC]
                   |
                  GND

   - R_f: fixed resistor with a known value.
   - NTC: thermistor with resistance that changes based on temperature.
   - V_ref: reference voltage.
   - V_out (U_adc): output voltage from the divider, measured by the ADC.

   According to Ohm's law, the output voltage V_out (U_adc) across the divider is given by:

      U_adc = V_ref * (R_ntc / (R_ntc + R_f))

   Where:
   - U_adc is the output voltage from the divider (measured by the ADC),
   - V_ref is the reference voltage supplied to the circuit,
   - R_ntc is the current resistance of the NTC thermistor,
   - R_f is the value of the fixed resistor.

   To calculate the NTC resistance based on U_adc, you can rearrange the formula:

      R_ntc = R_f * (U_adc / (V_ref - U_adc))

   This allows us to determine the resistance of the NTC thermistor, and using its characteristic (e.g., beta coefficient and resistance at 25°C), we can then calculate the temperature.
*/


#define T_25 298.15   // 25 degrees Celsius in Kelvin
#define T_0 273.15    // 0 degrees Celsius in Kelvin
#define PRECISION 3   // Precision for floating-point calculations


/**
 * base on https://www.giangrandi.org/electronics/ntc/ntc.shtml
 */
static int calculate_temperature_from_U(float U_adc, float beta, float R_25, float U_ref, float R_f, float* temperature, float* R_ntc, float* I_ntc, float* P_ntc) {
    if (U_ref <= 0) {
        printf("Enter a valid reference voltage U_ref.\n");
        return 0;
    }
    if (R_f <= 0) {
        printf("Enter a valid fixed resistor R_f.\n");
        return 0;
    }
    if (U_adc <= 0 || U_adc >= U_ref) {
        printf("Enter a valid ADC voltage U_adc.\n");
        return 0;
    }
    if (beta <= 0) {
        printf("Enter a valid NTC constant beta.\n");
        return 0;
    }
    if (R_25 <= 0) {
        printf("Enter a valid nominal resistance R_25.\n");
        return 0;
    }

    // First, calculate R_ntc using the voltage divider equation
    *R_ntc = R_f * U_adc / (U_ref - U_adc);

    // Ensure we won't divide by 0 in the next calculation
    if (*R_ntc <= (R_25 * exp(-beta / T_25))) {
        printf("Enter a valid ADC voltage U_adc, it cannot be too low.\n");
        return 0;
    }

    // Calculate temperature using the formula
    *temperature = 1.0 / ((log(*R_ntc / R_25) / beta) + (1.0 / T_25)) - T_0;

    // Compute current and power in the NTC
    *I_ntc = U_adc / *R_ntc;
    *P_ntc = U_adc * (*I_ntc);

    // Convert current to mA and power to mW
    *I_ntc *= 1000;
    *P_ntc *= 1000;

    return 1;
}


/*
float U_adc = 1.5; // Example value
float beta = 3950; // Example value
float R_25 = 10000; // Example value
float U_ref = 3.3; // Example value
float R_f = 10000; // Example value

float temperature, R_ntc, I_ntc, P_ntc;

if (calculate_temperature_from_U(U_adc, beta, R_25, U_ref, R_f, &temperature, &R_ntc, &I_ntc, &P_ntc)) {
    printf("Temperature: %.5f°C\n", temperature);
    printf("R_ntc: %.5f Ohms\n", R_ntc);
    printf("I_ntc: %.5f mA\n", I_ntc);
    printf("P_ntc: %.5f mW\n", P_ntc);
}
*/


/**
 * @return value i celcius
 */
float ntc_calcualte_temperatrue(float adc_mv, int vcc_mv) {
    const float beta = 3950; 
    const float R_25 = 10000;
    const float U_ref = 3.294; 
    const float R_f = 10000; 

    float temperature = 0, R_ntc, I_ntc, P_ntc;

    calculate_temperature_from_U(adc_mv/1000, beta, R_25, U_ref, R_f, &temperature, &R_ntc, &I_ntc, &P_ntc);

    return temperature;
}



// Stałe współczynniki z podanego wzoru
#define A -452.23
#define B 2790.2


/**
 * @return soil moisture in percetage
 */
float capacitive_sensor_calculate_moisture(uint16_t raw_value, soil_moisture_calib_data_t calibration_data) {
    // Ograniczenie raw_value do zakresu kalibracji
    if (raw_value < calibration_data.dry_value) {
        raw_value = calibration_data.dry_value;
    } else if (raw_value > calibration_data.wet_value) {
        raw_value = calibration_data.wet_value;
    }

    // Odwrócenie interpolacji liniowej: raw_value z zakresu [wet_value, dry_value] na [100.0, 0.0]
    float moisture_percentage = 100.0f * (raw_value - calibration_data.wet_value) 
                                / (float)(calibration_data.dry_value - calibration_data.wet_value);

    // Ograniczenie wyniku do zakresu [0.0, 100.0]
    if (moisture_percentage < 0.0) {
        moisture_percentage = 0.0;
    } else if (moisture_percentage > 100.0) {
        moisture_percentage = 100.0;
    }

    return moisture_percentage;
}


/**
 * took from https://forum.arduino.cc/t/calculate-battery-percentage-of-alkaline-batteries-using-the-voltage/669958/16
 * @return battery in percetage
 */
float vbat_calculate_battery(uint16_t raw_value) {
    float v = (float) raw_value / 2000;

    if (v >= 1.55)
        return 100.0; //static value
    else if (v <= 0)
        return 0.0; //static value
    else if (v > 1.4)
        return 60.60606*v + 6.060606; //linear regression
    else if (v < 1.1)
        return 8.3022*v; //linear regression
    else
        return 9412 - 23449*v + 19240*v*v - 5176*v*v*v; // cubic regression
}

/**
 * @return if device should update display/send data
 */
bool check_parameters_changes(float soil_moisture_percent, float temperature_ground, float battery, uint64_t up_time_ms) {

    return true;
}


/**
 * @return next wake up time for checking parameters base on changes detected by @see check_parameters_changes(...) function
 */
int get_next_wakeup_time_ms() {
    return 5*60000;
}