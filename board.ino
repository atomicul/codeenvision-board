#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <MQ135.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <string.h>
#include <string>
#include <sstream>

int delayMS = 3000;
const char* ssid = "balls";
const char* password = "prostu123";
const char* boardId = "inischiedutohxinuohneduijhkx";

#define DHTPIN 23
#define DHTTYPE DHT11     // DHT 11

DHT_Unified dht(DHTPIN, DHTTYPE);

#define PIN_MQ135 4
MQ135 mq135_sensor(PIN_MQ135);

#define PIN_DSM 2

void setup() {
  Serial.begin(9600);
  // Initialize device.
  pinMode(PIN_MQ135, INPUT_PULLUP);
  pinMode(PIN_DSM, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  while (!Serial || WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  dht.begin();
}

void postData(float temp, float humidity, float ppm, float dust) {
  const char * host = "https://172.20.10.3:3000/reading";
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(host);
    http.addHeader("Content-Type", "application/json");
    
    std::stringstream ss;
    ss << "{ \"id\": \"" << boardId << "\", \"temperature\": " << temp <<
        ", \"humidity\": " << humidity << ", \"ppm\": " << ppm << 
        ", \"dustConcentration\": " << dust << " }"; 

    std::string body = ss.str();
    Serial.println(body.c_str());
    int httpCode = http.POST(body.c_str());
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(httpCode);
      Serial.println(payload);
    } else {
      Serial.println("Error on HTTP request");
    }

    http.end();
  } else {
    Serial.println("Error in WiFi connection");
  }
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

  Serial.print(F("Temperature: "));
  Serial.print(temperature);
  Serial.println(F("°C"));

  Serial.print(F("Humidity: "));
  Serial.print(humidity);
  Serial.println(F("%"));

  float ppm = analogRead(PIN_MQ135);
  Serial.print(ppm);
  Serial.println( "ppm");

  float dust = analogRead(PIN_DSM);
  Serial.print(dust);
  Serial.println(" dust");

  postData(temperature, humidity, ppm, dust);
  delay(100000);
}
