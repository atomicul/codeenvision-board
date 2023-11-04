#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <MQ135.h>
#include<string.h>

byte buff[2];
#define DSM501 18
unsigned long duration;
unsigned long starttime;
unsigned long endtime;
unsigned long sampletime_ms = 30000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
uint32_t delayMS = 5000;

#define DHTPIN 23
#define DHTTYPE    DHT11     // DHT 11

DHT_Unified dht(DHTPIN, DHTTYPE);

#define PIN_MQ135 4
MQ135 mq135_sensor(PIN_MQ135);

void setup() {
  Serial.begin(9600);
  // Initialize device.
  pinMode(DSM501,INPUT);
  pinMode(PIN_MQ135, INPUT_PULLUP);
  starttime = millis(); 
  dht.begin();
}

void loop() {
  // Delay between measurements.
  delay(delayMS);
  // Get temperature event and print its value.
  sensors_event_t temp, humid;
  dht.temperature().getEvent(&temp);
  dht.humidity().getEvent(&humid);
  if (isnan(temp.temperature) || isnan(humid.relative_humidity)) {
    Serial.println(F("Error reading dht!"));
    if(isnan(temp.temperature))
      Serial.println("Error reading temperature");
    if(isnan(humid.relative_humidity))
      Serial.println("Error reading humidity");
    return;
  }

  auto temperature = temp.temperature;
  auto humidity = humid.relative_humidity;
  double rzero = mq135_sensor.getRZero();
  double correctedRZero = mq135_sensor.getCorrectedRZero(temperature, humidity);
  double resistance = mq135_sensor.getResistance();
  double ppm = mq135_sensor.getPPM();
  double correctedPPM = mq135_sensor.getCorrectedPPM(temperature, humidity);

  Serial.print(F("Temperature: "));
  Serial.print(temperature);
  Serial.println(F("°C"));

  Serial.print(F("Humidity: "));
  Serial.print(humidity);
  Serial.println(F("%"));

  Serial.print("MQ135 RZero: ");
  Serial.print(rzero);
  Serial.print("\t Corrected RZero: ");
  Serial.print(correctedRZero);
  Serial.print("\t Resistance: ");
  Serial.print(resistance);
  Serial.print("\t PPM: ");
  Serial.print(ppm);
  Serial.print("\t Corrected PPM: ");
  Serial.print(correctedPPM);
  Serial.println("ppm");

  duration = pulseIn(DSM501, LOW);
  lowpulseoccupancy += duration;
  endtime = millis();
  if ((endtime-starttime) > sampletime_ms)
  {
    ratio = (lowpulseoccupancy-endtime+starttime + sampletime_ms)/(sampletime_ms*10.0);  // Integer percentage 0=>100
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
    Serial.print("lowpulseoccupancy:");
    Serial.print(lowpulseoccupancy);
    Serial.print("    ratio:");
    Serial.print(ratio);
    Serial.print("    DSM501A:");
    Serial.println(concentration);
    lowpulseoccupancy = 0;
    starttime = millis();
  } 
}
