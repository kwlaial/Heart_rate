int upperThreshold = 2000;
int lowerThreshold = 800;
int ecgOffset = 8000;
float beatsPerMinute = 0.0;
bool alreadyPeaked = false;
unsigned long firstPeakTime = 0;
unsigned long secondPeakTime = 0;
unsigned long rrInterval = 0;
float LPF = 0.0;
float LPF_beat = 0.0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() { 
  int ecgReading = analogRead(ecgPin)*500 + ecgOffset; 
  float weight = 0.1;                               
  LPF = (1.0 - weight) * LPF + weight * ecgReading;
  
  if (LPF > upperThreshold && alreadyPeaked == false) { 
  // Check if the ECG reading is above the upper threshold and that
  // we aren't already in an existing peak
      if (firstPeakTime == 0) {
        // If this is the very first peak, set the first peak time
        firstPeakTime = millis();
        digitalWrite(LED_BUILTIN, HIGH);
      }
      else {
        // Otherwise set the second peak time and calculate the 
        // R-to-R interval. Once calculated we shift the second
        // peak to become our first peak and start the process
        // again
        secondPeakTime = millis();
        rrInterval = secondPeakTime - firstPeakTime;
        firstPeakTime = secondPeakTime;
        digitalWrite(LED_BUILTIN, HIGH);
      }
      alreadyPeaked = true;
  }

  if (LPF < lowerThreshold) {
  // Check if the ECG reading has fallen below the lower threshold
  // and if we are ready to detect another peak
    alreadyPeaked = false;
    digitalWrite(LED_BUILTIN, LOW);
  }  

  beatsPerMinute = (1.0/rrInterval) * 60.0 * 1000;


  // Print the final values to be read by the serial plotter
  //Serial.print("ECG: ");
  //Serial.println(LPF);
  //Serial.print(",");
  if (beatsPerMinute >= 60 && beatsPerMinute <= 140){
      float weight = 0.1;                               
      LPF_beat = (1.0 - weight) * LPF_beat + weight * beatsPerMinute;
    Serial.print("BPM: ");         
    Serial.println(LPF_beat);    
  }
    
  delay(5);
}
