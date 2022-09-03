#include <ESP8266WiFi.h>      // Connect to a WiFi
#include <ESP8266WebServer.h> // Establish a webserver
#include <Servo.h>            // Handle PWM signal to control servos
#include <ArduinoJson.h>

/**     Pins Defination    **/
int rows[] = {D0, D1};
int cols[] = {D2, D3, D4};

/**     Constants    **/
const char* AP_SSID = "ENTER_ACCESSPOINT_NAME";  
const char* AP_PASS = "ENTER_ACCESSPOINT_PASS";
const int ROWS = 2;
const int COLS = 3;

// Configure the network connection
const IPAddress LOCAL_IP(192, 168, 1, 145);
const IPAddress GATEWAY(192, 168, 1, 1);
const IPAddress SUBNET(255, 255, 255, 0);

/**     Global Variables    **/
bool isTesting = false;

/**     Global Objects    **/
ESP8266WebServer server(80); // A server object listen to port 80
DynamicJsonDocument data(2048); // use https://arduinojson.org/v6/assistant/ to determine the size

void setup() {
  /**   Configure Access Point    **/
  WiFi.softAPConfig(LOCAL_IP, GATEWAY, SUBNET);
  WiFi.softAP(AP_SSID, AP_PASS);

  /**   Configure pins   **/
  for (int i = 0; i < ROWS; i++) pinMode(rows[i], OUTPUT);
  for (int i = 0; i < COLS; i++) pinMode(cols[i], OUTPUT);

  /**   Configure Serial Communication    **/
  Serial.begin(115200);

  /**   Set the server routes   **/
  server.on("/", handleCheckConnection);
  server.on("/testLeds", handleTestLeds);
  server.on("/data", handleReceiveData);
  server.onNotFound(handleNotFound);

  /**   Establish the server    **/
  server.begin();
}

void loop() {
  // Start the server listening
  server.handleClient();

  if (isTesting) {
    testAllLeds();
  } else {
    if (!data.isNull()) {
      for (int row = 0; row < ROWS; row++) {
        digitalWrite(rows[row], LOW);
        
        for (int col = 0; col < COLS; col++) {
          digitalWrite(cols[col], data["bitmap"][row][col]);
        }
        delay(5);
        digitalWrite(rows[row], HIGH);
      }
    }
  }
}


/*
 * Check the app connection
 * ENDPOINT: /
 */
void handleCheckConnection() {
  server.send(200, "text/plain", "Connected"); 
}


/*
 * Handle the client when request a not found endpoint
 */
void handleNotFound(){
  server.send(404, "text/plain", "Not Found");
}


/*
 * Receive the bitmap data
 * ENDPOINT: /data
 */
void handleReceiveData () {
  // Parse post request body to settings json
  deserializeJson(data, server.arg("plain")); 

//  Test the json fully received
//  String dataString;
//  serializeJson(data, dataString);
//  Serial.println(dataString);
  
  server.send(200, "text/plain", "Received");
}


/*
 * Toggle leds testing mode
 * ENDPOINT: /testLeds
 */
void handleTestLeds() {
  isTesting = !isTesting;

  server.send(
    200, 
    "text/plain", 
    "Testing Mode: " + String((isTesting)? "ON" : "OFF")
  );
}


/*
 * Test all the leds in sequence
 */
void testAllLeds() {
  for (int row = 0; row < ROWS; row++) {
    for (int col = 0; col < COLS; col++) {
      turnOn(rows[row], cols[col]);
      delay(400);
      turnOff(rows[row], cols[col]);
    }
  }
}

void turnOn(int row, int col) {
  digitalWrite(row, LOW);
  digitalWrite(col, HIGH);
}


void turnOff(int row, int col) {
  digitalWrite(row, HIGH);
  digitalWrite(col, LOW);
}
