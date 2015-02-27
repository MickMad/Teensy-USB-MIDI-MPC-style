/*

Teensy 3.0 USB MIDI controller

Michele Perla 2014

*/
#define STATE_OFF 0
#define STATE_ON 1
#define MUX_D 0
#define MUX_C 1
#define MUX_B 2
#define MUX_INH1 3
#define MUX_INH3 4
#define MUX_INH2 5
#define MUX_A 8
#define MUX_IN2 A2
#define MUX_IN3 A3
#define MUX_IN1 A6
#define MUX_IN4 A7
const int analogInPin = A6;
const int ledStatusPin = 13;
const int startNote = 48;
int state[16] = { STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, };
int topValue = 0;
int lowValue = 125;
int sensorValue[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int sensorMap[16] = { 0, 8, 1, 9, 14, 6, 15, 7, 3, 11, 2, 10, 13, 5, 12, 4 };
int sensorValue2[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int state2[16] = { STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, STATE_OFF, };
int fixedVelocity = 1;
int currentScale = 0;
int fixedVelocityValue = 127;
int startNoteValue = 0;
int startOctaveValue = 0;
int scales[8][16] = {
  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
  { 0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19, 21, 23, 24, 26 },
  { 0, 2, 3, 5, 7, 9, 10, 12, 14, 15, 17, 19, 21, 22, 24, 26 },
  { 0, 1, 3, 5, 7, 8, 10, 12, 13, 15, 17, 19, 20, 22, 24, 25 },
  { 0, 2, 4, 6, 7, 9, 11, 12, 14, 16, 18, 19, 21, 23, 24, 26 },
  { 0, 2, 4, 5, 7, 9, 10, 12, 14, 16, 17, 19, 21, 22, 24, 26 },
  { 0, 2, 3, 5, 7, 8, 10, 12, 14, 15, 17, 19, 20, 22, 24, 26 },
  { 0, 1, 3, 5, 6, 8, 10, 12, 13, 15, 17, 18, 20, 22, 24, 25 }
};

IntervalTimer theTimer;
int i = 0;


void setup() {
  delay(5000);
  int c=0;
  Serial.println("Initializing Top Value. Press button 0 with maximum force.");
  pinMode(ledStatusPin,OUTPUT);
  pinMode(MUX_INH1,OUTPUT);
  pinMode(MUX_A,OUTPUT);
  pinMode(MUX_B,OUTPUT);
  pinMode(MUX_C,OUTPUT);
  pinMode(MUX_D,OUTPUT);
  pinMode(MUX_INH2,OUTPUT);
  digitalWrite(MUX_INH1,HIGH);
  digitalWrite(MUX_INH2,HIGH);
  digitalWrite(MUX_A,LOW);
  digitalWrite(MUX_B,LOW);
  digitalWrite(MUX_C,LOW);
  digitalWrite(MUX_D,LOW);
  digitalWrite(MUX_INH1,LOW);
  digitalWrite(ledStatusPin,HIGH);
  while (c++<1500){
    sensorValue[0] = analogRead(MUX_IN1);
    if (sensorValue[0]>topValue) topValue=sensorValue[0];
    delay(2);
  }
  digitalWrite(ledStatusPin,LOW);
  Serial.print("Top Value set to ");
  Serial.print(topValue);
  Serial.println(" . Please release button to continue.");
  sensorValue[0]=0;
  while (analogRead(MUX_IN1)>10) delay(10);
  Serial.println("Thank you. Booting up...");
  digitalWrite(ledStatusPin,HIGH);
  delay(100);
  digitalWrite(ledStatusPin,LOW);
  delay(100);
  digitalWrite(ledStatusPin,HIGH);
  delay(100);
  digitalWrite(ledStatusPin,LOW);
  delay(100);
  digitalWrite(ledStatusPin,HIGH);
  delay(100);
  digitalWrite(ledStatusPin,LOW);
  delay(100);
  //theTimer.begin(timerISR,31.25f);
  theTimer.begin(timerISR,31.25f);
}

void timerISR(){
  if (i==32) i=0;
  int tempValue;
  if (i<16){
     //READ FSRs STATUS
    digitalWrite(MUX_INH1,HIGH);
    digitalWrite(MUX_INH2,HIGH);
    digitalWrite(MUX_A,i&1);
    digitalWrite(MUX_B,i&2);
    digitalWrite(MUX_C,i&4);
    digitalWrite(MUX_D,i&8);
    digitalWrite(MUX_INH1,LOW);
    tempValue = analogRead(MUX_IN1);
    int realFSR = sensorMap[i];
    int tempNote = 36 + (startOctaveValue*12) + startNoteValue + scales[currentScale][realFSR];
    switch (state[realFSR]){
      case STATE_OFF:
        if (tempValue>lowValue){
          sensorValue[realFSR] = tempValue;
          int tempVel = map(tempValue,lowValue,topValue,1,127);
          state[realFSR] = STATE_ON;          
          //Serial.print("index ");
          //Serial.print(i);
          Serial.print("Note ");
          Serial.print(tempNote);
          Serial.print(" on, velocity ");
          if (fixedVelocity) {
            usbMIDI.sendNoteOn(tempNote, fixedVelocityValue, 0);  // 60 = C4
            Serial.print(fixedVelocityValue);
          }else{
            usbMIDI.sendNoteOn(tempNote, tempVel, 0);
            Serial.print(tempVel);
          }
          Serial.print(", sensor ");
          Serial.print(realFSR);
          Serial.print(" value is ");
          Serial.println(sensorValue[realFSR]);
        }
        break;
      case STATE_ON:
        if (tempValue<10){
          state[realFSR] = STATE_OFF;
          usbMIDI.sendNoteOn(tempNote, 0, 0);  // 60 = C4
          sensorValue[realFSR]=0;
        }
        break;
    }
  }else{
    //READ SECONDARY BOARD STATUS
    int k = i-16;
    digitalWrite(MUX_INH1,HIGH);
    digitalWrite(MUX_INH2,HIGH);
    digitalWrite(MUX_A,i&1);
    digitalWrite(MUX_B,i&2);
    digitalWrite(MUX_C,i&4);
    digitalWrite(MUX_D,i&8);
    digitalWrite(MUX_INH2,LOW);
    tempValue = analogRead(MUX_IN2);
    if (k<5){ //AVOID READING NOT PRESENT INPUTS
      if (k<2){ // READ BUTTONS and treat them accordingly
        switch (state2[k]){ 
          case STATE_OFF:
            if (tempValue>100){
              resetNotes();
              sensorValue2[k] = tempValue;
              state2[k] = STATE_ON;
              if (k==0){
                fixedVelocity=!fixedVelocity;
                Serial.print("Fixed velocity ");
                if (fixedVelocity) Serial.println("enabled");
                else Serial.println("disabled");
              }
              if (k==1){
                currentScale++;
                if (currentScale>7) currentScale=0;
                Serial.print("Current scale = ");
                Serial.print(currentScale);
                if (currentScale == 0) Serial.println(", chromatic.");
                else if (currentScale == 1) Serial.println(", Ionian.");
                else if (currentScale == 2) Serial.println(", Dorian.");
                else if (currentScale == 3) Serial.println(", Phrygian.");
                else if (currentScale == 4) Serial.println(", Lydian.");
                else if (currentScale == 5) Serial.println(", Mixolydian.");
                else if (currentScale == 6) Serial.println(", Aeolian.");
                else if (currentScale == 7) Serial.println(", Locrian.");
              }
            }
          break;
          case STATE_ON:
            if (tempValue<100){
              state2[k] = STATE_OFF;
            }
          break;
        }
      }else{ //READ POT VALUES and treat them accordingly
        if (k==2){
          if (tempValue>1000) tempValue=1000;
          tempValue = map(tempValue,0,1000,1,127);
          if (fixedVelocityValue!=tempValue) fixedVelocityValue = tempValue;
        }else if (k==3){
          tempValue = map(tempValue,0,1000,0,11);
          if (startNoteValue!=tempValue){
            resetNotes();
            startNoteValue = tempValue;
            Serial.print("Start from note ");
            switch (startNoteValue){
              case 0:
                Serial.println("C");
                break;
              case 1:
                Serial.println("C# / Db");
                break;
              case 2:
                Serial.println("D");
                break;
              case 3:
                Serial.println("D# / Eb");
                break;
              case 4:
                Serial.println("E");
                break;
              case 5:
                Serial.println("F");
                break;
              case 6:
                Serial.println("F# / Gb");
                break;
              case 7:
                Serial.println("G");
                break;       
              case 8:
                Serial.println("G# / Ab");
                break;
              case 9:
                Serial.println("A");
                break;        
              case 10:
                Serial.println("A# / Bb");
                break;
              case 11:
                Serial.println("B");
                break;
            }
          }
        }else if (k==4){
          tempValue = map(tempValue,0,1000,0,4);
          if (startOctaveValue!=tempValue){
            resetNotes();
            startOctaveValue = tempValue;
            Serial.print("Start from octave ");
            Serial.println(startOctaveValue);
          }
        }
      }
    }
  }
  i++;
}

void resetNotes(){
  Serial.println("Resetting notes.");
  for (int l=0;l<15;l++){
    // RESET ALL NOTES
    usbMIDI.sendNoteOn(12 + (startOctaveValue*12) + startNoteValue + scales[currentScale][l], 0 ,0);
  }
}

void loop() {
}
