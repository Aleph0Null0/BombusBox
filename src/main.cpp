#include <Arduino.h>
// Thermistor parameters from the datasheet
#define RTreference 10000
#define B 3988
// Series resistor value = 10 kΩ
#define R 10000
#define num_thermistors 5

#define HEATER_PIN D5
#define COOLER_PIN D6
constexpr int THERMISTOR_PINS[num_thermistors] = {A0, A1, A2, A3, A4};

float TEMPERATURE_ARRAY[num_thermistors] = {0, 0, 0, 0, 0};
int COUNTER_ARRAY[num_thermistors] = {0, 0, 0, 0, 0};
String TEMPERATURE_LABELS[num_thermistors] = {"HEATER", "COOLER", "OUTSIDE", "ELECTRONICS", "INSIDE"};

// Variables for thermistor calculation
float thermistor_resistance, resistor_voltage, ln, Tx, T0, thermistor_voltage;

void setup() {
    // Setup serial communication
    Serial.begin(9600);
    // Convert T0 from Celsius to Kelvin
    T0 = 25 + 273.15;
}

float get_temperature(const int thermistor_pin) {
    const float thermistor_voltage = (5.00 / 1023.00) * analogRead(thermistor_pin);
    const float resistor_voltage = 5.00 - thermistor_voltage;
    const float thermistor_resistance = thermistor_voltage / (resistor_voltage / R);
    ln = log(thermistor_resistance / RTreference);
    return (1 / ((ln / B) + (1 / T0))) - 273.15;
}

void get_temperature_array(int array_size, const int pin_array[]) {
    for (int i = 0; i < array_size; i++) {
        TEMPERATURE_ARRAY[i] = get_temperature(pin_array[i]);
    }
}

void update_counter_array(const int array_size) {
    for (int i = 0; i < array_size; i++) {
        if (TEMPERATURE_ARRAY[i] >= 35) { COUNTER_ARRAY[i]++;}
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