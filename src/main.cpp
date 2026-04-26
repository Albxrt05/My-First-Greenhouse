#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)
const int LOWER_TEMP = 21;
const int UPPER_TEMP = 29;
const int LOWER_HUM = 60;
const int UPPER_HUM = 80;

int temp = -1;
int humidity = -1;
unsigned long lastDashboard = 60000;
const unsigned long DASH_INTERVAL = 60000; // 60,000 ms = 1 minute


Adafruit_BME680 bme; // I2C
//Adafruit_BME680 bme(BME_CS); // hardware SPI
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);

enum State {
  IDLE,
  WATERING,
  WARNING_TEMP,
  WARNING_HUM
};

// function headers
void setupBME();
void checkBME();
bool tempInRange();
bool humidityInRange();
void checkDashboard();
void printDashboard();

State greenhouse = IDLE;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  //Serial.println(F("BME680 test"));

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  setupBME(); 
}

void loop() {
  //checkBME();
  //Serial.println("hello world");
  switch (greenhouse) {
    case IDLE: 
      checkBME();
      checkDashboard();
      break;
    case WATERING:
      Serial.println("watering");
      break;
    case WARNING_TEMP:
      Serial.println("Warning: temperature not in range. Please adjust accordingly.");
      Serial.println(String("Temperature: ") + temp + "%");
      delay(1000);
      greenhouse = IDLE;
      break;
    case WARNING_HUM:
      Serial.println("Warning: humidity not in range. Please adjust accordingly.");
      Serial.println(String("Humidity: ") + humidity + "%");
      delay(1000);
      greenhouse = IDLE;
      break;
    default:
      Serial.println("default case");
  }
}

void checkBME() {
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
 // Serial.print("Temperature = ");
  //Serial.print(bme.temperature);
 // Serial.println(" *C");
  temp = bme.temperature;
  if (!tempInRange()) {
    greenhouse = WARNING_TEMP;
  }

  humidity = bme.humidity;
  if (!humidityInRange()) {
    greenhouse = WARNING_HUM;
  }

  delay(3000);
}

bool tempInRange() {
  if ((temp >= LOWER_TEMP) && (temp <= UPPER_TEMP)) {
    //Serial.println("temp in range");
    return true;
  } else {
    return false;
  }
}

bool humidityInRange() {
  if ((humidity >= LOWER_HUM) && (humidity <= UPPER_HUM)) {
    //Serial.println("humidity in range");
    return true;
  } else {
    return false;
  }
}

void checkDashboard() {
  if (millis() - lastDashboard >= DASH_INTERVAL) {
    printDashboard();
    lastDashboard = millis();
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

void printDashboard() {
  if (!bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  float temp = bme.temperature;
  float humidity = bme.humidity;
  float gas = bme.gas_resistance / 1000.0;

  // update state based on temp
  if (!tempInRange()) {
    greenhouse = WARNING_TEMP;
  } else {
    greenhouse = IDLE;
  }
  

  Serial.println("\n----- GREENHOUSE STATUS -----");

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Gas: ");
  Serial.print(gas);
  Serial.println(" KOhms");

  Serial.print("Pressure: ");
  Serial.print(bme.pressure / 100.0);
  Serial.println(" hPa");

  Serial.print("Altitude: ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print("State: ");
  switch (greenhouse) {
    case IDLE: Serial.println("IDLE"); break;
    case WATERING: Serial.println("WATERING"); break;
    case WARNING_TEMP: Serial.println("WARNING_TEMP"); break;
  }

  Serial.println("-----------------------------\n");
}
