#include <EEPROM.h>
#include <NewPing.h>
#include <MFRC522.h>

#define SS_PIN 10  //slave select pin
#define RST_PIN 9  //reset pin

MFRC522 mfrc522(SS_PIN, RST_PIN);  // instatiate a MFRC522 reader object.
MFRC522::MIFARE_Key key;           //create a MIFARE_Key struct named 'key', which will hold the card information

byte readbackblock[18];
int block = 2;
// const int buzzer = ;
// const int pir = ;
// const int vibSensor = ;
// const int ultraTrigger = ;
// const int ultraEcho = ;
// const int lock = ;
// const int ultraMaxDistance = 800;

// const int redBtn = ;
// const int blueBtn = ;

// NewPing sonar(ultraTrigger, ultraEcho, ultraMaxDistance);

void setup() {
    // pinMode(buzzer, OUTPUT);
    // pinMode(pir, INPUT);
    // pinMode(vibSensor, INPUT);
    // pinMode(ultraTrigger, OUTPUT);
    // pinMode(ultraEcho, INPUT);
    // pinMode(redBtn, INPUT_PULLUP);
    // pinMode(blueBtn, INPUT_PULLUP);
    // pinMode(lock, OUTPUT);
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
  Serial.write(cardID);
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
    SPI.begin();
    mfrc522.PCD_Init();
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


// void lockDoor() {
//     digitalWrite(lock, LOW);
// }

// void unlockDoor() {
//     digitalWrite(lock, HIGH);
// }

// bool checkPIR() {
//     return digitalRead(pir) == HIGH;
// }

// bool checkVibration(){
//     return digitalRead(vibSensor) == HIGH;
// }

bool verifyCard(){
    if(!mfrc522.PICC_IsNewCardPresent()){
        return false;
    }
    if(!mfrc522.PICC_ReadCardSerial()){
        return false;
    }
    return true;
}

byte* readCard(){
    static byte readbackblock[18];
    
    if(!verifyCard()){
        return nullptr;
    }
    int block = 2;
    
    // Determine trailer block for the sector
    int largestModulo4Number = block / 4 * 4;
    int trailerBlock = largestModulo4Number + 3;

    // Authenticate the desired block for access
    byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    
    if (status != MFRC522::STATUS_OK) {
        Serial.print("PCD_Authenticate() failed (read): ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return nullptr;
    }

    // Reading the block
    byte buffersize = 18;
    status = mfrc522.MIFARE_Read(block, readbackblock, &buffersize);
    if (status != MFRC522::STATUS_OK) {
        Serial.print("MIFARE_read() failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return nullptr;
    }
    Serial.println("block was read");
    
    return readbackblock;
}

void write16ByteBlockToEEPROM(int address, byte data[16]) {
    for (int i = 0; i < 16; i++) {
        EEPROM.write(address + i, data[i]);
    }
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

// void setOffAlarm(){
//     // 3.5kHz tone with 1Hz pulsing pattern
//     tone(buzzer, 3500);
//     delay(500);
//     noTone(buzzer);
//     delay(500);
// }