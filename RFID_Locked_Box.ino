// Andrew C Mattson
// CS F241 Final Project Code
// RFID Locked Box
// Edited 4/26/2023

#include <Servo.h>

// RFID Reader Requirements
#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN); // Card reading object
/* RFID datasheet: http://www.handsontec.com/dataspecs/RC522.pdf
 * SDA --> Digital 10
 * SCK --> Digital 13
 * MOSI -> Digital 11
 * MISO -> Digital 12
 * IRQ --> unconnected
 * GND --> GND
 * RST --> Digital 9
 * 3.3V -> 3.3V
*/

Servo srv;      // The Locking Servo
int pinSrv = 6; // Servo Signal
int pinGrn = 3; // Green LED
int pinRed = 2; // Red LED
int pinSpk = 4; // Speaker Signal
int pinBtn = 8; // Button to lock the box when shut

char* tagUID = "D3 93 22 11"; // Authorized RFID
int digitalWriteUS = 4; 

// Lawlor function to play a given frequency for a given time.
void playNote(int frequencyHz,int durationMs) {
  int cycleTimeUS=long(1000000)/frequencyHz;
  int delayTimeUS=cycleTimeUS/2-digitalWriteUS;
  digitalWrite(pinSpk,LOW);
  bool bounce=true;
  int cycles=2*(durationMs*long(1000))/cycleTimeUS;
  for (int i=0;i<cycles;i++) {
    digitalWrite(pinSpk,bounce); 
    bounce=!bounce;
    delayMicroseconds(delayTimeUS);
  }
  digitalWrite(pinSpk,LOW);
}

// Function to play a happy sound!
void playSuccess() {
  int G = 786/2;
  int B = 493;
  int D = 588;
  int C = 525;

  playNote(G, 150);
  playNote(B, 150);
  playNote(D, 150);
  playNote(C, 150);
}

// Function to play an angry sound.
void playFail() {
  int C = 525/4;
  int G = 786/8;
  playNote(C, 150);
  playNote(G, 150);  
}

// Function to call when RFID tag has been authorized.
// Opens the box, lights up green, sounds happy.
void RFIDauthorized() {
  digitalWrite(pinGrn, HIGH); // Light up green
  playSuccess(); // Sound happy
  delay(200); 
  srv.write(0); // Unlock the box
  delay(3000); // Wait three seconds
  digitalWrite(pinGrn, LOW); // Turn off the light
  while (digitalRead(pinBtn) != 0); // Wait for the box to close
  srv.write(90); // Lock the box
}

// Function to call when RFID tag has NOT been authorized.
// Lights up red, sounds angry.
void RFIDdenied() {
  digitalWrite(pinRed, HIGH); // Light up red
  playFail(); // Sound angry
  delay(700); // Wait (to prevent spamming RFID codes)
  digitalWrite(pinRed, LOW); // Turn off the light
}

void setup() {
	Serial.begin(9600);   // Initiate a serial communication

  // RFID Reader Setup
  SPI.begin(); // Initiate SPI bus
  mfrc522.PCD_Init(); // Initiate MFRC522
  Serial.println("Scan your card...");
  Serial.println(); 

  // Set up LEDs
  pinMode(pinGrn, OUTPUT);
  pinMode(pinRed, OUTPUT);
  digitalWrite(pinGrn, LOW);
  digitalWrite(pinRed, LOW);

  // Set up the speaker
  pinMode(pinSpk, OUTPUT);
  digitalWrite(pinSpk, LOW);

  // Set up the servo
  srv.attach(pinSrv);
  srv.write(90); // Lock the box on startup

  // Set up the close-when-shut button
  pinMode(pinBtn, INPUT_PULLUP);
}

void loop() {
	 // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent())
    return;

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
    return;

  // Save UID and show on serial monitor
  Serial.print("UID tag :");
  String content= "";
  for (byte i = 0; i < mfrc522.uid.size; i++) { // Read only the UID bytes
    // Print out each byte
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    // Concatenate each byte to a string
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase(); // Make UID uppercase
  Serial.println();
  
  // Check if UID matches authorized card.
  //   I had no need to, but an entire static const list of authorized
  //   UIDs could be looped through if multiple UIDs need access.
  if (content.substring(1) == tagUID) { // UID match; open box, be happy.
    Serial.println("Authorized access");
    RFIDauthorized();
  }
 else { // UID mismatch; be angry.
    Serial.println("Access denied");
    RFIDdenied();
  }
}