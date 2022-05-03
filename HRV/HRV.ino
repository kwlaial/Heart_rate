const int ecgPin = 0;
int upperThreshold = 2200;
int lowerThreshold = 1000;
int ecgOffset = 8000;
float beatsPerMinute = 0.0;
bool alreadyPeaked = false;
unsigned long firstPeakTime = 0;
unsigned long secondPeakTime = 0;
unsigned long rrInterval = 0;
int numRRDetected = 0;
bool hrvStarted = false;
bool hrvUpdate = false;
bool hrvComplete = false;
unsigned long hrvStartTime = 0;
unsigned long rrIntervalPrevious = 0;
float rrDiff = 0.0;
float rrDiffSquaredTotal = 0.0;
float diffCount = 0.0;
float rmssd = -1.0;
float LPF = 0.0;
float LPF_beat = 0.0;

void setup() {
  Serial.begin(115200);
}

void loop() { 
  float weight = 0.1;                               
  LPF = (1.0 - weight) * LPF + weight * (analogRead(ecgPin)*500);
  int ecgReading = LPF + ecgOffset; 

  if (ecgReading > upperThreshold && alreadyPeaked == false) { 
    
      if (firstPeakTime == 0) {
        firstPeakTime = millis();
      }
      else {
        secondPeakTime = millis();
        rrIntervalPrevious = rrInterval;
        rrInterval = secondPeakTime - firstPeakTime;
        firstPeakTime = secondPeakTime;
        hrvUpdate = true;
        numRRDetected = numRRDetected + 1;
      }
      alreadyPeaked = true;
  }

  if (ecgReading < lowerThreshold) {
    alreadyPeaked = false;
  }  

  beatsPerMinute = (1.0/rrInterval) * 60.0 * 1000;
  
  if (!hrvStarted && numRRDetected >= 2) {
    hrvStarted = true;
    hrvStartTime = millis();
  }
  

  if (hrvUpdate && hrvStarted) {
    // Add the square of successive differences between 
    // R-to-R intervals to the running total
    rrDiff = float(rrInterval) - float(rrIntervalPrevious);
    rrDiffSquaredTotal = rrDiffSquaredTotal + sq(rrDiff);
    // Count the number of successive differences for averaging
    diffCount = diffCount + 1.0;
    // Reset the hrvUpdate flag
    hrvUpdate = false;
  }

  // Once five minute window has elapsed, calculate the RMSSD
  if (millis() - hrvStartTime >= 120000 && !hrvComplete) {
    rmssd = sqrt(rrDiffSquaredTotal/diffCount);
    hrvComplete = true;
  } 

  //Serial.println(ecgReading);
  if (beatsPerMinute >= 60 && beatsPerMinute <= 140){
      float weight = 0.1;                               
      LPF_beat = (1.0 - weight) * LPF_beat + weight * beatsPerMinute;
      Serial.println(LPF_beat);    
  }
  if (hrvComplete == true) Serial.println(rmssd); 
  delay(10);
}
