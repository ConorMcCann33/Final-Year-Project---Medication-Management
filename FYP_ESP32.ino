/* To Do:
  Add a way of getting custom/unique device id's
  Database security

  Update alarms on alarm time Change
  heart beats slowing alarm
  Make notification time changeable
  If wifi fails keeps sending Nextion disconnected event (Line 596)
  Move refill request to after the motor runs in the alert system and add it to the startup
  */

//Packages
  #include <Stepper.h>
  #include <WiFi.h>
  #include <WiFiClientSecure.h>
  #include "time.h"

  //ESP Pin Nums
  const int stepPin = 13;
  const int dirPin = 12;
  const int ledPin = 5;
  const int buzzerPin = 4;

  //Global Variables
  int mode = 3; // Mode 0 is Live, Mode 1 is test, Mode 2 is Demo Mode
  bool LED = 0; // Used to flash LED 
  bool testAlert = false; // Flag for when test alert is used
  int doses = 0;

  String a1 = "N/A"; // String for Alarm 1
  String a2 = "N/A"; // String for Alarm 2
  String a3 = "N/A"; // String for Alarm 3
  String a4 = "N/A"; // String for Alarm 4

  //UART
  HardwareSerial nextion(1);  // Sets Nextion UART to channel 1
  String incomingMessage = ""; // String for UART Message storage

  //Checks
  bool wifiSentDisconnected = true;
  bool startUp = true;
  bool alertActive = false; // Check that alert is active
  bool alertCancel = false;
  bool timerRunning = false; // Check that alert timer is counting down
  bool notified = false; // Check that the notification has been sent
  bool wifiSetup = false; // Check that wifi has been set up
  bool wifiConnected = false;
  bool refilled = false;
  //Timer
  unsigned long alertStartTime = 0;
  unsigned long alertTimeout = 0;
  int delayminutes = 0;

  //Wifi
  String ssid = "";
  String password = "";

  const int WIFI_MAX_RETRIES = 5;
  const unsigned long WIFI_RETRY_INTERVAL = 2000;

  int wifiRetryCount = 0;
  unsigned long lastWifiAttempt = 0;

  unsigned long lastHeartbeat = 0;
  const unsigned long HEARTBEAT_INTERVAL_SEC  = 30;

  //Firebase
  const char* FIREBASE_HOST = "fyp-medication-dispenser-default-rtdb.europe-west1.firebasedatabase.app";
  const char* FIREBASE_SECRET = ""; //Removed for Server Safety

  WiFiClientSecure firebaseClient;

  String deviceId = "device_001";

  //Log
  String alarmtime;
  String timetaken;

  struct LogEntry {
  String alarmtime;
  String timetaken;
  int delayminutes;
  };
  LogEntry logEntries[28];
  int logCount = 0;

  //Time
  const char* ntpServer = "pool.ntp.org";
  const long  gmtOffset_sec = 0;
  const int   daylightOffset_sec = 0;

void setup() { 
  Serial.begin(115200); // Sets serial monitor baud rate
  Serial.println("ESP32 starting...");

  nextion.begin(115200, SERIAL_8N1 , 1, 2);
  Serial.println("Nextion UART ready");
  firebaseClient.setInsecure();
  // Initialize pins
  pinMode(stepPin,OUTPUT);
  pinMode(dirPin,OUTPUT);
  pinMode(ledPin,OUTPUT);
  pinMode(buzzerPin,OUTPUT);
  digitalWrite(stepPin, LOW);  // Ensure motor driver disabled
  digitalWrite(dirPin, LOW);   // Ensure correct direction idle
  WiFi.onEvent(WiFiEvent);
}
void loop(){
  receiver();
  if(startUp == false){
    handleWiFiReconnect();
    sendHeartbeat();
    if(alertActive == true){
      handleAlert();
    }
  }
}
void initialisation(){
  refill();
  startUp = false;
  Serial.println("Setup Finished");

  switch (mode){
  case 0:
    Serial.println("New Setup chosen");
    break;

  case 1:
    Serial.println("Demo Setup chosen");
    break;

  case 2:
    Serial.println("Sync Setup chosen");
    sync();
    break;

  default:
    Serial.println("Error, No mode selected");
    break;
  
  }
}
void alert(){
  updateServer("/status/alert", "Active");
  startAlertTimer(1);
}
void notify(String message) {
  Serial.println("Send Notification");

  bool valid = 
  (message == "refill") ||
  (message == "missed") ||
  (message == "late");
  if(!valid) return;

  updateServer("/status/notify", message);
}
void refill(){
  notify("refill");
  sendToNextion("empty", "1");
  digitalWrite(ledPin, HIGH);
}
void motor(int steps){
  for(steps; steps > 0; steps--) 
  {
    digitalWrite(stepPin,HIGH);
    delay(50);
    digitalWrite(stepPin,LOW);
    delay(50);
    Serial.print("Steps to go: ");
    Serial.println(steps);
  }
}
void alarm(){
  //Alarm Code
  if(LED == 1){
    digitalWrite(ledPin, LOW);
    LED = 0;
  } 
  else if(LED == 0){
    digitalWrite(ledPin, HIGH);
    LED = 1;  
  } 
  for(int i=0;i<5;i++){
    digitalWrite(buzzerPin,HIGH);
    delay(50);
    digitalWrite(buzzerPin,LOW);
    delay(100);
  }
}
void receiver() {
  if (nextion.available()) {
    String msg = nextion.readStringUntil(';');
    msg.trim();
    if(msg.length() == 0){
      Serial.println("Length of 0");
      return;
    }
    bool valid =
      msg.startsWith("T,") ||
      msg.startsWith("A,") ||
      msg.startsWith("S,") ||
      msg.startsWith("I,") ||
      msg.startsWith("M,") ||
      msg.startsWith("W,") ||
      msg.startsWith("L,") ||
      msg.startsWith("F") ||
      msg.startsWith("R");

    if(!valid){
      Serial.println("Not Valid");
      return;
    }

    Serial.print("Received: ");
    Serial.println(msg);

    //Example:T,08:50;
    if (msg.startsWith("T,")) {
      Serial.println("Alert to be Cancelled");
      alertCancel = true;
        // Turn off outputs here
      int firstComma = msg.indexOf(',');

      timetaken = msg.substring(firstComma + 1);

      Serial.print("Time: ");
      Serial.println(timetaken);
    }
    //Example:A,1,08:50;
    if (msg.startsWith("A,")) {
      Serial.println("Alert recieved");
      int firstComma = msg.indexOf(',');
      int secondComma = msg.indexOf(',', firstComma + 1);

      int alarm = msg.substring(firstComma + 1, secondComma).toInt();
      alarmtime = msg.substring(secondComma + 1);

    
      if (alarm<5){
        testAlert = false;
        Serial.print("Alarm: ");
        Serial.println(alarm);
        alert();
      }
      if (alarm==5){
        testAlert = true;
        Serial.println("Test Alert");
        alert();
      }
    }
    //Example:S,1,12,15;
    if (msg.startsWith("S,")) {
      int firstComma = msg.indexOf(',');
      int secondComma = msg.indexOf(',', firstComma + 1);
      int thirdComma = msg.indexOf(',', secondComma + 1);
      int fourthComma = msg.indexOf(',', thirdComma + 1);

      int al_num = msg.substring(firstComma + 1, secondComma).toInt();
      int a_hr = msg.substring(secondComma + 1, thirdComma).toInt();
      int a_min = msg.substring(thirdComma + 1, fourthComma).toInt();
      int checked = msg.substring(fourthComma + 1).toInt();
      al_update(al_num, a_hr, a_min, checked);
    }
    //Example:I,1
    if (msg.startsWith("I,")) {
      int firstComma = msg.indexOf(',');
      mode = msg.substring(firstComma + 1).toInt();
      initialisation();
    }
    //Example:M,1
    if (msg.startsWith("M,")) {
      int firstComma = msg.indexOf(',');
      int steps = msg.substring(firstComma + 1).toInt();
      motor(steps);
      doses -= 1;
    }
    //Example:W,SSID,Password;
    if (msg.startsWith("W,")) {
      int firstComma = msg.indexOf(',');
      int secondComma = msg.indexOf(',', firstComma + 1);

      ssid = msg.substring(firstComma + 1, secondComma);
      password = msg.substring(secondComma + 1);

      Serial.print("SSID: ");
      Serial.println(ssid);
      Serial.print("Password: ");
      Serial.println(password);
      wifiSentDisconnected = false;
      connectToWiFi();
    }
    //Example:L,1;
    if (msg.startsWith("L,")) {
      int firstComma = msg.indexOf(',');
      int page = msg.substring(firstComma + 1).toInt();
      sendlog(page);
    }
    //F;
    if (msg.startsWith("F")) {
      reset();
    }
    //R;
    if (msg.startsWith("R")) {
      refilled = true;
      doses = 28;
      digitalWrite(ledPin, LOW);
    }
  }
}
void al_update(int al_num, int a_hr, int a_min, int checked){
  char timeBuffer[6];   // "HH:MM" + null terminator
  sprintf(timeBuffer, "%02d:%02d", a_hr, a_min);
  Serial.print("Alarm: ");
  Serial.println(al_num);
  switch(al_num){
    case 1:
      switch(checked){
        case 0:
          a1 = "N/A";
          Serial.print("Time not set");
          break;

        case 1:
          a1 = String(timeBuffer);
          Serial.print("New Time: ");
          Serial.println(a1);
          break;
      
        default:
          Serial.print("Error: Checked not 0 or 1");
          break;
      }
      updateServer("/alarms/alarm1", a1);
      break;

    case 2:
      switch(checked){
        case 0:
          a2 = "N/A";
          Serial.print("Time not set");
          break;
          
        case 1:
          a2 = String(timeBuffer);
          Serial.print("New Time: ");
          Serial.println(a2);
          break;

        default:
          Serial.print("Error: Checked not 0 or 1");
          break;
      }
      updateServer("/alarms/alarm2", a2);
      break;

    case 3:
      switch(checked){
        case 0:
          a3 = "N/A";
          Serial.print("Time not set");
          break;

        case 1:
          a3 = String(timeBuffer);
          Serial.print("New Time: ");
          Serial.println(a3);
          break;

        default:
          Serial.print("Error: Checked not 0 or 1");
          break;
      }

      updateServer("/alarms/alarm3", a3);
      break;

    case 4:
      switch(checked){
        case 0:
          a4 = "N/A";
          Serial.print("Time not set");
          break;
          
        case 1:
          a4 = String(timeBuffer);
          Serial.print("New Time: ");
          Serial.println(a4);
          break;

        default:
          Serial.print("Error: Checked not 0 or 1");
          break;
      }
      updateServer("/alarms/alarm4", a4);
      break;

    default:
      Serial.print("Error: Alarm Number not found");
      break;
    
  }
}
void sendToNextion(String loc, String update) {
  nextion.print(loc + "=" + update);
  nextion.write(0xFF);
  nextion.write(0xFF);
  nextion.write(0xFF);
}
void startAlertTimer(unsigned long timeoutMs) {
  timeoutMs *= 60UL * 1000UL;
  alertActive = true;
  timerRunning = true;
  notified = false;

  alertTimeout = timeoutMs;
  alertStartTime = millis();

  Serial.println("Alert timer started");
}
void handleAlert() {
  if(alertCancel == true){
    Serial.println("Alert Cancelled");
    alertActive = false;
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzerPin, LOW);
    cancelAlert();
    delayminutes = (millis() - alertStartTime)/60000;
    Serial.print("Taken after: ");
    Serial.println(delayminutes);
    if(delayminutes > 30){
      notify("late");
    }
    updateServer("/status/alert", "Not Active");
    alertCancel = false; 
    if(testAlert == true) return;
    addlog(alarmtime, timetaken, delayminutes);
    motor(10);
    if(doses <= 0){
        refilled = false;
        refill();
    }
    
  }
  if (timerRunning && !notified) {
    alarm();
    Serial.println((millis() - alertStartTime)/60000);
    if (millis() - alertStartTime >= alertTimeout) {
      notify("missed");
      notified = true;
      cancelAlert(); // Cancel alert 
    }
  }
}
void cancelAlert() {
  timerRunning = false;
  Serial.println("Alarm cancelled");
}
void connectToWiFi() {

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Already connected");
    return;
  }

  Serial.print("Connecting to network: ");
  Serial.println(ssid);

  wifiRetryCount = 0;
  lastWifiAttempt = 0;

  WiFi.disconnect(true);
  delay(200);

  WiFi.begin(ssid.c_str(), password.c_str());
}
void updateServer(String address, String value) {
  if (!wifiConnected) {
    Serial.println("WiFi offline, skipping Firebase update");
    return;
  }


  String path ="/devices/" + deviceId + address + ".json?auth=" + FIREBASE_SECRET;
  String payload = "\"" + value + "\"";

  Serial.print("Updating Firebase: ");
  Serial.println(path);

  if (!firebaseClient.connect(FIREBASE_HOST, 443)) {
    Serial.println("Firebase connection FAILED");
    return;
  }

  firebaseClient.println("PUT " + path + " HTTP/1.1");
  firebaseClient.println("Host: " + String(FIREBASE_HOST));
  firebaseClient.println("Content-Type: application/json");
  firebaseClient.print("Content-Length: ");
  firebaseClient.println(payload.length());
  firebaseClient.println();
  firebaseClient.println(payload);

  while (firebaseClient.connected()) {
    if (firebaseClient.readStringUntil('\n') == "\r") break;
  }

  String response = firebaseClient.readString();
  Serial.println("Firebase response:");
  Serial.println(response);

  firebaseClient.stop();
}
void addlog(String alarmtime, String timetaken, int delayminutes){
  for(int i = 27; i > 0; i--){
    logEntries[i]=logEntries[i-1];

    Serial.print("Entry ");
    Serial.print(i);
    Serial.print(": AlarmTime=");
    Serial.print(logEntries[i].alarmtime);
    Serial.print(", TimeTaken=");
    Serial.print(logEntries[i].timetaken);
    Serial.print(", DelayMinutes=");
    Serial.println(logEntries[i].delayminutes);
  }
 
  logEntries[0] = { alarmtime, timetaken, delayminutes};

  Serial.print("Entry ");
  Serial.print(0);
  Serial.print(": AlarmTime=");
  Serial.print(logEntries[0].alarmtime);
  Serial.print(", TimeTaken=");
  Serial.print(logEntries[0].timetaken);
  Serial.print(", DelayMinutes=");
  Serial.println(logEntries[0].delayminutes);

  uploadLogToFirebase();
}
void sendlog(int page){
  for(int i = 1; i <= 4; i++){
    int index = i-1 + page * 4;
    if(index >= 28){
      Serial.println("Error: Outside of Array");
      return;
    }
    Serial.println("Alarm "+ String(i) +": " + logEntries[index].alarmtime);
    sendToNextion("a" + String(i) + ".txt", String("\"" + logEntries[index].alarmtime + "\""));
    Serial.println("Taken "+ String(i) +": " + logEntries[index].timetaken);
    sendToNextion("tk" + String(i) + ".txt", String("\"" + logEntries[index].timetaken + "\""));
  }
}

void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info){
  switch (event) {

    case ARDUINO_EVENT_WIFI_STA_GOT_IP: {
      Serial.println("WiFi connected");
      wifiConnected = true;
      wifiRetryCount = 0;
      setupTime();
      delay(2000);

      sendToNextion("wifi", "1");
      String ack = ""; 
      unsigned long startTime = millis(); 
      int sent = 0; 
      while(ack != "ACK") { 
        if (nextion.available()) { 
          ack = nextion.readStringUntil(';'); 
          ack.trim(); 
          Serial.println("Received: " + ack); 
        } 
        if(millis() - startTime > 10000) { 
          if(sent>3){ 
            break; 
          } 
          sent++; 
          Serial.println("Resending Message"); 
          sendToNextion("wifi", "1"); 
          startTime = millis(); 
        } 
      }
      break;
    }

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED: {

      wifiConnected = false;

      Serial.print("WiFi disconnected. Reason: ");
      Serial.println(info.wifi_sta_disconnected.reason);
      if (!wifiSentDisconnected) {
        sendToNextion("wifi", "0");
        wifiSentDisconnected = true;
      }
      break;
    }

    default:
      break;
  }
}
void setupTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.print("Waiting for NTP time");
  
  time_t now;
  int retry = 0;
  const int maxRetries = 30;
  
  while ((now = time(nullptr)) < 100000 && retry < maxRetries) {
    Serial.print(".");
    delay(1000);
    retry++;
  }
  
  if (retry == maxRetries) {
    Serial.println("Failed to get NTP time!");
  } else {
    Serial.println("");
    Serial.print("NTP time set: ");
    Serial.println(ctime(&now));
  }
}
void sendHeartbeat() {
  if (!wifiConnected) return;
  time_t now = time(nullptr);
  if (now < 100000) {
    Serial.println("Heartbeat Skipped: " + String(now));
    return;
  }
  if (now - lastHeartbeat >= HEARTBEAT_INTERVAL_SEC ) {
    lastHeartbeat = now;

    unsigned long long timestampMillis = (unsigned long long)now * 1000ULL; // convert to milliseconds
    updateServer("/status/heartbeat", String(timestampMillis)); 
    Serial.println("Heartbeat sent: " + String(timestampMillis));

  }
}
void handleWiFiReconnect(){
  if (wifiConnected) return;

  unsigned long now = millis();

  if (wifiRetryCount >= WIFI_MAX_RETRIES) {
    return; // Stop trying
  }

  if (now - lastWifiAttempt >= WIFI_RETRY_INTERVAL) {
    lastWifiAttempt = now;
    wifiRetryCount++;

    Serial.print("WiFi reconnect attempt ");
    Serial.println(wifiRetryCount);

    WiFi.disconnect();
    WiFi.begin(ssid.c_str(), password.c_str());
  }
}
void uploadLogToFirebase(){
  if (!wifiConnected) {
    Serial.println("WiFi offline, skipping Firebase update");
    return;
  }
  
  String path = "/devices/" + deviceId + "/log.json?auth=" + String(FIREBASE_SECRET);

  String payload = "{";
  for (int i = 0; i < 28; i++) {
    payload += "\"" + String(i) + "\":{";
    payload += "\"alarmtime\":\"" + logEntries[i].alarmtime + "\",";
    payload += "\"timetaken\":\"" + logEntries[i].timetaken + "\",";
    payload += "\"delayminutes\":" + String(logEntries[i].delayminutes) + "}";

    if (i < 27) payload += ",";
  }
  payload += "}";

  Serial.print("Uploading log JSON: ");
  Serial.println(payload);

  if (!firebaseClient.connect(FIREBASE_HOST, 443)) {
    Serial.println("Firebase connection FAILED");
    return;
  }

  firebaseClient.println("PUT " + path + " HTTP/1.1");
  firebaseClient.println("Host: " + String(FIREBASE_HOST));
  firebaseClient.println("Content-Type: application/json");
  firebaseClient.print("Content-Length: ");
  firebaseClient.println(payload.length());
  firebaseClient.println();
  firebaseClient.println(payload);

  while (firebaseClient.connected()) {
    String line = firebaseClient.readStringUntil('\n');
    if (line == "\r") break;
  }

  String response = firebaseClient.readString();
  Serial.println("Firebase response:");
  Serial.println(response);

  firebaseClient.stop();
}

void reset(){
  Serial.println("FACTORY RESET STARTED");
  alertActive = false;
  timerRunning = false;
  notified = false;
  alertCancel = false;
  refilled = false;
  doses = 0;

  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);

  a1 = "N/A";
  a2 = "N/A";
  a3 = "N/A";
  a4 = "N/A";

  for (int i = 0; i < 28; i++) {
    logEntries[i].alarmtime = "";
    logEntries[i].timetaken = "";
    logEntries[i].delayminutes = 0;
  }

  if (wifiConnected) {

    String path = "/devices/" + deviceId + ".json?auth=" + String(FIREBASE_SECRET);

    if (!firebaseClient.connect(FIREBASE_HOST, 443)) {
      Serial.println("Firebase connection FAILED (reset)");
      return;
    }

    firebaseClient.println("DELETE " + path + " HTTP/1.1");
    firebaseClient.println("Host: " + String(FIREBASE_HOST));
    firebaseClient.println("Connection: close");
    firebaseClient.println();

    while (firebaseClient.connected()) {
      String line = firebaseClient.readStringUntil('\n');
      if (line == "\r") break;
    }

    String response = firebaseClient.readString();
    Serial.println("Firebase DELETE response:");
    Serial.println(response);

    firebaseClient.stop();
  }
  ssid = "";
  password = "";
  WiFi.disconnect(true);  
  wifiConnected = false;

  sendToNextion("wifi", "0");

  Serial.println("FACTORY RESET COMPLETE");
}
void sync() {

  if (!wifiConnected) {
    Serial.println("SYNC FAILED: No WiFi");
    return;
  }

  Serial.println("Starting SYNC...");

  String path = "/devices/" + deviceId + "/alarms.json?auth=" + String(FIREBASE_SECRET);

  if (!firebaseClient.connect(FIREBASE_HOST, 443)) {
    Serial.println("Firebase connection FAILED (sync)");
    sendToNextion("synced","2");
    return;
  }

  firebaseClient.println("GET " + path + " HTTP/1.1");
  firebaseClient.println("Host: " + String(FIREBASE_HOST));
  firebaseClient.println("Connection: close");
  firebaseClient.println();

  // Skip headers
  while (firebaseClient.connected()) {
    String line = firebaseClient.readStringUntil('\n');
    if (line == "\r") break;
  }

  String payload = firebaseClient.readString();
  firebaseClient.stop();

  if (payload == "null" || payload.length() < 10) {
    Serial.println("No alarm data found.");
    return;
  }

  Serial.println("Payload:");
  Serial.println(payload);

  a1 = extractValue(payload, "\"alarm1\":\"", "\"");
  a2 = extractValue(payload, "\"alarm2\":\"", "\"");
  a3 = extractValue(payload, "\"alarm3\":\"", "\"");
  a4 = extractValue(payload, "\"alarm4\":\"", "\"");

  if (a1 == "") a1 = "N/A";
  if (a2 == "") a2 = "N/A";
  if (a3 == "") a3 = "N/A";
  if (a4 == "") a4 = "N/A";

  Serial.println("SYNC COMPLETE");
  Serial.println(a1);
  Serial.println(a2);
  Serial.println(a3);
  Serial.println(a4);
  
  sendToNextion("synced","1");
}
String extractValue(String data, String startKey, String endChar) {

  int start = data.indexOf(startKey);
  if (start == -1) return "";

  start += startKey.length();
  int end = data.indexOf(endChar, start);
  if (end == -1) return "";

  return data.substring(start, end);
}


