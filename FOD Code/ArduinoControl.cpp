const int pwmPin = 9;         
const int envelopePin = A0;   
const int thresholdVoltage = 300; // Change based on real world testing

void setup() {
  Serial.begin(9600);
  pinMode(pwmPin, OUTPUT);

  // Hardware Timer 1 Setup for ~85kHz on Pin 9
  TCCR1A = 0; 
  TCCR1B = 0;
  TCCR1A |= (1 << WGM11); 
  TCCR1B |= (1 << WGM12) | (1 << WGM13);
  TCCR1B |= (1 << CS10); 
  
  ICR1 = 188;   
  OCR1A = 94;   
}

void loop() {
  // Brief 200us pulse to excite the cap for the testing
  TCCR1A |= (1 << COM1A1); 
  delayMicroseconds(200);  
  
  // Cut the signal and read the decay of the cap
  TCCR1A &= ~(1 << COM1A1); 
  digitalWrite(pwmPin, LOW); 
  
  delayMicroseconds(50); // Wait 50us for the decay to start
  int decayVoltage = analogRead(envelopePin);
  
  // FOD Logic
  if (decayVoltage < thresholdVoltage) {  
    // FAIL: Object detected - PWM stays off
    Serial.println("FOD Fail: Object Detected - Power off");
    delay(2000); // Wait 2s before retry
    
  } else {
    // PASS: Pad is clear, Turn on PWM
    Serial.println("FOD Pass: Pad Clear - Power on");
    TCCR1A |= (1 << COM1A1); // Turn 85kHz back on
    
    // Keep power on for 3 seconds
    delay(3000); 
    
    // Turn OFF briefly so the loop can start over and run the ping test again safely
    TCCR1A &= ~(1 << COM1A1); 
    digitalWrite(pwmPin, LOW); 
    delay(10); 
  }
}
