#include <EEPROM.h>
// #include <NewPing.h>
#include <MFRC522.h>

#define NEWPING_TIMER_USE_TIMER2
#define SS_PIN 10  //slave select pin
#define RST_PIN 9  //reset pin

MFRC522 mfrc522(SS_PIN, RST_PIN);  // instatiate a MFRC522 reader object.
MFRC522::MIFARE_Key key;           //create a MIFARE_Key struct named 'key', which will hold the card information

int triggers = 0;
bool activateSecurity = false;
byte readbackblock[18];
int block = 2;
const int buzzer = 2;
const int pir = 7;
const int vibSensor = 4;
// const int ultraTrigger = ;
// const int ultraEcho = ;
const int lock = 8;
// const int ultraMaxDistance = 800;

// const int redBtn = ;
const int blueBtn = 3;

// NewPing sonar(ultraTrigger, ultraEcho, ultraMaxDistance);

void setup() {
    pinMode(buzzer, OUTPUT);
    // pinMode(pir, INPUT);
    // pinMode(vibSensor, INPUT);
    // pinMode(ultraTrigger, OUTPUT);
    // pinMode(ultraEcho, INPUT);
    // pinMode(redBtn, INPUT_PULLUP);
    pinMode(blueBtn, INPUT_PULLUP);
    pinMode(lock, OUTPUT);
    SPI.begin();
    mfrc522.PCD_Init();
    Serial.begin(115200);

    for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;  //keyByte is defined in the "MIFARE_Key" 'struct' definition in the .h file of the library
    }
    // sonar.setup();
}

void loop() {
    // Look for new cards
  String cardID = checkForCards();
  Serial.print(cardID);
  
  bool btnState = digitalRead(blueBtn);
  if (btnState == LOW) {
    delay(500);
    btnState = digitalRead(blueBtn);
    if (btnState == LOW) {
      ping(2);
      initRFID();
    } else {
      if (activateSecurity == false) {
        ping(1);
        activateSecurity = true;
      } else {
        lowPing(1);
        activateSecurity = false;
      }
    }
  }

  if (activateSecurity){
    if (checkPIR()){
      triggers++;
      Serial.println("PIR trigger" + checkPIR());
    }
    if (checkVibration()){
      triggers++;
      Serial.println("Vibration trigger" + checkVibration());
    }
    if (triggers >= 2) {
      setOffAlarm();
    }
    triggers = 0;
  }


  if (verifyCard(cardID)) {
    Serial.println("Card verified");
    unlockDoor();
    playSuccessDing();
    delay(10000);
    lockDoor();
    cardID = "";
  }

}


void initRFID() {
  String cardID = checkForCards();
  Serial.print("Please init card");
  while (cardID == "") {
    cardID = checkForCards();
  }
  for (int i = 1; i < 17; i++) {
      EEPROM.write(i, cardID[i-1]);
  }
}


String checkForCards() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return "";
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return "";
  }

  readBlock(2, readbackblock);
  String cardID = "";
  for (int j = 0; j < 16; j++) {
    cardID += (char)readbackblock[j];
  }
  
  // Halt PICC and stop crypto
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  
  return cardID;
}
void resetCardReader() {
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

//Read specific block
int readBlock(int blockNumber, byte arrayAddress[]) {
  int largestModulo4Number = blockNumber / 4 * 4;
  int trailerBlock = largestModulo4Number + 3;

  // Authentication with retry logic
  byte status;
  int retryCount = 0;
  const int maxRetries = 3;
  
  do {
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.print("PCD_Authenticate() failed (read): ");
      Serial.println(mfrc522.GetStatusCodeName(status));
      retryCount++;
      if(retryCount < maxRetries) {
        delay(100); // Short delay between retries
      }
    }
  } while (status != MFRC522::STATUS_OK && retryCount < maxRetries);
  
  if (status != MFRC522::STATUS_OK) {
    return 3;
  }

  // Reading the block
  byte buffersize = 18;
  status = mfrc522.MIFARE_Read(blockNumber, arrayAddress, &buffersize);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("MIFARE_read() failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 4;
  }

  // Print block contents
  Serial.print("Block ");
  Serial.print(blockNumber);
  Serial.print(" contents (HEX): ");
  for (int i = 0; i < 16; i++) {
    if(arrayAddress[i] < 0x10) Serial.print("0");
    Serial.print(arrayAddress[i], HEX);
    Serial.print(" ");
  }
  
  Serial.print("\nASCII: ");
  for (int i = 0; i < 16; i++) {
    if(arrayAddress[i] >= 32 && arrayAddress[i] <= 126) {
      Serial.write(arrayAddress[i]);
    } else {
      Serial.print("."); // Print dot for non-printable chars
    }
  }
  Serial.println();
  
  Serial.println("block was read");
  return 0;
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

bool verifyCard(String ID) {
  resetCardReader();
  if(ID.length() == 0) {
    return false;
  }
  
  String storedID = "";
  
  // Read from EEPROM and trim whitespace
  for (int i = 1; i < 17; i++) {
    char c = (char)EEPROM.read(i);
    if(c != 0 && c != ' ') { // Skip null bytes and spaces
      storedID += c;
    }
  }
  
  // Trim input ID
  ID.trim();
  
  return storedID.equals(ID); // Use equals() for exact comparison
}

// void resetUltraSound(){
//     EEPROM.write(0, sonar.ping());
// }

// bool checkUltraSound(){
//     byte default = EEPROM.read(0);
//     if(default == 0){
//         resetUltraSound();
//     }
//     else{
//         if(sonar.ping() > default + 100 || sonar.ping() < default - 50){
//             return true;
//         } 
//     }
//     return false;
// }

// 播放成功提示音，使用 2kHz 音调，持续 200ms
void playSuccessDing() {
    tone(buzzer, 2000);
    delay(200);
    noTone(buzzer);
}



void setOffAlarm(){
    // 3.5kHz tone with 1Hz pulsing pattern
    tone(buzzer, 3500);
    delay(200);
    tone(buzzer, 2000);
    delay(200);    
    tone(buzzer, 3500);
    delay(200);
    tone(buzzer, 2000);
    delay(200);    
    tone(buzzer, 3500);
    delay(200);
    tone(buzzer, 2000);
    delay(200);
    noTone(buzzer);
}

void ping(int times) {
  for (int i = 0; i < times; i++) {
    tone(buzzer, 2500);
    delay(100);
    noTone(buzzer);
    delay(200);
  }
}

void lowPing(int times){
  for (int i = 0; i < times; i++) {
    tone(buzzer, 1500);
    delay(100);
    noTone(buzzer);
    delay(200);
  }
}