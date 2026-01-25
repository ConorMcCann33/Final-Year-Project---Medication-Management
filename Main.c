/*
  Server functions (connect, Get and Post)
  See if refill function is required
  */
  //Packages
  #include <Stepper.h>
  #include <WiFi.h>

  //ESP Pin Nums
  const int stepPin = 5;
  const int dirPin = 7;
  const int ledPin = 11;
  const int buzzerPin = 12;

  //Global Variables
  int mode = 3; // Mode 0 is Live, Mode 1 is test, Mode 2 is Demo Mode
  bool LED = 0; // Used to flash LED 
  bool testAlert = false; // Flag for when test alert is used

  String a1 = ""; // String for Alarm 1
  String a2 = ""; // String for Alarm 2
  String a3 = ""; // String for Alarm 3
  String a4 = ""; // String for Alarm 4

  //UART
  HardwareSerial nextion(1);  // Sets Nextion UART to channel 1
  String incomingMessage = ""; // String for UART Message storage

  //Timer
  bool alertActive = false; // Check that alert is active
  bool timerRunning = false; // Check that alert timer is counting down
  bool notified = false; // Check that the notification has been sent
  bool modeSelect = false; // Check that the mode has been selected
  bool wifiSetup = false; // Check that wifi has been set up

  unsigned long alertStartTime = 0;
  unsigned long alertTimeout = 0;

  String ssid = "";
  String password = "";

void setup() { // Sets serial monitor baud rate
  Serial.begin(9600);
  Serial.println("ESP32 starting...");
  nextion.begin(9600, SERIAL_8N1 , 2, 1); // Initialise UART1 for Nextion
  Serial.println("Nextion UART ready");

  //Sets pin inputs and outputs
  pinMode(stepPin,OUTPUT);
  pinMode(dirPin,OUTPUT);
  pinMode(ledPin,OUTPUT);
  pinMode(buzzerPin,OUTPUT);

  digitalWrite(dirPin,LOW);
  //Timer
  initialisation();
}
void loop(){
  receiver();
}
void initialisation(){
  while(modeSelect == false)
  {
  receiver();
  }
  switch (mode){
  case 0:
    Serial.println("Live mode enabled");
    break;
  
  case 1:
    Serial.println("Test mode enabled");
    break;

  case 2:
    Serial.println("Demo mode enabled");
    break;
    
  default:
    initialisation();
    break;
  
  }
}
void alert(){
  startAlertTimer(1);
  while(alertActive == true){
    receiver();
    handleAlert();
  }
  cancelAlert();
  if(testAlert == true) return;
  motor(10);
}
void notify(){
  Serial.println("Send Notification");
  //Tells app to send notification
  //Sends whether it was taken or not and how long it took to take it
  //Cancels the buzzer
  
}
void refill(){
  //28 motors
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

    Serial.print("Received: ");
    Serial.println(msg);

    //Example:T,8,0;
    if (msg.startsWith("T,")) {
      alertActive = false;
      int firstComma = msg.indexOf(',');
      int secondComma = msg.indexOf(',', firstComma + 1);

      int hour = msg.substring(firstComma + 1, secondComma).toInt();
      int minute = msg.substring(secondComma + 1).toInt();

      Serial.print("Hour: ");
      Serial.println(hour);
      Serial.print("Minute: ");
      Serial.println(minute);
    }
    //Example:A,1;
    if (msg.startsWith("A,")) {
      int firstComma = msg.indexOf(',');

      int alarm = msg.substring(firstComma + 1).toInt();
    
      if (alarm<5){
        testAlert = false;
        Serial.print("Alarm: ");
        Serial.println(alarm);
        alert();
      }
      if (alarm==5){
        testAlert = true;
        Serial.print("Test Alert");
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
      modeSelect = true;
    }
    //Example:M,1
    if (msg.startsWith("M,")) {
      int firstComma = msg.indexOf(',');
      int steps = msg.substring(firstComma + 1).toInt();
      motor(steps);
    }
    //Example:T,8,0;
    if (msg.startsWith("W,")) {
      int firstComma = msg.indexOf(',');
      int secondComma = msg.indexOf(',', firstComma + 1);

      ssid = msg.substring(firstComma + 1, secondComma);
      password = msg.substring(secondComma + 1);

      Serial.print("SSID: ");
      Serial.println(ssid);
      Serial.print("Password: ");
      Serial.println(password);

      connectToWiFi();
    }
    //sendToNextionText("t0.txt=/","ACK");
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
      break;

    default:
      Serial.print("Error: Alarm Number not found");
      break;
    
  }
}
void sendToNextionText(String loc, String text) {
  Serial.println("Sending Message");
  nextion.print(loc);
  nextion.print("=\"");
  nextion.print(text);
  nextion.print("\"");
  nextion.write(0xFF);
  nextion.write(0xFF);
  nextion.write(0xFF);
  Serial.println("Message Sent");
}
void sendToNextionNumber(String loc, int value) {
  nextion.print(loc);
  nextion.print(value);
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
  // Timer expired?
  if (timerRunning && !notified) {
    alarm();
    if (millis() - alertStartTime >= alertTimeout) {
      notify();          // Notify ONCE
      notified = true;
      cancelAlert();     // Cancel alert
    }
  }
}
void cancelAlert() {
  timerRunning = false;

  // Turn off outputs here
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);

  Serial.println("Alarm cancelled");
}
void connectToWiFi() {
    Serial.print("Connecting to network: ");
    Serial.println(ssid);
    Serial.flush();
    WiFi.begin(ssid, password);
    int i = 0;
    while(WiFi.status() != WL_CONNECTED){
      if(i>100){
        Serial.println();
        Serial.println("Failed to Connect");
        sendToNextionText("t0.txt", "Failed to Connect"); 
        return;
      }
      i++;
      Serial.print(i);
      Serial.print(",");
      Serial.flush();
      delay(300);
    }
    Serial.println();
    Serial.println("Connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    sendToNextionText("t0.txt", "Connected");
    wifiSetup = true; 
}
