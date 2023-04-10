#include <WiFi.h>
#include <Arduino.h>
#include <WebServer.h>
#include <Stepper.h>

const bool debugMode = false;

//GLOBAL Constants
const int SoundSpeed = 0.034;

//pins leds
const int ledTest = 22;
const int ledStatut = 23;
//pins aimants step motor
const int IN1 = 19;
const int IN2 = 18;
const int IN3 = 5;
const int IN4 = 17;
//pins capteur distance
const int echo = 32;
const int trigger = 33;
//controle ouverture portail
bool isOpen = false;

//constant step motor
const int stepsPerRevolution = 512 ;
const int step = 2;
const int iteration = stepsPerRevolution / step;
const int rotationSpeed = 10;
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

//Config serveur web
const int port = 80;
const char* ssid = "Wifi_Portail";
//Had to be longer than 7 characters
const char* password = "1234567890";
WebServer server(port);

void setup(){
    //INIT
    Serial.begin(115200);
    //Pin mode
    pinMode(ledTest,OUTPUT);
    pinMode(ledStatut,OUTPUT);
    pinMode(IN1,OUTPUT);
    pinMode(IN2,OUTPUT);
    pinMode(IN3,OUTPUT);
    pinMode(IN4,OUTPUT);
    pinMode(trigger,OUTPUT);
    pinMode(echo,INPUT);
    if (!debugMode) {
      Serial.print("[DEBUG MODE] Credentials: \n -> SSID = ");
      Serial.print(ssid);
      Serial.print("\n ->PASSWORD = ");
      Serial.print(password); 
    }
    
    //init access point
    Serial.println("[*] Creating AP");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    Serial.print("[+] AP Created with IP Gateway ");
    Serial.println(WiFi.softAPIP());
    Serial.println("[*] Creating Routes");
    initRouter();
    //init motor
    myStepper.setSpeed(rotationSpeed);
    //init led
    digitalWrite(ledTest,HIGH);
}

void loop(){
  //Waiting for request
  server.handleClient();
}

// Web server functions
void initRouter(){
    server.on("/", handleRoot);
    server.on("/test", test);
    server.on("/open", openGate);
    server.on("/close", closeGate);
    Serial.println("[+] Route /test created");
    Serial.println("[+] Route /close created");
    Serial.println("[+] Route /open created");
    Serial.println("[*] Launch webserver");
    server.begin();
}


bool etatLed = 0;
char texteEtatLed[2][10] = {"ÉTEINTE!","ALLUMÉE!"};

//Home Page
void handleRoot()
{
    String page = "<!DOCTYPE html>";

    page += "<html lang='fr'>";

    page += "<head>";
    page += "    <title>Serveur ESP32</title>";
    page += "    <meta http-equiv='refresh' content='60' name='viewport' content='width=device-width, initial-scale=1' charset='UTF-8' />";
    page += "    <link rel='stylesheet' href='https://www.w3schools.com/w3css/4/w3.css'>";
    page += "</head>";

    page += "<body>";
    page += "    <div class='w3-card w3-blue w3-padding-small w3-jumbo w3-center'>";
    page += "        <p>ÉTAT LED: "; page += texteEtatLed[etatLed]; + "</p>";
    page += "    </div>";

    page += "    <div class='w3-bar'>";
    page += "        <a href='/open' class='w3-bar-item w3-button w3-border w3-jumbo' style='width:50%; height:50%;'>ON</a>";
    page += "        <a href='/close' class='w3-bar-item w3-button w3-border w3-jumbo' style='width:50%; height:50%;'>OFF</a>";
    page += "    </div>";

    page += "    <div class='w3-center w3-padding-16'>";
    page += "        <p>Ce serveur est hébergé sur un ESP32</p>";
    page += "        <i>Créé par Tommy Desrochers</i>";
    page += "    </div>";

    page += "</body>";

    page += "</html>";

    server.setContentLength(page.length());
    server.send(200, "text/html", page);
}

void test() {

  server.send(200, "application/json", "test");
}

//Gate control functions

void openGate() {
  if (isOpen == false) {
    digitalWrite(ledTest, LOW);
    for (int i = 0; i < iteration; i++) {
      stopWhenCrossing();
      myStepper.step(step);
      digitalWrite(ledStatut, HIGH);
      delay(50);
      digitalWrite(ledStatut, LOW);
    }
    digitalWrite(ledTest, HIGH);
    isOpen = true;
  }
  server.sendHeader("Location","/");
  server.send(303);
}

void closeGate() {
  if (isOpen == true) {
    digitalWrite(ledTest, LOW);
    for (int i = 0; i < iteration; i++) {
      stopWhenCrossing();
      myStepper.step(-step);
      digitalWrite(ledStatut, HIGH);
      delay(50);
      digitalWrite(ledStatut, LOW);
    }
    digitalWrite(ledTest, HIGH);
    isOpen = false;
  }
  server.sendHeader("Location","/");
  server.send(303);
}

//Security protocols

void stopWhenCrossing(){
  long duration;
  float distance;
  digitalWrite(trigger,LOW);
  delayMicroseconds(2);
  digitalWrite(trigger,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger,LOW);
  duration = pulseIn(echo, HIGH);
  distance = duration * SoundSpeed/2;
  Serial.println(distance);
  if (distance <= 3) {
    stopWhenCrossing();
  }
}
