#include <Arduino.h>
#include <math.h>
#include <Servo.h>
#include <Wire.h>
//#include <cmath>

// Thermistor parameters from the datasheet
constexpr int RTref = 4000;
constexpr int B = 3568;
// Series resistor value = 10 kΩ
constexpr int R = 10000;
constexpr double T0 = 298.15;
constexpr int num_thermistors = 5;
constexpr double instr_gain = 1 + (49.4 / 42.2);
constexpr int8_t i2c_address = 0b0101100;
constexpr int8_t i2c_instruction = 0b00000000;

//Pin definitions
#define LSB_MUX 7
#define MSB_MUX 8
#define CONVECTION_FAN 3
#define ATMOSPHERE_FAN 9
#define ATMOSPHERE_SERVO 10
#define ENTRY_DIODE 11
#define EXIT_DIODE 12

Servo Atmospheric_Cycle_Servo;

constexpr int THERMISTOR_PINS[num_thermistors] = {A0, A1, A2, A3, A6};
float TEMPERATURE_ARRAY[num_thermistors] = {0, 0, 0, 0, 0};
int COUNTER_ARRAY[num_thermistors] = {0, 0, 0, 0, 0};
String TEMPERATURE_LABELS[num_thermistors] = {"HEATER", "COOLER", "OUTSIDE", "ELECTRONICS", "INSIDE"};
constexpr float resistance_ratio = 4.0/14.0;
constexpr float TH = 33.0;
float TC = 27.0;
int heating_cooling_intensity = 0;

// Variables for thermistor calculation
float thermistor_resistance, resistor_voltage, ln, Tx, thermistor_voltage;
unsigned long atmospheric_cycle_delay = 1800000; //30 min.
unsigned int atmospheric_cycle_time = 60; //1 min cycle time
unsigned long last_atmospheric_cycle = millis();
bool cycling_atmosphere = false;
bool overheat_block = false;
int hour = 0;

//Entry-Exit Tracking
//int ENTRY_EXIT[2] = {0, 0};

void setup() {
    // Setup serial communication
    Serial.begin(9600);
    Wire.begin(i2c_address);
    //Wire.beginTransmission(i2c_address);
    Atmospheric_Cycle_Servo.attach(ATMOSPHERE_SERVO);
    pinMode(ATMOSPHERE_SERVO, OUTPUT);
    pinMode(ENTRY_DIODE, INPUT);
    pinMode(EXIT_DIODE, INPUT);
    analogWrite(CONVECTION_FAN, 255);
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
            COUNTER_ARRAY[i] = 0;
            //if (COUNTER_ARRAY[i] > 0) { COUNTER_ARRAY[i]--; }
            //else if (COUNTER_ARRAY[i] < 0) { COUNTER_ARRAY[i]++; }
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
    auto heating_message = static_cast<uint8_t>(output_intensity);
    //heating_message = 0b01100000;
    digitalWrite(LSB_MUX, LOW);
    digitalWrite(MSB_MUX, HIGH);
    //Serial.println("heat");
    //Serial.println(digitalRead(LSB_MUX));
    //Serial.println(digitalRead(MSB_MUX));
    Wire.beginTransmission(i2c_address);  // Start I2C transmission
    Wire.write(i2c_instruction);

    Wire.write(heating_message);         // Send the heating message
    byte status = Wire.endTransmission(); // End I2C transmission and check status
    /*
    if (status == 0) {
        Serial.println("I2C transmission successful");
    } else {
        Serial.print("I2C transmission failed with status: ");
        Serial.println(status);
    }*/
}

void start_cooling(const int output_intensity) {
    auto cooling_message = static_cast<uint8_t>(output_intensity);
    digitalWrite(MSB_MUX, LOW); digitalWrite(LSB_MUX, HIGH);
    analogWrite(CONVECTION_FAN, output_intensity);
    Wire.beginTransmission(i2c_address);  // Start I2C transmission
    Wire.write(i2c_instruction);

    Wire.write(cooling_message);         // Send the cooling message
    byte status = Wire.endTransmission(); // End I2C transmission and check status
/*
    if (status == 0) {
        Serial.println("I2C transmission successful");
    } else {
        Serial.print("I2C transmission failed with status: ");
        Serial.println(status);
    }
    */
}


void stop_thermals() {
    digitalWrite(MSB_MUX, 0); digitalWrite(LSB_MUX, 0);
    digitalWrite(CONVECTION_FAN, 0);
}

void cycle_atmosphere_toggle(bool* cycling) {
    if (*cycling) {
        *cycling = false;
        Atmospheric_Cycle_Servo.write(0);
        digitalWrite(LSB_MUX, 0); digitalWrite(MSB_MUX, 0);
    }
    if (!*cycling) {
        *cycling = true;
        Atmospheric_Cycle_Servo.write(90);
        digitalWrite(LSB_MUX, 1); digitalWrite(MSB_MUX, 0);
    }
}

void sendTemperatureSerial() {
    for (int i=0;i<5;i++) {
        Serial.print(TEMPERATURE_LABELS[i]);
        Serial.print(": ");
        Serial.print(TEMPERATURE_ARRAY[i]);
        Serial.print("C,");
    }
    Serial.println();
    //Serial.flush();
}

void updateHour(int* hour, float* cold_threshold_temperature) {
    if (Serial.available() > 0) {
        *hour = Serial.readStringUntil('\n').toInt();
    }
    if (0 <= *hour and *hour <= 6) {
        *cold_threshold_temperature = 20.0;
    }
    else {
        *cold_threshold_temperature = 27.0;
    }
}

/*
void check_entry_exit(int* entry_exit_array) {
    bool entry_detected = false;
    bool exit_detected = false;
    if (entry_detected) {entry_exit_array[0]++;}
    if (exit_detected) {entry_exit_array[1]++;}
}*/

void loop() {
    analogWrite(CONVECTION_FAN, 255);
    digitalWrite(7, 0);
    get_temperature_array(num_thermistors, THERMISTOR_PINS);
    update_counter_array(num_thermistors);
    //check_entry_exit(&ENTRY_EXIT[0]);
    int thermal_factor = abs(deltaTemperature(TEMPERATURE_ARRAY)*counterFunction(COUNTER_ARRAY));
    if (thermal_factor > 120) {
        thermal_factor = 120;
    }
    heating_cooling_intensity = map(thermal_factor,
                                    0, 120, 128, 255);
    /*
    Serial.println(thermal_factor);
    Serial.println(heating_cooling_intensity);
    Serial.println(counterFunction(COUNTER_ARRAY));
    Serial.println(deltaTemperature(TEMPERATURE_ARRAY));
    Serial.println(COUNTER_ARRAY[4]);
    */
    if (not cycling_atmosphere and not overheat_block) {
        if (COUNTER_ARRAY[4] > 0) {start_cooling(255-heating_cooling_intensity); Serial.println("cool");}
        else if (COUNTER_ARRAY[4] < 0) {start_heating(heating_cooling_intensity);}
        else if (COUNTER_ARRAY[4] == 0) {stop_thermals();}

        if (TEMPERATURE_ARRAY[0] > 60) {
            overheat_block = true;
            stop_thermals();
        }
    }

    if (overheat_block and TEMPERATURE_ARRAY[0] < 50) {
        overheat_block = false;
    }

    if (millis() - last_atmospheric_cycle > atmospheric_cycle_delay) {
        last_atmospheric_cycle = millis();
        cycle_atmosphere_toggle(&cycling_atmosphere);
    }

    if (cycling_atmosphere and (millis() - last_atmospheric_cycle > atmospheric_cycle_time)) {
        cycle_atmosphere_toggle(&cycling_atmosphere);
    }
    //updateHour(&hour, &TC);
    sendTemperatureSerial();
    delay(500);
}