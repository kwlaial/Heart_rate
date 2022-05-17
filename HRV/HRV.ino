const int ecgPin = 0;
int upperThreshold = 1500;
int lowerThreshold = 100;
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
bool start = false;
enum str_conditions { STRESSED , RELAXED };
int menuChoice = 0;

void setup() {
  Serial.begin(115200);
  
}

void(* resetFunc) (void) = 0;//declare reset function at address 0

void phq_func (int rmsdd){
  if (rmsdd >= 0 && rmsdd <= 4) Serial.println("Depression severity: none");
  else if (rmsdd >= 5 && rmsdd <= 9) Serial.println("Depression severity: mild");
  else if (rmsdd >= 10 && rmsdd <= 14) Serial.println("Depression severity: moderate");
  else if (rmsdd >= 15 && rmsdd <= 19) Serial.println("Depression severity: moderately severe");
  else if (rmsdd >= 20 && rmsdd <= 27) Serial.println("Depression severity: severe");  
}

void gad_func (int rmsdd){
  if (rmsdd >= 0 && rmsdd <= 4) Serial.println("Anxiety level: minimal anxiety");
  else if (rmsdd >= 5 && rmsdd <= 9) Serial.println("Anxiety level: mild anxiety");
  else if (rmsdd >= 10 && rmsdd <= 14) Serial.println("Anxiety level: moderate anxiety");
  else if (rmsdd >= 15) Serial.println("Anxiety level: severe anxiety");
}


void loop() { 

  if(start == false) {
    Serial.println("Do you feel (1)positively STRESSED, (2)negatively STRESSED or (3)RELAXED? Please enter the number.");
    while (Serial.available() == 0) {
    }
    menuChoice = Serial.parseInt();
    delay(5000);
    start = true;
  }

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
    if(rrInterval > 500 && rrInterval <1300 &&rrIntervalPrevious > 500 && rrIntervalPrevious <1300){
      rrDiff = float(rrInterval) - float(rrIntervalPrevious);
      rrDiffSquaredTotal = rrDiffSquaredTotal + sq(rrDiff);
      // Count the number of successive differences for averaging
      diffCount = diffCount + 1.0;
      // Reset the hrvUpdate flag
      hrvUpdate = false;
      }
  }

  // Once five minute window has elapsed, calculate the RMSSD
  if (millis() - hrvStartTime >= 60000 && !hrvComplete) {
    rmssd = sqrt(rrDiffSquaredTotal/diffCount);
    hrvComplete = true;
  } 

  //Serial.println(ecgReading);
  //Serial.println(rrInterval);
  /*if (beatsPerMinute >= 60 && beatsPerMinute <= 140){
      float weight = 0.1;                               
      LPF_beat = (1.0 - weight) * LPF_beat + weight * beatsPerMinute;
      Serial.println(LPF_beat);    
  }*/
  if (hrvComplete == true && rmssd < 100) {
    Serial.print("Your HRV reading is: ");
    Serial.println(rmssd);
    float phq_posstr_hrv = -0.07761*rmssd + 14.40413;
    float phq_negstr_hrv = -0.08183*rmssd + 15.12931;
    float phq_rel_hrv = -0.2331*rmssd + 29.3654;
    float gad_posstr_hrv = -0.09276*rmssd + 14.14535;
    float gad_negstr_hrv = -0.09875*rmssd + 15.06781;
    float gad_rel_hrv = -0.1847*rmssd + 24.3324;
  
    switch (menuChoice) {

      case 1:
      phq_func (phq_posstr_hrv);
      gad_func (gad_posstr_hrv);
      break;

      case 2:
      phq_func (phq_negstr_hrv);
      gad_func (gad_negstr_hrv);
      break;

      case 3:
      phq_func (phq_rel_hrv);
      gad_func (gad_rel_hrv);
      break;
    }
    
  }
  else if (hrvComplete == true){
    Serial.println("please try not to move and relax");
    Serial.println("taking reading again");
    delay(5000);
    resetFunc();  //call reset
  }
  delay(10);
}
