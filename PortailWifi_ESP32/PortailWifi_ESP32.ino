#include <WiFi.h>
#include <Arduino.h>
#include <WebServer.h>
#include <Stepper.h>

const bool debugMode = false;
//GLOBAL Constants
const float SoundSpeed = 0.034;
float timeStart;
float timeTotal;
//pins leds
const int ledTest = 22;
const int ledStatut = 23;
//pins aimants step motor
const int IN1 = 19;
const int IN2 = 18;
const int IN3 = 5;
const int IN4 = 17;
//pins capteur distance
const int echo = 14;
const int trigger = 12;
//controle ouverture portail
bool isOpen = false;
 
//constant step motor
const int stepsPerRevolution = 512 ;
const int step = 4;
const int iteration = stepsPerRevolution / step;
const int rotationSpeed = 10;
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

//Config serveur web
const int port = 80;
const char* ssid = "Wifi_Portail";
//Had to be longer than 7 characters
const char* password = "1234567890";
WebServer server(port);

//logs
String logs = "<a href='/'> Retour</a> <br> Logs Gate:<br>";

void setup(){
    //INIT
    Serial.begin(9600);
    Serial.println(logs);
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
    server.on("/logs", sendLogs);
    server.on("/open", openGate);
    server.on("/close", closeGate);
    Serial.println("[+] Route /logs created");
    Serial.println("[+] Route /close created");
    Serial.println("[+] Route /open created");
    Serial.println("[*] Launch webserver");
    server.begin();
}

char statutPortail[2][10] = {"Fermé! ","Ouvert!"};

//Home Page
void handleRoot()
{
    String page = "<!DOCTYPE html>";

    page += "<html lang='fr'>";

    page += "<head>";
    page += "    <title>Portail Wifi</title>";
    page += "    <meta http-equiv='refresh' content='60' name='viewport' content='width=device-width, initial-scale=1' charset='UTF-8' />";
    page += "    <link rel='stylesheet' href='https://www.w3schools.com/w3css/4/w3.css'>";
    page += "</head>";

    page += "<body>";
    page += "    <div class='w3-card w3-blue w3-padding-small w3-jumbo w3-center'>";
    page += "        <p>Le portail est "; page += statutPortail[isOpen]; + "</p>";
    page += "    </div>";

    page += "    <div class='w3-bar'>";
    page += "        <a href='/open' class='w3-bar-item w3-button w3-border w3-jumbo' style='width:50%; height:50%;'>Open</a>";
    page += "        <a href='/close' class='w3-bar-item w3-button w3-border w3-jumbo' style='width:50%; height:50%;'>Close</a>";
    page += "        <a href='/logs' class='w3-bar-item w3-button w3-border w3-jumbo' style='width:50%; height:50%;'>Show logs</a>";
    page += "    </div>";

    page += "</body>";

    page += "</html>";

    server.setContentLength(page.length());
    server.send(200, "text/html", page);
}

//Gate control functions

void openGate() {
  if (isOpen == false) {
    timeStart = millis();
    logs += "Opening gate... <br>";
    digitalWrite(ledTest, LOW);
    for (int i = 0; i < iteration; i++) {
      stopWhenCrossing(1);
      digitalWrite(ledStatut, HIGH);      
      myStepper.step(step);
      digitalWrite(ledStatut, LOW);
    }
    digitalWrite(ledTest, HIGH);
    isOpen = true;
    timeTotal = (millis() - timeStart)*0.001;    
    logs += "Gate open in " + String(timeTotal) + " seconds <br>" ;
    if (timeTotal > 7.5) {
      logs += "Something blocked the gate. <br>";
    }
  }
  server.sendHeader("Location","/");
  server.send(303);
}

void closeGate() {
  if (isOpen == true) {
    timeStart = millis();
    logs += "Closing gate... <br>";
    digitalWrite(ledTest, LOW);
    for (int i = 0; i < iteration; i++) {
      stopWhenCrossing(1);
      digitalWrite(ledStatut, HIGH);      
      myStepper.step(-step);
      digitalWrite(ledStatut, LOW);
    }
    digitalWrite(ledTest, HIGH);
    isOpen = false;
    timeTotal = (millis() - timeStart)*0.001;    
    logs += "Gate closed "+ String(timeTotal) + " seconds <br>";
    if (timeTotal > 7.5) {
      logs += "Something blocked the gate. <br>";
    }
  }
  server.sendHeader("Location","/");
  server.send(303);
}

//logs

void sendLogs(){
  server.send(200, "text", logs);
}

//Security protocols

void stopWhenCrossing(int status){
  if (status = 1){
    //logs += "Gate blocked \n";
  }
  long duration;
  float distance;
  digitalWrite(trigger,LOW);
  delayMicroseconds(2);
  digitalWrite(trigger,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger,LOW);
  duration = pulseIn(echo, HIGH);
  distance = duration * SoundSpeed/2 ;
  Serial.println(distance);
  if (distance <= 10) {
    stopWhenCrossing(0);
  }
}
