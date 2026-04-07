// --- PIN DEFINITIONS ---
// Analog Sensing Pins
const int VSENSE_PIN = A0;   
const int ISENSE_PIN = A1;   
const int QFACTOR_PIN = A2;  

// Gate Driver Control Pins
const int VPULSE_PIN = 9;    
const int VSD_PIN = 8;       

// Interlock Pins (Bridge these two together to enable the system)
const int INTERLOCK_SENSE = 4; // The pin that detects the bridge
const int INTERLOCK_GND = 5;   // The pin acting as a localized ground

// --- VARIABLES ---
int vsenseValue = 0;
int isenseValue = 0;
int qfactorValue = 0;
bool faultDetected = false;    // Safety latch to permanently lock the system on fault

void setup() {
  Serial.begin(115200);
  
  // 1. Configure the Interlock Pins
  pinMode(INTERLOCK_GND, OUTPUT);
  digitalWrite(INTERLOCK_GND, LOW); // Forces Pin 5 to act as Ground (0V)
  
  pinMode(INTERLOCK_SENSE, INPUT_PULLUP); // Holds Pin 4 at 5V internally
  
  // 2. Configure Gate Driver Pins
  pinMode(VSD_PIN, OUTPUT);
  pinMode(VPULSE_PIN, OUTPUT);
  digitalWrite(VSD_PIN, LOW); // Start with Gate Driver safely OFF

  // 3. Configure Hardware Timer1 for 85kHz Fast PWM
  TCCR1A = 0; 
  TCCR1B = 0;
  TCCR1A |= (1 << COM1A1) | (1 << WGM11);
  TCCR1B |= (1 << WGM13) | (1 << WGM12);
  ICR1 = 187; 
  OCR1A = 94; 
  TCCR1B |= (1 << CS10);
  
  Serial.println("System Initialized. Awaiting Interlock Bridge (Pin 4 to Pin 5).");
}

void loop() {
  // --- SAFETY LATCH CHECK ---
  // If a fault was previously detected, halt all operations permanently.
  if (faultDetected) {
    digitalWrite(VSD_PIN, LOW); // Ensure driver stays dead
    return; // Skips the rest of the loop entirely
  }

  // --- HARDWARE INTERLOCK LOGIC ---
  // If Pin 4 is bridged to Pin 5, it will read LOW.
  if (digitalRead(INTERLOCK_SENSE) == LOW) {
    digitalWrite(VSD_PIN, HIGH); // WAKE UP IR2104
  } else {
    digitalWrite(VSD_PIN, LOW);  // KILL IR2104 (Pins are unbridged)
  }

  // --- SENSOR READING ---
  vsenseValue = analogRead(VSENSE_PIN);
  isenseValue = analogRead(ISENSE_PIN);
  qfactorValue = analogRead(QFACTOR_PIN);

  // --- FOD FAULT DETECTION ---
  // Only check for faults if the system is actively running
  if (digitalRead(VSD_PIN) == HIGH) {
    if (vsenseValue > 800 || isenseValue > 800) {
      
      // TRIGGER THE FAULT
      digitalWrite(VSD_PIN, LOW); 
      faultDetected = true; // Trip the safety latch
      
      Serial.println("=========================================");
      Serial.println("FOD FAULT: Metal object detected!");
      Serial.println("Transmitter DISABLED. Hard reset required.");
      Serial.println("=========================================");
    }
  }

  // --- SERIAL DEBUGGING ---
  Serial.print("Interlock: "); 
  Serial.print(digitalRead(INTERLOCK_SENSE) == LOW ? "BRIDGED " : "OPEN    ");
  Serial.print("| V:"); Serial.print(vsenseValue);
  Serial.print("\tI:"); Serial.print(isenseValue);
  Serial.print("\tQ:"); Serial.println(qfactorValue);
  
  delay(10);
}
