#include "Arduino.h"
#include "SensirionI2CSen5x.h"
#include "Wire.h"

const char SENSOR_ID[] = "DEVICE001";
const uint8_t TCA9548A_ADDRESS = 0x70;
const uint8_t SENSOR_A_BUS = 4;
const uint8_t SENSOR_B_BUS = 5;
const uint8_t SAMPLE_PAIRS = 5;
const uint32_t BAUD_RATE = 115200;

SensirionI2CSen5x sen5x;

struct SensorReading {
  float temp;
  float rh;
  float pm1p0;
  float pm2p5;
  float pm4p0;
  float pm10p0;
  float voc;
  float nox;
  int16_t co;
};

void selectMuxChannel(uint8_t channel) {
  if (channel > 7) {
    return;
  }

  Wire.beginTransmission(TCA9548A_ADDRESS);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

void setupSensorOnBus(uint8_t channel) {
  const float tempOffset = 0.0f;

  selectMuxChannel(channel);
  sen5x.begin(Wire);
  sen5x.deviceReset();
  sen5x.setTemperatureOffsetSimple(tempOffset);
  sen5x.startMeasurement();
}

bool readSen55(uint8_t channel, SensorReading &reading) {
  selectMuxChannel(channel);

  uint16_t error = sen5x.readMeasuredValues(
    reading.pm1p0,
    reading.pm2p5,
    reading.pm4p0,
    reading.pm10p0,
    reading.rh,
    reading.temp,
    reading.voc,
    reading.nox
  );

  reading.co = 0;
  return error == 0;
}

void addReading(SensorReading &sum, const SensorReading &reading) {
  sum.temp += reading.temp;
  sum.rh += reading.rh;
  sum.pm1p0 += reading.pm1p0;
  sum.pm2p5 += reading.pm2p5;
  sum.pm4p0 += reading.pm4p0;
  sum.pm10p0 += reading.pm10p0;
  sum.voc += reading.voc;
  sum.nox += reading.nox;
  sum.co += reading.co;
}

void divideReading(SensorReading &reading, float divisor) {
  reading.temp /= divisor;
  reading.rh /= divisor;
  reading.pm1p0 /= divisor;
  reading.pm2p5 /= divisor;
  reading.pm4p0 /= divisor;
  reading.pm10p0 /= divisor;
  reading.voc /= divisor;
  reading.nox /= divisor;
  reading.co /= divisor;
}

void printReading(Stream &out, const SensorReading &reading) {
  out.print(reading.temp, 2);
  out.print(", ");
  out.print(reading.rh, 2);
  out.print(", ");
  out.print(reading.pm1p0, 2);
  out.print(", ");
  out.print(reading.pm2p5, 2);
  out.print(", ");
  out.print(reading.pm4p0, 2);
  out.print(", ");
  out.print(reading.pm10p0, 2);
  out.print(", ");
  out.print(reading.voc, 1);
  out.print(", ");
  out.print(reading.nox, 1);
  out.print(", ");
  out.print(reading.co, DEC);
}

void printMissingReading(Stream &out) {
  out.print("-99, -99, -99, -99, -99, -99, -99, -99, -99");
}

void sendPacket(Stream &out, bool ok, const SensorReading &sensorA, const SensorReading &sensorB) {
  out.print("0,");
  out.print(SENSOR_ID);
  out.print(", A, ");

  if (ok) {
    printReading(out, sensorA);
    out.print(", B, ");
    printReading(out, sensorB);
  } else {
    printMissingReading(out);
    out.print(", B, ");
    printMissingReading(out);
  }

  out.println();
}

void setup() {
  Serial.begin(BAUD_RATE);
  Serial1.begin(BAUD_RATE);
  Wire.begin();

  setupSensorOnBus(SENSOR_A_BUS);
  setupSensorOnBus(SENSOR_B_BUS);
}

void loop() {
  SensorReading sensorASum = {};
  SensorReading sensorBSum = {};
  bool ok = true;

  for (uint8_t i = 0; i < SAMPLE_PAIRS; i++) {
    SensorReading sensorA = {};
    SensorReading sensorB = {};

    ok = readSen55(SENSOR_A_BUS, sensorA) && ok;
    ok = readSen55(SENSOR_B_BUS, sensorB) && ok;

    addReading(sensorASum, sensorA);
    addReading(sensorBSum, sensorB);
    delay(1000);
  }

  divideReading(sensorASum, SAMPLE_PAIRS);
  divideReading(sensorBSum, SAMPLE_PAIRS);

  sendPacket(Serial, ok, sensorASum, sensorBSum);
  sendPacket(Serial1, ok, sensorASum, sensorBSum);
}
