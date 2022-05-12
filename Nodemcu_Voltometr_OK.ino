#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <TimeLib.h>
#include <LiquidCrystal_I2C.h> // Library for LCD
#include <Wire.h>
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x3F, 16, 2); // Change to (0x27,16,2) for 16x2 LCD.

const char* ssid     = "gmarkop";
const char* password = "111111111111";
const char* serverName = "http://192.168.1.15/post-esp-data.php";
String apiKeyValue = "1111111111";
const int device = 3; // bike=3 or treadmill=4
int count = 0;
long t;
int rpm;
const int inputPin = D3;               // Connect magnetic sensor to input pin a1
int val = 0;                    // variable for reading the pin status
int analogInput = 0;
float vout = 0.0;
float vin = 0.0;
float R1 = 100000.0; // resistance of R1 (100K) -see text!
float R2 = 10000.0; // resistance of R2 (10K) - see text!
int value = 0;
const unsigned long sampleTime = 1000;
const float a = 0.4;// αμπερ  που βγάζουν τα μοτερ
float w;
void setup() {
  pinMode(analogInput, INPUT);
  pinMode(inputPin, INPUT);
  Wire.begin(D2, D1);
  lcd.begin();
  lcd.home();
  lcd.print("Autonomous Bike");
  lcd.setCursor(0, 1);
  lcd.print("  Genarator!!!");
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // delay(100);
  rpm = getRPM();
  // read the value at analog input
  value = analogRead(analogInput);
  vout = (value * 5.0) / 1024.0;
  vin = vout / (R2 / (R1 + R2)) -5;

  if (vin < 0.9) {
    vin = 0.0; //statement to quash undesired reading !
  }

  w = a * vin;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("V=");
  lcd.print(vin,1);
  lcd.setCursor(7, 0);
  lcd.print("Watt=");
  lcd.print(w);
  lcd.setCursor(5, 1);
  lcd.print("Rpm=");
  lcd.print(rpm);
  Serial.println(rpm);




  if (millis() > t + 3000) {        //Send an HTTP POST request every 3 seconds
    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverName);
      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Prepare your HTTP POST request data
      String httpRequestData = "api_key=" + apiKeyValue + "&datetime=" + now()
                               + "&deviceID=" + device + "&deviceType=" + String("BIKE")
                               + "&value=" + String(vin) + "";
      Serial.print("httpRequestData: ");
      Serial.println(httpRequestData);
      int httpResponseCode = http.POST(httpRequestData);

      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else  {
      Serial.println("WiFi Disconnected");
    }
    t = millis();
  }
}


int getRPM()
{
  int count = 0;
  boolean countFlag = LOW;
  unsigned long currentTime = 0;
  unsigned long startTime = millis();
  while (currentTime <= sampleTime)
  {
    if (digitalRead(inputPin) == HIGH)
    {
      countFlag = HIGH;
    }
    if (digitalRead(inputPin) == LOW && countFlag == HIGH)
    {
      count++;
      countFlag = LOW;
    }
    currentTime = millis() - startTime;
  }
  int countRpm = int(60000 / int(sampleTime)) * count;
  return countRpm;
}
