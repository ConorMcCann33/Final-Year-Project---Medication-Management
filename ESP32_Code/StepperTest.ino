
const int stepPin = 5;
const int dirPin = 7;
void setup() {
 // Sets the two pins as Outputs
 pinMode(stepPin,OUTPUT);
 pinMode(dirPin,OUTPUT);
}
void loop() {
  digitalWrite(dirPin,HIGH); // Enables the motor to move in a particular direction
  // Makes 200 pulses for making one full cycle rotation
  for(int x = 0; x < 10; x++) {
    digitalWrite(stepPin,HIGH);
    delay(50);
    digitalWrite(stepPin,LOW);
    delay(50);
  }
  delay(2000); 
}
