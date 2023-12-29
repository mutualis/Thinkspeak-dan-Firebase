//GPIO17 itu tx2
//GPIO16 itu rx2
#include <WiFi.h>
#include <WiFiClient.h>
#include <IOXhop_FirebaseESP32.h>
#include <ThingSpeak.h>
const char* server = "api.thingspeak.com";

#define FIREBASE_Host "https://pbl2023-fb699-default-rtdb.firebaseio.com/"    
#define FIREBASE_authorization_key "XDxsKWs9qRmIeHw10Y2YPI3qAy0gz7OOFjm6MY25"  
#define Your_SSID "PBL2023"                                                  
#define Your_PASSWORD "00000000"                                          

unsigned long channel = 2371386;
const char* myCounterWriteAPIKey = "J8DEFJDH5XAE5L1M";
WiFiClient client;

unsigned long lastTime = 0;
unsigned long timerDelay = 30000;
unsigned long lastTime1 = 0;
unsigned long timerDelay1 = 10000;

String data;
String dataIn;
String dt[20];
int i;
int val;
boolean parsing = false;
bool cw = 0, ccw = 0;
int speed, dutyCycle;
float volt, voltMo;

void setup() {
  Serial2.begin(9600);
  Serial.begin(9600);
  dataIn = "";
  WiFi.begin(Your_SSID, Your_PASSWORD);
  Serial.print("Connecting to WIFI");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("Connected to WIFI!");
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());                             //print local IP address
  Firebase.begin(FIREBASE_Host, FIREBASE_authorization_key);  // connect to firebase
  ThingSpeak.begin(client);
}
void loop() {
  if (Serial2.available() > 0) {
    char inChar = (char)Serial2.read();
    dataIn += inChar;
    if (inChar == '\n') {
      parsing = true;
    }
  }

  if (parsing) {
    val = dataIn.toInt();
    volt = map(val, 0, 1023, 0, 500);
    if (volt >= 0 && volt <= 100) {
      cw = 0;
      ccw = 0;
      speed = 0;
      voltMo = 0;
    } else if (volt > 100 && volt <= 300) {
      cw = 1;
      ccw = 0;
      speed = map(volt, 101, 300, 0, 1800);
      dutyCycle = map(speed, 0, 1800, 0, 255);
      voltMo = map(dutyCycle, 0, 255, 0, 900);
    } else if (volt > 300 && volt <= 500) {
      cw = 0;
      ccw = 1;
      speed = map(volt, 301, 500, 0, 1800);
      dutyCycle = map(speed, 0, 1800, 0, 255);
      voltMo = map(dutyCycle, 0, 255, 0, 900);
    }
    parsing = false;
    dataIn = "";
  }


  if ((millis() - lastTime1) > timerDelay1) {
    Firebase.set("dataPBL/val", val);
    Serial.print("val : ");
    Serial.print(val);
    Serial.print("\n\n");
    Serial.println("Success");
    lastTime1 = millis();
  }
  if ((millis() - lastTime) > timerDelay) {
    ThingSpeak.setField(1, cw);
    ThingSpeak.setField(2, ccw);
    ThingSpeak.setField(3, speed);
    ThingSpeak.setField(4, voltMo/100);
    int x = ThingSpeak.writeFields(channel, myCounterWriteAPIKey);
    if (x == 200) {
      Serial.println("Channel update successful.");
    } else {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    lastTime = millis();
  }
}
