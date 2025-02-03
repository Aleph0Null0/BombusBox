#include <Arduino.h>
#include <math.h>
//#include <cmath>

// Thermistor parameters from the datasheet
#define RTref 4000
#define B 3568
// Series resistor value = 10 kΩ
#define R 10000
#define T0 298.15
#define num_thermistors 5
#define instr_gain (1+(49.4/42.2))
#define HEATER_PIN D5
#define COOLER_PIN D6
constexpr int THERMISTOR_PINS[num_thermistors] = {A0, A1, A2, A3, A4};
const double resistance_ratio = 4.0/14.0;
float TEMPERATURE_ARRAY[num_thermistors] = {0, 0, 0, 0, 0};
int COUNTER_ARRAY[num_thermistors] = {0, 0, 0, 0, 0};
String TEMPERATURE_LABELS[num_thermistors] = {"HEATER", "COOLER", "OUTSIDE", "ELECTRONICS", "INSIDE"};

// Variables for thermistor calculation
float thermistor_resistance, resistor_voltage, ln, Tx, thermistor_voltage;

void setup() {
    // Setup serial communication
    Serial.begin(9600);
    digitalWrite(3, HIGH);
    // Convert T0 from Celsius to Kelvin
}

float get_temperature(const int thermistor_pin) {
    const float thermistor_voltage = (5.00 / 1023.00) * analogRead(thermistor_pin);
    //Serial.print(thermistor_voltage);
    //Serial.println();
    const float differential_voltage = thermistor_voltage/instr_gain;
    //Serial.print(differential_voltage);
    //Serial.println();
    const float resistance_factor = (differential_voltage/5) + (4.0/14);
    /*Serial.print(resistance_ratio);
    Serial.println();
    Serial.print(resistance_factor);
    Serial.println();*/
    //const float resistor_voltage = 5.00 - differential_voltage;
    //const float resistance_factor = RTref / (R + RTref);
    const float thermistor_resistance = R*(resistance_factor / (1 - resistance_factor));
    //Serial.print(thermistor_resistance);
    //Serial.println();
    //return (1 / ((ln / B) + (1 / T0))) - 273.15;
    return (1 / (log(thermistor_resistance/R)/B + 1/T0)) - 273.15;
}

void get_temperature_array(int array_size, const int pin_array[]) {
    for (int i = 0; i < array_size; i++) {
        TEMPERATURE_ARRAY[i] = get_temperature(pin_array[i]);
    }
}

void update_counter_array(const int array_size) {
    for (int i = 0; i < array_size; i++) {
        if (TEMPERATURE_ARRAY[i] >= 33) { COUNTER_ARRAY[i]++;}
        else if (TEMPERATURE_ARRAY[i] <= 27) { COUNTER_ARRAY[i]--;}
        else {
            if (COUNTER_ARRAY[i] > 0) { COUNTER_ARRAY[i]--; }
            else if (COUNTER_ARRAY[i] < 0) { COUNTER_ARRAY[i]++; }
        }
    }
}

void start_heating() {

}
void start_cooling() {}


void loop() {
    get_temperature_array(num_thermistors, THERMISTOR_PINS);
    update_counter_array(num_thermistors);
    //TODO: Implement evaluation of counter array, and adjust signal to heater/cooler accordingly

    if (COUNTER_ARRAY[0] > 0) {start_heating();}
    else if (COUNTER_ARRAY[0] < 0) {start_cooling();}
    //TODO: Run an atmospheric cycling routine at a given time interval

    for (int i=0;i<5;i++) {
        Serial.print("Temperature ");
        Serial.print(TEMPERATURE_LABELS[i]);
        Serial.print(": ");
        Serial.print(TEMPERATURE_ARRAY[i]);
        Serial.print("C ");

    }
    Serial.println();

    delay(500);
}