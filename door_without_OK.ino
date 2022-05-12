#define USE_ARDUINO_INTERRUPTS true
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <TimeLib.h>
#include <LiquidCrystal_I2C.h> // Library for LCD
#include <Wire.h>
#include <Adafruit_MLX90614.h>

#include <SPI.h>
#include <MFRC522.h>
constexpr uint8_t RST_PIN = D3;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = D4;     // Configurable, see typical pin layout above
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;
String tag;
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2); // Change to (0x27,16,2) for 16x2 LCD.
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
const char* ssid     = "gmarkop";
const char* password = "11111111";
const char* serverName = "http://192.168.1.15/post-esp-data.php";
String apiKeyValue = "1111111111111";
const int device = 7; // bike=3 or treadmill=4 door=7
const int trigPin = 16; // or D0;
const int echoPin = 15; //  or D8;
long duration;
int distance;
float TempReading;
bool flag_Temp;
String user = "";
bool keypass=0, temppass=0;
void setup() {
  pinMode(1, FUNCTION_3); //GPIO 1 swap the pin to a GPIO.
  pinMode(3, FUNCTION_3); //GPIO 3 swap the pin to a GPIO.
  //****************************************************************
  pinMode(1, OUTPUT);
  pinMode(3, OUTPUT);
 // Serial.begin(9600);
  SPI.begin();      // Init SPI bus
  rfid.PCD_Init(); // Init
  delay(4);       // Optional delay. Some board do need more time after init to be ready, see Readme
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  mlx.begin();
 // lcd.init();
  lcd.backlight();
  lcd.home();
  lcd.print("Autonomous");
  lcd.setCursor(0, 1);
  lcd.print(" SafeDoor !!!");

  WiFi.begin(ssid, password);
  //Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }
  //Serial.println("");
 // Serial.print("Connected to WiFi network with IP Address: ");
  //Serial.println(WiFi.localIP());
  delay(3000);
  lcd.clear();

}


void loop() {
  delay(500);
  lcd.clear();
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.0340 / 2;
  //Serial.print("Distance:");
  //Serial.println(distance);

  if (distance < 100 and distance>25)
  {
    lcd.setCursor(0, 0);
    lcd.print("Cm:");
    lcd.print(distance);
    lcd.setCursor(0, 1);
    lcd.print("Get closer..  ");
  }
  else if  (distance <= 25 and distance >20)
  {
    TempReading = temp();
    //Serial.print("  Target  :");
    //Serial.print(TempReading);
    //Serial.println("  C");
    lcd.setCursor(0, 0);
    lcd.print("Cm:");
    lcd.print(distance);
    lcd.setCursor(0, 1);
    lcd.print("Temp:");
    lcd.print(TempReading);
    if (TempReading<37.5 and TempReading>34.5)
    {
      lcd.setCursor(12, 0);
      lcd.print("Ok!!");
      delay(2500);
      temppass=1;
    } 
     else if (TempReading>38) 
     {
    lcd.setCursor(10, 0);
      lcd.print("fever! :(");
      temppass=0;
      keypass=0;
     }   
    }
   
  else if (distance < 20 )
  {
    lcd.setCursor(0, 0);
    lcd.print("Cm:");
    lcd.print(distance);
    lcd.setCursor(0, 1);
    lcd.print("Too close...  ");
  }
  else
  {
    lcd.setCursor(0, 0);
    lcd.print("1)Covid Test");
    lcd.setCursor(0, 1);
    lcd.print("2)Identification");
  }

  user = rfid_port();
  if (user != "0" and user != "1")
  {
    //Serial.print("Welcome "); Serial.println( user );
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   Welcome ");
    lcd.setCursor(4, 1);
    lcd.print(user);
    keypass=1;
    delay(1500);
  }
    
  
  else if (user == "1")
  {
    //Serial.println("Access Denied!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("    Access");
    lcd.setCursor(0, 1);
    lcd.print("   Denied!");
    delay(2000);
    keypass=0;
    temppass=0;
  }

if (keypass==1 and temppass==1)
{
  //Serial.println("Open Door..");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  The Door ");
    lcd.setCursor(0, 1);
    lcd.print("   opens!!!");
    delay(3000);
    keypass=0;
    temppass=0;
    analogWrite(1,1000);
    delay(1600  tg tghhhhhhhhj );
    analogWrite(1,0);
    delay(5000);
    analogWrite(3,1000);
    delay(1600);
    analogWrite(3,0);
    send_data();
}
}

void send_data(){
  
  //Check WiFi connection status

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverName);
      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Prepare your HTTP POST request data
      String httpRequestData = "api_key=" + apiKeyValue + "&datetime=" + now()
                               + "&deviceID=" + device + "&deviceType=" + String(user)
                               + "&value=" + String(TempReading) + "";
      //Serial.print("httpRequestData: ");
      //Serial.println(httpRequestData);
      int httpResponseCode = http.POST(httpRequestData);

      if (httpResponseCode > 0) {
        //Serial.print("HTTP Response code: ");
        //Serial.println(httpResponseCode);
      }
      else {
        //Serial.print("Error code: ");
        //Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else  {
      Serial.println("WiFi Disconnected");
    }

}





float temp() {

  float temperature, temp;
  float sum;
  int i = 0;
  while (i < 30) {
    temp = mlx.readObjectTempC() + 10;
    sum += temp;
    i++;
    delay(1);
  }
  temperature = sum / 30.0;
  sum = 0;
  i = 0;
  //Serial.println(temperature);
  delay(50);
  return temperature ;
}





String rfid_port() {
  String onoma = "";

  if ( ! rfid.PICC_IsNewCardPresent())
    return "0";
  if (rfid.PICC_ReadCardSerial()) {
    for (byte i = 0; i < 4; i++) {
      tag += rfid.uid.uidByte[i];
    }
    //Serial.println(tag);
    if (tag == "19412120468") {
      onoma = "George";
    }
    else if (tag == "20819312550") {
      onoma = "Vasilis";
    }
    else if (tag == "2103616468") {
      onoma = "Kyriaki";
    }
    else {
      onoma = "1";
    }
    tag = "";
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
  return onoma;
}
