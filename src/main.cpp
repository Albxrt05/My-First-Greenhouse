#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "Adafruit_seesaw.h"

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)
const int LOWER_TEMP = 21;
const int UPPER_TEMP = 29;

Adafruit_seesaw soilSensor; // soil moisture sensor object

const int WINDOW_SIZE = 4; // size of moving average window
float moistureReadings[WINDOW_SIZE];

int currentIndex = 0;
float avgMoisture = 0;
bool soilIsWet = false;


Adafruit_BME680 bme; // I2C
//Adafruit_BME680 bme(BME_CS); // hardware SPI
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);

enum State {
  IDLE,
  WATERING,
  WARNING_TEMP
};

// function headers
void setupBME();
void checkBME();

State greenhouse = IDLE;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println(F("BME680 test"));

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  setupBME(); 

  Serial.begin(115200);

  // initialize sensor
  if (!soilSensor.begin(0x36)) {
    Serial.println("Could not find seesaw sensor. Check wiring.");
    while (1);
  }

  Serial.print("Seesaw initialized. Version: ");
  Serial.println(soilSensor.getVersion(), HEX);
}

void loop() {
  //checkBME();
  Serial.println("hello world");
  switch (greenhouse) {
    case IDLE: 
      checkBME();
      break;
    case WATERING:
      Serial.println("watering");
      break;
    case WARNING_TEMP:
      Serial.println("Warning: temperature not in range. Please adjust accordingly.");
    default:
      Serial.println("default case");
  }

  float temperature = soilSensor.getTemp();
  uint16_t moisture = soilSensor.touchRead(0);

  // store reading in array (circular buffer)
  if (currentIndex >= WINDOW_SIZE) {
    currentIndex = 0;
  }

  moistureReadings[currentIndex] = moisture;
  currentIndex++;

  // compute average
  avgMoisture = 0;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    avgMoisture += moistureReadings[i];
  }
  avgMoisture /= WINDOW_SIZE;

  // determine if soil is wet
  if (avgMoisture > 400) {
    soilIsWet = true;
  } else {
    soilIsWet = false;
  }

  // serial output
  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.println(" C");

  Serial.print("Moisture (avg): ");
  Serial.println(avgMoisture);

  Serial.print("Soil Status: ");
  if (soilIsWet) {
    Serial.println("Wet");
  } else {
    Serial.println("Not Wet");
  }

  delay(1000);
}

void checkBME() {
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
  Serial.print("Temperature = ");
  Serial.print(bme.temperature);
  Serial.println(" *C");
  if (!tempInRange(bme.temperature)) {
    greenhouse = WARNING_TEMP;
  }

  Serial.print("Pressure = ");
  Serial.print(bme.pressure / 100.0);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.humidity);
  Serial.println(" %");

  Serial.print("Gas = ");
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(" KOhms");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.println();
  delay(2000);
}

bool tempInRange(int temp) {
  if ((temp >= LOWER_TEMP) && (temp <= UPPER_TEMP)) {
    return true;
  } else {
    return false;
  }
}


void setupBME() {
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}
