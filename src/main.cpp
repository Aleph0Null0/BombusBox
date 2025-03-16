#include <Arduino.h>
#include <math.h>
#include <Servo.h>
//#include <cmath>

// Thermistor parameters from the datasheet
constexpr int RTref = 4000;
constexpr int B = 3568;
// Series resistor value = 10 kΩ
constexpr int R = 10000;
constexpr double T0 = 298.15;
constexpr int num_thermistors = 5;
constexpr double instr_gain = 1 + (49.4 / 42.2);

//Pin definitions
#define HEATER_PIN A4
#define COOLER_PIN A5
#define HEATER_CONTROLLER D4
#define COOLER_CONTROLLER D5
#define ATMOSPHERE_FAN 9
#define ATMOSPHERE_SERVO 10
#define ENTRY_DIODE 11
#define EXIT_DIODE 12

Servo Atmospheric_Cycle_Servo;

constexpr int THERMISTOR_PINS[num_thermistors] = {A0, A1, A2, A3, A6};
float TEMPERATURE_ARRAY[num_thermistors] = {0, 0, 0, 0, 0};
int COUNTER_ARRAY[num_thermistors] = {0, 0, 0, 0, 0};
//String TEMPERATURE_LABELS[num_thermistors] = {"INSIDE", "AMBIENT", "HEATSINKOUTSIDE", "TOTE", "HEATSINKINSIDE"};
String TEMPERATURE_LABELS[num_thermistors] = {"HEATER", "COOLER", "OUTSIDE", "ELECTRONICS", "INSIDE"};
constexpr float resistance_ratio = 4.0/14.0;
constexpr float TH = 33.0;
constexpr float TC = 27.0;

// Variables for thermistor calculation
float thermistor_resistance, resistor_voltage, ln, Tx, thermistor_voltage;
unsigned long atmospheric_cycle_delay = 1800000; //30 min.
unsigned int atmospheric_cycle_time = 60; //1 min cycle time
unsigned long last_atmospheric_cycle = millis();
bool cycling_atmosphere = false;

//Entry-Exit Tracking
int ENTRY_EXIT[2] = {0, 0};

void setup() {
    // Setup serial communication
    Serial.begin(9600);
    Atmospheric_Cycle_Servo.attach(ATMOSPHERE_SERVO);
    pinMode(ATMOSPHERE_SERVO, OUTPUT);
    pinMode(ENTRY_DIODE, INPUT);
    pinMode(EXIT_DIODE, INPUT);
}

float get_temperature(const int thermistor_pin) {
    const float thermistor_voltage = (5.00 / 1023.00) * analogRead(thermistor_pin);
    const float differential_voltage = thermistor_voltage/instr_gain;
    const float resistance_factor = (differential_voltage/5) + (4.0/14);
    const float thermistor_resistance = R*(resistance_factor / (1 - resistance_factor));
    return (1 / (log(thermistor_resistance/R)/B + 1/T0)) - 273.15;
}

void get_temperature_array(int array_size, const int pin_array[]) {
    for (int i = 0; i < array_size; i++) {
        TEMPERATURE_ARRAY[i] = get_temperature(pin_array[i]);
    }
}

void update_counter_array(const int array_size) {
    for (int i = 0; i < array_size; i++) {
        if (TEMPERATURE_ARRAY[i] >= TH) { COUNTER_ARRAY[i]++;}
        else if (TEMPERATURE_ARRAY[i] <= TC) { COUNTER_ARRAY[i]--;}
        else {
            if (COUNTER_ARRAY[i] > 0) { COUNTER_ARRAY[i]--; }
            else if (COUNTER_ARRAY[i] < 0) { COUNTER_ARRAY[i]++; }
        }
    }
}

float deltaTemperature(const float* TEMPERATURES) {
    return TEMPERATURES[2] - TEMPERATURES[4];
}

int counterFunction(const int* COUNTER_ARRAY) {
    return abs(COUNTER_ARRAY[4]*0.9 + COUNTER_ARRAY[3]*0.05 + COUNTER_ARRAY[1]*0.025 + COUNTER_ARRAY[0]*0.025);
}

void start_heating(const int output_intensity) {
    analogWrite(HEATER_PIN, 1);
    //TODO: Figure out how to define output voltage in I2C System
}
//TODO: Stop output voltage signal if needed
void stop_heating() { analogWrite(HEATER_PIN, 0); }

void start_cooling(const int output_intensity) {
    analogWrite(COOLER_PIN, 1);
    //TODO: Figure out how to define output voltage in I2C System
}
//TODO: Stop output voltage signal if needed
void stop_cooling() { analogWrite(COOLER_PIN, 0); }

void cycle_atmosphere_toggle(bool* cycling) {
    if (*cycling) {
        *cycling = false;
        Atmospheric_Cycle_Servo.write(0);
        //TODO: Stop fan
    }
    if (!*cycling) {
        *cycling = true;
        Atmospheric_Cycle_Servo.write(90);
        //TODO: Start fan
    }
}

void sendTemperatureSerial() {
    for (int i=0;i<5;i++) {
        //Serial.print("Temperature ");
        Serial.print(TEMPERATURE_LABELS[i]);
        Serial.print(": ");
        Serial.print(TEMPERATURE_ARRAY[i]);
        Serial.print("C,");
    }
    Serial.println();
}

void check_entry_exit(int* entry_exit_array) {
    //TODO: Figure out what to read for detection
    bool entry_detected = false;
    bool exit_detected = false;
    if (entry_detected) {entry_exit_array[0]++;}
    if (exit_detected) {entry_exit_array[1]++;}
}

void loop() {
    get_temperature_array(num_thermistors, THERMISTOR_PINS);
    update_counter_array(num_thermistors);
    check_entry_exit(&ENTRY_EXIT[0]);
    //TODO: Implement evaluation of counter array, and adjust signal to heater/cooler accordingly

    int heating_cooling_intensity = deltaTemperature(TEMPERATURE_ARRAY)*counterFunction(COUNTER_ARRAY);
    if (COUNTER_ARRAY[0] > 0) {start_heating(heating_cooling_intensity);}
    else if (COUNTER_ARRAY[0] < 0) {start_cooling(heating_cooling_intensity);}

    if (millis() - last_atmospheric_cycle > atmospheric_cycle_delay) {
        last_atmospheric_cycle = millis();
        cycle_atmosphere_toggle(&cycling_atmosphere);
    }
    if (cycling_atmosphere and (millis() - last_atmospheric_cycle > atmospheric_cycle_time)) {
        cycle_atmosphere_toggle(&cycling_atmosphere);
    }

    sendTemperatureSerial();
    delay(500);
}