#include <FS.h>
#include <WiFiMulti.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <ArduinoJson.h>
#include "DHT.h"

#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

// Create your Data Point here
Point sensor("climate");

// Set up the DHT connection
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Start Serial for monitoring
  Serial.begin(115200);

  // Init the DHT sensor
  dht.begin();

  // Mount the file system
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  // Read the Wi-Fi credentials and InfluxDB parameters from the data.json file
  File configFile = SPIFFS.open("/data.json", "r");
  if (!configFile) {
    Serial.println("Failed to open data.json file");
    return;
  }

  StaticJsonDocument<256> config;
  DeserializationError error = deserializeJson(config, configFile);
  if (error) {
    Serial.print("Failed to parse data.json file: ");
    Serial.println(error.c_str());
    return;
  }

  const char* wifi_ssid = config["wifi_ssid"];
  const char* wifi_password = config["wifi_password"];
  const char* influxdb_url = config["influxdb_url"];
  const char* influxdb_token = config["influxdb_token"];
  const char* influxdb_org = config["influxdb_org"];
  const char* influxdb_bucket = config["influxdb_bucket"];
  const char* tz_info = config["tz_info"];

  // Setup wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);

  Serial.print("Connecting to wifi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  // Add tags. Here we will track which device our data is coming from
  #if defined(ESP32)
  Point sensor("climate", DEVICE);
  #elif defined(ESP8266)
  Point sensor("climate");
  sensor.addTag("device", DEVICE);
  #endif

  // Accurate time is necessary for certificate validation and writing in batches
  // For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(tz_info, "pool.ntp.org", "time.nis.gov");

  // InfluxDB client instance with preconfigured InfluxCloud certificate
  InfluxDBClient client(influxdb_url, influxdb_org, influxdb_bucket, influxdb_token, InfluxDbCloud2CACert);

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}


void loop() {
  // Clear fields for reusing the point. Tags will remain untouched
  sensor.clearFields();

  // Read the temperature and humidity from the sensor, and add them to your data point, along with a calculated heat index
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  sensor.addField("humidity", h);
  sensor.addField("temperature", t);
  sensor.addField("heat_index", dht.computeHeatIndex(t, h, false));

  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(sensor.toLineProtocol());

  // If no Wifi signal, try to reconnect it
  if ((WiFi.RSSI() == 0) && (wifiMulti.run() != WL_CONNECTED)) {
    Serial.println("Wifi connection lost");
  }

  // Write point
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  //Wait 10s
  Serial.println("Wait 10s");
  delay(10000);
}
