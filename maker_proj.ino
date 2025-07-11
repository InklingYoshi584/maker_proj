#include <EEPROM.h>
#include <NewPing.h>
const int buzzer = ;
const int pir = ;
const int vibSensor = ;
const int ultraTrigger = ;
const int ultraEcho = ;
const int lock = ;
const int ultraMaxDistance = 800;

const int redBtn = ;
const int blueBtn = ;

NewPing sonar(ultraTrigger, ultraEcho, ultraMaxDistance);

void setup() {
    pinMode(buzzer, OUTPUT);
    pinMode(pir, INPUT);
    pinMode(vibSensor, INPUT);
    pinMode(ultraTrigger, OUTPUT);
    pinMode(ultraEcho, INPUT);
    pinMode(redBtn, INPUT_PULLUP);
    pinMode(blueBtn, INPUT_PULLUP);
    pinMode(lock, OUTPUT);
    Serial.begin(115200);
    sonar.setup();
}

void loop() {
    redBtnState = digitalRead(redBtn);
    blueBtnState = digitalRead(blueBtn);
    if(redBtnState == HIGH){
        lockDoor();
    }
    else if(blueBtnState == HIGH){
        unlockDoor();
    }
}

void lockDoor() {
    digitalWrite(lock, LOW);
}

void unlockDoor() {
    digitalWrite(lock, HIGH);
}

bool checkPIR() {
    return digitalRead(pir) == HIGH;
}

bool checkVibration(){
    return digitalRead(vibSensor) == HIGH;
}

void resetUltraSound(){
    EEPROM.write(0, sonar.ping());
}

bool checkUltraSound(){
    byte default = EEPROM.read(0);
    if(default == 0){
        resetUltraSound();
    }
    else{
        if(sonar.ping() > default + 100 || sonar.ping() < default - 50){
            return true;
        } 
    }
    return false;
}

void setOffAlarm(){
    // 3.5kHz tone with 1Hz pulsing pattern
    tone(buzzer, 3500);
    delay(500);
    noTone(buzzer);
    delay(500);
}