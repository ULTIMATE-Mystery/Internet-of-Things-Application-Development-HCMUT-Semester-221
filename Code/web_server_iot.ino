//TODO: Update actuator status on server for physical buttons
// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "time.h"
#include "stdlib.h"
#include "ctype.h"

const char* ssid = "vmp";
const char* password = "25112002";

// const char* ssid = "HungREO";
// const char* password = "12345678";
//const char* ssid = "Quang";
//const char* password = "18012002";
//Setup pins for actuators
int HEAT_SYSTEM = 26, WATER_PUMP = 27, AIR_PUMP = 14, FISH_FEEDER = 12; 
int BUTTON[4] = {35, 32, 33, 25};
int manual[4] = { 0 };
int buttonStr = 0;
//Data variables
float temp, oxy, ph;
#define LOW_STATE 0
#define NORMAL  1
#define HIGH_STATE  2
int temp_flag = NORMAL, oxy_flag = NORMAL, ph_flag = NORMAL;
int real_h = 25, real_m = 60, real_s = 60;
int set_hour = 25, set_minute = 61, set_duration = 0;
int print_flag = 0;
int temp_counter[3] = { 0 }, oxy_counter[3] = { 0 }, ph_counter[3] = { 0 };
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readTemperature() {
  srand(time(NULL));
  if (temp_flag == LOW_STATE) {
    temp = (rand() % (229-200+1) + 200);
    
  }
  else if (temp_flag == NORMAL) {
    temp = (rand() % (270-230+1) + 230);
  }
  else {
    temp = (rand() % (300-281+1) + 281);
  }
  temp /= 10;
  return String(temp);
}

String readOxygen() {
  srand(time(NULL));
  if (oxy_flag == LOW_STATE) {
    oxy = (rand() % (59-40+1) + 40);
  }
  else if (oxy_flag == NORMAL) {
    oxy = (rand() % (80-60+1) + 60);
  }
  else {
    oxy = (rand() % (100-81+1) + 81);
  }
  oxy /= 10;
  return String(oxy);
}

String readPH() {
  srand(time(NULL));
  if (ph_flag == LOW_STATE) {
    ph = (rand() % (67-50+1) + 50);
  }
  else if (ph_flag == NORMAL) {
    ph = (rand() % (78-68+1) + 68);
  }
  else {
    ph = (rand() % (90-79+1) + 79);
  }
  ph /= 10;
  return String(ph);
}


bool isScheduledTime() {
  if (set_hour == real_h && set_minute == real_m && real_s <= set_duration) {
    if (!print_flag) {
      Serial.print(">> Activated auto feeder at ");
      if (real_h < 10) Serial.print("0" + String(real_h));
      else Serial.print(String(real_h));
      Serial.print(":");
      if (real_m < 10) Serial.println("0" + String(real_m));
      else Serial.println(String(real_m));
      print_flag = 1;
    }
    return true;
  }
  else {
    if (print_flag) {
      Serial.print(">> Deactivated auto feeder at ");
      if (real_h < 10) Serial.print("0" + String(real_h));
      else Serial.print(String(real_h));
      Serial.print(":");
      if (real_m < 10) Serial.println("0" + String(real_m));
      else Serial.println(String(real_m));
      print_flag = 0;
      set_hour = 88;
      set_minute = 88;
      set_duration = 88;
    }
    return false;
  }
}
int lastButtonState[4] = {0}, currentButtonState[4] = {1};
void buttonReading() {
  for (int i = 0; i < 4; i++) {
    lastButtonState[i] = currentButtonState[i];
    currentButtonState[i] = digitalRead(BUTTON[i]);
    if (currentButtonState[i] == 0 && lastButtonState[i] == 1) {
      manual[i] = !manual[i];
    } 
  }

}
void controlActuator() {
  //Heat system -> controlled by temperature
  if (manual[0] == 0) {
    if (temp_flag == LOW_STATE) {
      digitalWrite(HEAT_SYSTEM, HIGH);
    }
    else {
      digitalWrite(HEAT_SYSTEM, LOW);
    }
  }
  else {
    digitalWrite(HEAT_SYSTEM, HIGH);
  }
  //Water pump
  //ON: High temp/High oxy/High or low pH
  if (manual[1] == 0) {
    if (temp_flag == HIGH_STATE || oxy_flag == HIGH_STATE || ph_flag == LOW_STATE || ph_flag == HIGH_STATE) {
      digitalWrite(WATER_PUMP, HIGH);
    }
    else {
      digitalWrite(WATER_PUMP, LOW);
    }  
  }
  else {
    digitalWrite(WATER_PUMP, HIGH);
  }
  //AIR PUMP
  if (manual[2] == 0) {
    if (oxy_flag == LOW_STATE) {
      digitalWrite(AIR_PUMP, HIGH);
    }
    else {
      digitalWrite(AIR_PUMP, LOW);
    }   
  }
  else {
    digitalWrite(AIR_PUMP, HIGH);
  }
  //FISH FEEDER
  if (manual[3] == 0) {
    if (isScheduledTime()) {
      digitalWrite(FISH_FEEDER, HIGH);
    }
    else {
      digitalWrite(FISH_FEEDER, LOW);
    }  
  }
  else {
    digitalWrite(FISH_FEEDER, HIGH);
  }
}
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    .dashboard, .actuatorboard{
       text-align: center;
  }
   
  .circle-container {
       display: inline-block;
       margin: 5px;
  }
   
  #circle-temp, #circle-oxy, #circle-ph {
      border-size: 5px;
      border-style: solid;
      border-radius: 50%;
      display: table-cell;
      height: 125px;
      vertical-align: middle;
      width: 125px;
  }
   
  #temperature, #oxygen, #ph{
      color: gray;
      font-family: Arial, Helvetica, sans-serif;
      font-size: 35px;
  }
   
  .circle-text{
       color: gray;
       font-family: Arial, Helvetica, sans-serif;
       font-size: 12px;
  }
  .device-table {
    margin: 5px auto;
    display: inline-block;
    table-layout: fixed;
    font-family: Arial, Helvetica, sans-serif;
  }
  .device-status {
    margin: 0 auto;
    width: 80px;
  }
  .feeder-status {
    display: flex;
    flex-direction: row;
    align-items: center;
    justify-content: center;
  }
  #device-btn-1, #device-btn-2, #device-btn-3, #device-btn-4 {
    background-color: gray;
    width: 75px;
    height: 35px;
    margin: 0 auto;
  }
  .control-table {
    display: flex;
    align-items: center;
    justify-content: center;
  }
  </style>
</head>
<body>
    <div class="dashboard">
        <h1>DASHBOARD</h1>
        <div class="circle-container">
            <div id="circle-temp">
                <div><span id="temperature"></span>&deg;C</div>
                <div class="circle-text">
                    Temperature
                </div>
                <div id="temp-status"></div>
            </div>
        </div>
        <div class="circle-container">
            <div id="circle-oxy">
                <div><span id="oxygen"></span>&permil;</div>
                <div class="circle-text">
                    Oxygen
                </div>
                <div id="oxy-status"></div>
            </div>
        </div>
        <div class="circle-container">
            <div id="circle-ph">
                <span id="ph"></span>
                <div class="circle-text">
                    pH
                </div>
                <div id="ph-status"></div>
            </div>
        </div>
    </div>
    <div class="actuatorboard">
        <h1>ACTUATOR STATUS</h1>
        <table class="device-table">
            <tr>
              <th>Actuator name</th>
              <th>Status</th>
              <th>Control</th>
            </tr>
            <tr>
              <td>Heat system</td>
              <td><div class="device-status"><div id="heat-system"></div></div></td>
              <td><button id="device-btn-1" onclick="heat_on_off()"></button></td>
            </tr>
            <tr>
              <td>Extra water pump</td>
              <td><div class="device-status"><div id="water-pump"></div></div></td>
              <td><button id="device-btn-2" onclick="wp_on_off()"></button></td>
            </tr>
            <tr>
              <td>Air pump</td>
              <td><div class="device-status"><div id="air-pump"></div></div></td>
              <td><button id="device-btn-3" onclick="ap_on_off()"></button></td>
            </tr>
            <tr>
              <td>Fish feeder</td>
              <td><div class="device-status"><div id="fish-feeder"></div></div></td>
              <td><button id="device-btn-4" onclick="ff_on_off()"></button></td>
            </tr>
        </table>
        <div class="feeder-status">
        Fish feeder activated at&nbsp;
        <div id="feeder-hour"></div>:
        <div id="feeder-minute"></div>
        &nbsp;-&nbsp;Duration:&nbsp;
        <div id="feeder-duration"></div>&nbsp;second(s).
        </div>
    </div>
    
</body>
<script>
function turn_off(name) {
  document.getElementById(name).innerHTML = "OFF";
  document.getElementById(name).style.backgroundColor = "gray";
  document.getElementById(name).style.color = "white";
}
function turn_on(name) {
  document.getElementById(name).innerHTML = "ON";
  document.getElementById(name).style.backgroundColor = "blue";
  document.getElementById(name).style.color = "white";
}
manual_1 = 0, manual_2 = 0, manual_3 = 0, manual_4 = 0;
function heat_on_off() {
  if (manual_1 == 0) {
    document.getElementById("device-btn-1").style.backgroundColor = "blue";
    manual_1 = 1;
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "/control?hs=1", true);
    xhttp.send();
  }
  else {
    document.getElementById("device-btn-1").style.backgroundColor = "gray";
    manual_1 = 0;
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "/control?hs=0", true);
    xhttp.send();
  }

}
function wp_on_off() {
  if (manual_2 == 0) {
    document.getElementById("device-btn-2").style.backgroundColor = "blue";
    manual_2 = 1;
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "/control?wp=1", true);
    xhttp.send();
  }
  else {
    document.getElementById("device-btn-2").style.backgroundColor = "gray";
    manual_2 = 0;
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "/control?wp=0", true);
    xhttp.send();
  }

}
function ap_on_off() {
  if (manual_3 == 0) {
    document.getElementById("device-btn-3").style.backgroundColor = "blue";
    manual_3 = 1;
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "/control?ap=1", true);
    xhttp.send();
  }
  else {
    document.getElementById("device-btn-3").style.backgroundColor = "gray";
    manual_3 = 0;
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "/control?ap=0", true);
    xhttp.send();
  }

}
function ff_on_off() {
  if (manual_4 == 0) {
    document.getElementById("device-btn-4").style.backgroundColor = "blue";
    manual_4 = 1;
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "/control?ff=1", true);
    xhttp.send();
  }
  else {
    document.getElementById("device-btn-4").style.backgroundColor = "gray";
    manual_4 = 0;
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "/control?ff=0", true);
    xhttp.send();
  }

}
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      manual_1 = buttonArr[0].toInt();
      manual_2 = buttonArr[1].toInt();
      manual_3 = buttonArr[2].toInt();
      manual_4 = buttonArr[3].toInt();
    }
  };
  xhttp.open("GET", "/button", true);
  xhttp.send();
}, 1);
temp_state = 1, oxy_state = 1, ph_state = 1, sethour = 88, setminute = 88, setduration = 88;

setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      sethour = this.responseText; 
    }
  }; 
  xhttp.open("GET", "/sethour", true);
  xhttp.send();
}, 1000) ;
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      setminute = this.responseText; 
    }
  }; 
  xhttp.open("GET", "/setminute", true);
  xhttp.send();
}, 1000) ;
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      setduration = this.responseText; 
    }
  }; 
  xhttp.open("GET", "/setduration", true);
  xhttp.send();
}, 1000) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 3000 ) ;
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      temp_state = parseInt(this.responseText);
      if (temp_state == 2) {
          document.getElementById("temp-status").innerHTML = "";
          document.getElementById("circle-temp").style.borderColor = "red";
        }
        else if (temp_state == 1) {
          document.getElementById("temp-status").innerHTML = "";
          document.getElementById("circle-temp").style.borderColor = "aquamarine";
        }
        else {
          document.getElementById("temp-status").innerHTML = "";
          document.getElementById("circle-temp").style.borderColor = "blue";
        }
    }
  };
  xhttp.open("GET", "/checktemp", true);
  xhttp.send(); 
}, 1);

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("oxygen").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/oxygen", true);
  xhttp.send();
}, 3000 ) ;
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      oxy_state = parseInt(this.responseText);
      if (oxy_state == 2) {
          document.getElementById("oxy-status").innerHTML = "";
          document.getElementById("circle-oxy").style.borderColor = "red";
        }
        else if (oxy_state == 1) {
          document.getElementById("oxy-status").innerHTML = "";
          document.getElementById("circle-oxy").style.borderColor = "aquamarine";
        }
        else {
          document.getElementById("oxy-status").innerHTML = "";
          document.getElementById("circle-oxy").style.borderColor = "blue";
        }
    }
  };
  xhttp.open("GET", "/checkoxy", true);
  xhttp.send(); 
}, 1);
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ph").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/ph", true);
  xhttp.send();
}, 3000 ) ;
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      ph_state = parseInt(this.responseText);
      if (ph_state == 2) {
          document.getElementById("ph-status").innerHTML = "";
          document.getElementById("circle-ph").style.borderColor = "red";
        }
        else if (ph_state == 1) {
          document.getElementById("ph-status").innerHTML = "";
          document.getElementById("circle-ph").style.borderColor = "aquamarine";
        }
        else {
          document.getElementById("ph-status").innerHTML = "";
          document.getElementById("circle-ph").style.borderColor = "blue";
        }
    }
  };
  xhttp.open("GET", "/checkph", true);
  xhttp.send(); 
}, 1);
setInterval( function () {
  var currDate = new Date();
  var h, m, s;
  if (currDate.getHours() < 10) {
    h = "0" + currDate.getHours();
  }
  else {
    h = currDate.getHours();
  }
  if (currDate.getMinutes() < 10) {
    m = "0" + currDate.getMinutes();
  }
  else {
    m = currDate.getMinutes();
  }
  if (currDate.getSeconds() < 10) {
    s = "0" + currDate.getSeconds();
  }
  else {
    s = currDate.getSeconds();
  }
  if (currDate.getHours() == sethour && currDate.getMinutes() == setminute && currDate.getSeconds() <= setduration) {
    turn_on("fish-feeder");
    if (sethour < 10) {
      document.getElementById("feeder-hour").innerHTML = "0"+ sethour; 
    }
    else {
      document.getElementById("feeder-hour").innerHTML = sethour;
    }
    if (setminute < 10) {
      document.getElementById("feeder-minute").innerHTML = "0"+ setminute; 
    }
    else {
      document.getElementById("feeder-minute").innerHTML = setminute;
    }
    if (setduration < 10) {
      document.getElementById("feeder-duration").innerHTML = "0"+ setduration; 
    }
    else {
      document.getElementById("feeder-duration").innerHTML = setduration;
    }
    
  }
  else {
    document.getElementById("feeder-hour").innerHTML = "--";
    document.getElementById("feeder-minute").innerHTML = "--";
    document.getElementById("feeder-duration").innerHTML = "--";
    turn_off("fish-feeder");
  }
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/time?hour="+h+"&min="+m+"&sec="+s, true);
  xhttp.send();
}, 1000);

setInterval(function () {
  if (manual_1 == 0) {
    if (temp_state == "0") {
      turn_on("heat-system");
    }
    else {
      turn_off("heat-system");
    }
  }
  else {
    turn_on("heat-system");
  }
  if (manual_2 == 0) {
    if (temp_state == 2 || oxy_state == 2 || ph_state == 0 || ph_state == 2) {
      turn_on("water-pump");
    }
    else {
      turn_off("water-pump");
    }
  }
  else {
    turn_on("water-pump");
  }
  if (manual_3 == 0) {
    if (oxy_state == 0) {
      turn_on("air-pump"); 
    }
    else {
      turn_off("air-pump");
    } 
  }
  else {
    turn_on("air-pump");
  }
  if (manual_4 == 1) {
    turn_on("fish-feeder");
  }
}, 1);

</script>
</html>)rawliteral";

String processor(const String& var){
  if(var == "TEMPERATURE"){
    return readTemperature();
  }
  else if(var == "OXYGEN"){
    return readOxygen();
  }
  else if (var == "PH"){
    return readPH();
  }
  else if (var =="SETHOUR"){
    return String(set_hour);
  }
  else if (var =="SETMIN"){
    return String(set_minute);
  }
  return String();
}

void setup(){
  //Init output pin for actuators
  pinMode(HEAT_SYSTEM, OUTPUT);
  digitalWrite(HEAT_SYSTEM, LOW);
  pinMode(WATER_PUMP, OUTPUT);
  digitalWrite(WATER_PUMP, LOW);
  pinMode(AIR_PUMP, OUTPUT);
  digitalWrite(AIR_PUMP, LOW);
  pinMode(FISH_FEEDER, OUTPUT);
  digitalWrite(FISH_FEEDER, LOW);
  for (int i = 0; i < 4; i++) {
    pinMode(BUTTON[i], INPUT_PULLUP); 
  }
 
  // Serial port for debugging purposes
  Serial.begin(115200);

  // Connect to Wi-Fi
//  WiFi.mode(WIFI_AP); 
//  WiFi.softAP(ssid, password);
 WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");

  }
  
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  Serial.println(">> Command: sensor-range, eat-hh-mm-duration, time, reset");
  Serial.println(">> Sensors: temp, oxygen, ph");
  Serial.println(">> Ranges: low, normal, high");
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readTemperature().c_str());
  });
  server.on("/oxygen", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readOxygen().c_str());
  });
  server.on("/ph", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readPH().c_str());
  });
  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("hour") && request->hasParam("min") && request->hasParam("sec")) {
      real_h = (request->getParam("hour")->value()).toInt();
      real_m = (request->getParam("min")->value()).toInt();
      real_s = (request->getParam("sec")->value()).toInt();
      //Serial.println("Get real time.");
    }
    else {
      Serial.println("Get real time failed.");
    }
    request->send_P(200, "text/plain", "OK");
  });
  //Check sensor values range
  server.on("/checktemp", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(temp_flag).c_str());
  });
  server.on("/checkoxy", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(oxy_flag).c_str());
  });
  server.on("/checkph", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(ph_flag).c_str());
  });
  //Display the time user want to activate auto feeder
  server.on("/sethour", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(set_hour).c_str());
  });
  server.on("/setminute", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(set_minute).c_str());
  });
  server.on("/setduration", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(set_duration).c_str());
  });
  server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("hs")) {
      manual[0] = (request->getParam("hs")->value()).toInt();
    }
    else if (request->hasParam("wp")) {
      manual[1] = (request->getParam("wp")->value()).toInt();

    }
    else if (request->hasParam("ap")) {
      manual[2] = (request->getParam("ap")->value()).toInt();
    }
    else if (request->hasParam("ff")) {
      manual[3] = (request->getParam("ff")->value()).toInt();
    }
    request->send_P(200, "text/plain", "OK");
  });

  server.on("/button", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(buttonStr).c_str());
  });
    // Start server
  server.begin();
}
char receivedChars[32];
boolean newData = false;
void receiveData() {
  boolean still_rc = false;
  int index = 0;
  char rc, startChar = '!', endChar = '!';
  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();
    if (still_rc) {
      if (rc != endChar) {
        receivedChars[index] = rc;
        index++;
        if (index >= 32) {
          index = 31;
        }
      }
      else {
        receivedChars[index] = '\0';
        still_rc = false;
        index = 0;
        newData = true;
      }
    }
    else if (rc == startChar) {
      still_rc = true;
    }
  }
}

void showData() {
  if (newData == true) {
    Serial.print(">> ");
    Serial.println(receivedChars);
    if (strcmp(receivedChars, "temp-low") == 0) {
      Serial.println(">> Successfully changed temperature to low range");
      temp_flag = LOW_STATE;
    }
    else if (strcmp(receivedChars, "temp-normal") == 0) {
      Serial.println(">> Successfully changed temperature to normal range");
      temp_flag = NORMAL;
    }
    else if (strcmp(receivedChars, "temp-high") == 0) {
      Serial.println(">> Successfully changed temperature to high range");
      temp_flag = HIGH_STATE;
    }
    else if (strcmp(receivedChars, "oxygen-low") == 0) {
      Serial.println(">> Successfully changed oxygen rate to low range");
      oxy_flag = LOW_STATE;
    }
    else if (strcmp(receivedChars, "oxygen-normal") == 0) {
      Serial.println(">> Successfully changed oxygen rate to normal range");
      oxy_flag = NORMAL;
    }
    else if (strcmp(receivedChars, "oxygen-high") == 0) {
      Serial.println(">> Successfully changed oxygen rate to high range");
      oxy_flag = HIGH_STATE;
    }
    else if (strcmp(receivedChars, "ph-low") == 0) {
      Serial.println(">> Successfully changed pH to low range");
      ph_flag = LOW_STATE;
    }
    else if (strcmp(receivedChars, "ph-normal") == 0) {
      Serial.println("Successfully changed pH to normal range");
      ph_flag = NORMAL;
    }
    else if (strcmp(receivedChars, "ph-high") == 0) {
      Serial.println(">> Successfully changed pH to high range");
      ph_flag = HIGH_STATE;
    }
    else if (strcmp(receivedChars, "reset") == 0) {
      Serial.println(">> Reset all sensor values to normal range");
      temp_flag = NORMAL;
      oxy_flag = NORMAL;
      ph_flag = NORMAL;
    }
    else if (receivedChars[0] == 'e' && receivedChars[1] == 'a' && receivedChars[2] == 't') {
      if (receivedChars[3] == '-' && receivedChars[6] == '-') {
        if (isDigit(receivedChars[4]) && isDigit(receivedChars[5])) {
          set_hour = (receivedChars[4] - '0') * 10 + (receivedChars[5] - '0');  
        }
        else {
          Serial.println(">> Invalid hour.");
          return;
        }
        if (isDigit(receivedChars[7]) && isDigit(receivedChars[8])) {
          set_minute = (receivedChars[7] - '0') * 10 + (receivedChars[8] - '0');
        }
        else {
          Serial.println(">> Invalid minute.");
          return;
        }
        if (isDigit(receivedChars[10]) && isDigit(receivedChars[11])) {
          set_duration = (receivedChars[10] - '0') * 10 + (receivedChars[11] - '0');
        }
        else {
          Serial.println(">> Invalid duration.");
          return;
        }
      }
      Serial.print(">> Successfully setup automatic feeder at ");
      Serial.print(String(set_hour));
      Serial.print(":");
      Serial.print(String(set_minute));
      Serial.print(" with duration is ");
      Serial.print(set_duration);
      Serial.println(" seconds.");
    }
    else if (strcmp(receivedChars, "time") == 0) {
      Serial.print(">> Curent time is: ");
      if (real_h < 10) Serial.print("0" + String(real_h));
      else Serial.print(String(real_h));
      Serial.print(":");
      if (real_m < 10) Serial.print("0" + String(real_m));
      else Serial.print(String(real_m));
      Serial.print(":");
      if (real_s < 10) Serial.println("0" + String(real_s));
      else Serial.println(String(real_s));
    }
    else {
      Serial.println(">> Invalid command, try again.");
    }
    newData = false;
  }
  
}

void loop(){ 
  buttonReading();
  buttonStr = 1000*manual[0] + 100*manual[1] + 10*manual[2] + 1*manual[3];
  controlActuator();
  receiveData();
  showData();
}
