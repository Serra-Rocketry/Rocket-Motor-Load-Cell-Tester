#if defined(ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#elif defined(ESP8266)
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#endif

#include <InfluxDbClient.h>
#include <ArduinoOTA.h>

// WiFi AP SSID
#define WIFI_SSID "Raspberry-AP"
// WiFi password
#define WIFI_PASSWORD "raspberry"
// InfluxDB  server url. Don't use localhost, always server name or ip address.
// E.g. http://192.168.1.48:8086 (In InfluxDB 2 UI -> Load Data -> Client Libraries), 
#define INFLUXDB_URL "http://192.168.1.108:8086/"
// InfluxDB 2 server or cloud API authentication token (Use: InfluxDB UI -> Load Data -> Tokens -> <select token>)
#define INFLUXDB_TOKEN "oDwDIOL9HpwKM16Jl7iZ2ijLGoROkwpe5qMpRBY155eULgs8dQdU8nlZdRVaLDc-nkDWkW_TinftyvoNmOVjWA=="
// InfluxDB 2 organization id (Use: InfluxDB UI -> Settings -> Profile -> <name under tile> )
#define INFLUXDB_ORG "UERJ"
// InfluxDB 2 bucket name (Use: InfluxDB UI -> Load Data -> Buckets)
#define INFLUXDB_BUCKET "rocket"
// InfluxDB v1 database name 
//#define INFLUXDB_DB_NAME "database"

// InfluxDB client instance
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
// InfluxDB client instance for InfluxDB 1
//InfluxDBClient client(INFLUXDB_URL, INFLUXDB_DB_NAME);

// Data point
Point sensor("StaticTester");

#include "HX711.h"

#define CELULA_DT  26
#define CELULA_SCK  27

HX711 escala;

float fator_calib = 473893; // Coloque aqui o valor encontrado na calibração

void OTA() {
  ArduinoOTA.setHostname("StaticTester");
  ArduinoOTA.onStart([](){
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([](){
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void setupInfluxDB(){
  // Set up the InfluxDB client
  client.setConnectionParams(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
  configTzTime("America/Sao_Paulo", "a.st1.ntp.br", "b.st1.ntp.br");
  // Add tags to the data point
  sensor.addTag("Version", "V0.1");
  // Check if the connection to the InfluxDB server is valid
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  // Set the write options for the data point
  client.setWriteOptions(WriteOptions().writePrecision(WritePrecision::MS));
  // Set the batch size of the write options
  client.setWriteOptions(WriteOptions().batchSize(60));
};

void setupLoadCell(){
  escala.begin(CELULA_DT, CELULA_SCK);
  escala.set_scale(fator_calib);
  escala.tare();
}

void setup() {
  // Start the serial communication
  Serial.begin(115200);
  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  #if defined(ESP32)
    WiFi.setTxPower(WIFI_POWER_19_5dBm);    // Set WiFi RF power output to highest level
  #elif defined(ESP8266)
    WiFi.setOutputPower(20.5); // Set WiFi RF power output to highest level
  #endif

  Serial.println("Connected to WiFi!");
  setupInfluxDB();
  setupLoadCell();
  OTA();
}

void dataWriteInfluxDB(){
  // Clear fields of the sensor data point
  sensor.clearFields();
  // Add the load cell weight 
  // sensor.addField("loadCell", escala.get_units(1),3);
  // Add a sine wave as a reference field to the sensor data point
  float referenceValue = sin(6.28*(millis()/30));
  sensor.addField("reference", referenceValue, 3);
  // Print the line protocol for the sensor data point
  Serial.print("Writing: ");
  Serial.println(client.pointToLineProtocol(sensor));
  // If the WiFi connection is lost, try to reconnect it
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi connection lost");
  }
  // Write the sensor data point to the InfluxDB server
  if (client.writePoint(sensor)) {
    Serial.println("Data point written successfully");
  } else {
    Serial.print("Failed to write data point: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void loop() {
  dataWriteInfluxDB();
  ArduinoOTA.handle();
}
