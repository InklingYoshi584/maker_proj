const int buzzer = ;
const int pir = ;
const int vibSensor = ;


void setup() {

}

void loop() {

}

bool checkPIR() {
    return digitalRead(pir) == HIGH;
}

bool checkVibration(){
    return digitalRead(vibSensor) == HIGH;
}

bool checkUltraSound(){
    
}

void setOffAlarm(){
    // 3.5kHz tone with 1Hz pulsing pattern
    tone(buzzer, 3500);
    delay(500);
    noTone(buzzer);
    delay(500);
}