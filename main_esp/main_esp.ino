#include <WiFi.h>
#include <string.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_Fingerprint.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//OLED
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


//PRINT SENSOR
#define MODEM_RX 16
#define MODEM_TX 17
#define mySerial Serial2 // use for ESP32
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);



//RFID
#define SS_PIN 21
#define RST_PIN 2
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key; 
// Init array that will store new NUID 
byte nuidPICC[4];


//WiFi AND SOCKET
WiFiClient client;
const char* ssid = "sha-de";
const char* password =  "shadeairtel123";
const uint16_t port = 5059;
const char * host = "192.168.1.145";

//HANDLE JSON RESPONSE
DynamicJsonDocument doc(1024);

//PINS
int register_button = 13;
int attendance_button = 12;


void show(String Line1, String Line2, String Line3)
{
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0,5);             // Start at top-left corner
  display.println(Line1);
  display.setCursor(0,25);             // Start at top-left corner
  display.println(Line2);
  display.setCursor(0,45);             // Start at top-left corner
  display.println(Line3);
  display.display();
}



void setup()
{
  //PINS
  pinMode(register_button, INPUT_PULLDOWN);
  pinMode(attendance_button, INPUT_PULLDOWN);

  //WIFI CONNECT
  Serial.begin(115200); 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.println("...");
  }
 
  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("--------------");
  Serial.println();

  //OLED
  Wire.begin(33,32);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
  
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }



  //RFID SETUP
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
 
 
  //PRINT SENSOR
  finger.begin(57600);
  if (finger.verifyPassword()) {
  Serial.println("Found fingerprint sensor!");
  } else {
  Serial.println("Did not find fingerprint sensor :(");
  while (1) { delay(1); }
  }


  display.setRotation(2);
  show("PRESS", "BLUE TO REGISTER", "GREEN TO SIGN-IN");

 
}
 
void loop()
{
  
  
  if (digitalRead(register_button) == 1)
  {
    int passed = 0;
    delay(200);
    while(digitalRead(register_button) == 1)
    {
      passed += 1;
      delay(1000);
      if (passed>8)
      {
          finger.emptyDatabase();
          Serial.println("Now database is empty :)");
          show("DATABASE", "CLEARED", "!!!!!!!!!!");
          delay(4000);
          String response5 = send_to_server("clear_db", "", "", "" );
          ESP.restart();

      }
    }

    

    show("-------------", "REGISTERATION", "------------");
    delay(4000);
    String response = add_std();
    Serial.println(response);
    show("PRESS", "BLUE TO REGISTER", "GREEN TO SIGN-IN");
  }


  if (digitalRead(attendance_button) == 1)
  {
   show("-------------", "SIGNING IN", "------------");
    delay(1500);
    String response =  add_atd();
    Serial.println(response);
    show("PRESS", "BLUE TO REGISTER", "GREEN TO SIGN-IN");
  }

  //delay(50);
   
}



/// FUNCTIONS ///

String enrol_print(int aid){
if (aid == 0) {// ID #0 not allowed, try again!
return "ID NOT ALLOWED";
}
Serial.print("Enrolling ID #");
Serial.println(aid);
int code = 1;
 
while(code != 0)
{ //returns 1 if everything works fine
  code = getFingerprintEnroll(aid);
  Serial.print("CODE:");
  Serial.println(code);
}
 return "Print Enrolment Done!";
}

String add_std()
{
  String added_id = "";
  Serial.print("Getting next available id .... :");
  String response = send_to_server("get_id", "", "", "" );

  deserializeJson(doc, response);
  const char* mode = doc["mode"];
  int available_id  = doc["message"];

  if (available_id != 0)
  {
    Serial.println(available_id);
    show("NEW STUDENT ID:", String(available_id), "------------");
    delay(4000);
    String response1 =  enrol_print(available_id);
    Serial.println(response1);
    added_id = String(available_id);
  }

  String card_uid = get_card_uid();
  if (card_uid=="failed"){show("SOMETHING", "WENT" , "WRONG"); return "failed";}
  else
  {
  Serial.println();
  Serial.print(F("Unique ID: "));
  Serial.println(card_uid);
  Serial.println();
  show("NEW STUDENT NAME:", "ENTER NAME ON ", "SERVER/COMPUTER");
  String response10 = send_to_server("add_std", card_uid, added_id, "" );
  if (response10 == "{\"mode\": \"response\", \"message\": \"card already added\"}" )
      {
        deleteFingerprint(available_id);
        show("CARD", "ALREADY", "ASSIGNED");
        delay(4000);
      }
    
  return response10;
  }
  
}



String add_atd()
{

  show("HOLD DOWN", "BLUE TO USE CARD", "GREEN TO USE PRINT");
  while(digitalRead(register_button) == 0 && digitalRead(attendance_button) == 0)
  {
    
  }

  if (digitalRead(register_button) == 1)
  {
    show("-------------", "BY CARD", "------------");
    delay(1000);
    String card_uid = get_card_uid();
    if (card_uid !="failed")
    {
      Serial.println();
      Serial.print(F("Unique ID: "));
      Serial.println(card_uid);
      Serial.println();
      show("STUDENT ID:"+card_uid, "ADDED TO" , "ATTENDANCE");
      delay(3000);
      String response = send_to_server("add_atd", card_uid, "", "" );
      deserializeJson(doc, response);
      const char* mode = doc["mode"];
      const char* message  = doc["message"];
      
      return response;
    }

    else
    {
      show("SOMETHING:", "WENT" , "WRONG");
      

    }
  }


  if (digitalRead(attendance_button) == 1)
  {
    show("-------------", "BY PRINT", "------------");
    delay(1000);
    uint8_t print_id_placed =  getFingerprintIDez();
    if (print_id_placed !=0 )
    {
      show("STUDENT PRINT ID:"+print_id_placed, "ADDED TO" , "ATTENDANCE");
      delay(2000);
      String response = send_to_server("add_atd", "", String(print_id_placed), "" );
      deserializeJson(doc, response);
      const char* mode = doc["mode"];
      const char* message  = doc["message"];
      return response;
    }
    else
    {
      show("SOMETHING:", "WENT" , "WRONG");
      
    }

  }
  
  


 
  

}


String send_to_server(String mode, String card , String print, String name)
{
    WiFiClient client;
    if (!client.connect(host, port)) 
    {
        Serial.println("Connection to host failed");
        delay(1000);
        return "failed to Connect";
    }
    Serial.println("Connected to server successful!");
    String to_send = "{\"mode\":\"" + mode + "\",\"card_id\":\"" + card + "\",\"print_id\":\"" + print + "\", \"name\":\"" + name + "\"}";
    Serial.println(to_send);  //display
    char message[to_send.length() + 1];
    strcpy(message, to_send.c_str());
    
    client.print(message);
    String str = "";              
    Serial.println("Waiting for server response..");
    while (str=="")
    str = client.readStringUntil('\n');  // read entire response
    disconnect_from_server();
    deserializeJson(doc, str);
    const char* dmessage  = doc["message"];

    show("SERVER SAYS:", dmessage , "");
    delay(6000);
    return str;
 
}

void disconnect_from_server()
{
   // Serial.println("Disconnecting...");
    client.stop();
}

String get_card_uid()
{

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  Serial.println("Place Card on Scanner..");
  show("PLACE CARD", "ON" , "SCANNER");
  while ( ! rfid.PICC_IsNewCardPresent()) {}
  

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
  {
    Serial.println("Card Error!");
    return "failed";
  }

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return "failed";
  }

  // Store NUID into nuidPICC array
  for (byte i = 0; i < 4; i++) {nuidPICC[i] = rfid.uid.uidByte[i]; }
   
  // Serial.println(F("The NUID tag is:"));
  // Serial.print(F("In hex: "));
  // printHex(rfid.uid.uidByte, rfid.uid.size);
  // Serial.println();
  // Serial.print(F("In dec: "));
  // printDec(rfid.uid.uidByte, rfid.uid.size);

  //convert to string
  String unique_card_id ="1"; //just avoid number starting with 0
  for (byte i = 0; i < rfid.uid.size; i++)
  {
    if (rfid.uid.uidByte[i] > 0x10)
    {
    int in_int = int(rfid.uid.uidByte[i]);
    String in_string = String(in_int);
    unique_card_id = unique_card_id + in_string;
    }
  }

  
  // Halt PICC
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
  return unique_card_id;
}


/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

uint8_t readnumber(void) {
  uint8_t num = 0;
  
  while (num == 0) {
  while (! Serial.available());
  num = Serial.parseInt();
  }
  return num;
}
 
uint8_t getFingerprintEnroll(int aid ) {
 
int p = -1;
Serial.print("Waiting for valid finger to enroll as #"); Serial.println(aid);
show("STUDENT ID:"+String(aid), "PLACE FINGER" , "");
while (p != FINGERPRINT_OK) {
p = finger.getImage();
switch (p) {
case FINGERPRINT_OK:
Serial.println("Image taken");
show("STUDENT ID:"+String(aid), "IMAGE TAKEN" , "");
delay(4000);
break;
case FINGERPRINT_NOFINGER:
Serial.println(".");
break;
case FINGERPRINT_PACKETRECIEVEERR:
Serial.println("Communication error");
show("STUDENT ID:"+String(aid), "COMMUNICATION ERROR" , "");
delay(4000);
break;
case FINGERPRINT_IMAGEFAIL:
Serial.println("Imaging error");
show("STUDENT ID:"+String(aid), "IMAGING ERROR" , "");
delay(4000);
break;
default:
Serial.println("Unknown error-1");
show("STUDENT ID:"+String(aid), "UNKNOWN ERROR" , "");
delay(4000);
break;
}
}
 
// OK success!
 
p = finger.image2Tz(1);
switch (p) {
case FINGERPRINT_OK:
Serial.println("Image converted");
break;
case FINGERPRINT_IMAGEMESS:
Serial.println("Image too messy");
show("STUDENT ID:"+String(aid), "MESSY PRINT" , "");
delay(4000);
return p;
case FINGERPRINT_PACKETRECIEVEERR:
Serial.println("Communication error");
show("STUDENT ID:"+String(aid), "COMMUNICATION ERROR" , "");
delay(4000);
return p;
case FINGERPRINT_FEATUREFAIL:
Serial.println("Could not find fingerprint features");
show("STUDENT ID:"+String(aid), "IMAGING ERROR" , "");
delay(4000);
return p;
case FINGERPRINT_INVALIDIMAGE:
Serial.println("Could not find fingerprint features");
show("STUDENT ID:"+String(aid), "IMAGING ERROR" , "");
delay(4000);
return p;
default:
Serial.println("Unknown error-2");
show("STUDENT ID:"+String(aid), "UNKNOWN ERROR" , "");
delay(4000);
return p;
}
 
Serial.println("Remove finger");
show("STUDENT ID:"+String(aid), "REMOVE FINGER" , "");
delay(4000);
p = 0;
while (p != FINGERPRINT_NOFINGER) {
p = finger.getImage();
}
Serial.print("ID "); Serial.println(aid);
p = -1;
Serial.println("Place same finger again");
show("STUDENT ID:"+String(aid), "PLACE FINGER AGAIN" , "");
while (p != FINGERPRINT_OK) {
p = finger.getImage();
switch (p) {
case FINGERPRINT_OK:
Serial.println("Image taken");
break;
case FINGERPRINT_NOFINGER:
Serial.print(".");
break;
case FINGERPRINT_PACKETRECIEVEERR:
Serial.println("Communication error");
show("STUDENT ID:"+String(aid), "COMMUNICATION ERROR" , "");
delay(4000);
break;
case FINGERPRINT_IMAGEFAIL:
Serial.println("Imaging error");
show("STUDENT ID:"+String(aid), "IMAGING ERROR" , "");
delay(4000);
break;
default:
Serial.println("Unknown error-3");
show("STUDENT ID:"+String(aid), "UNKNOWN ERROR" , "");
delay(4000);
break;
}
}
 
// OK success!
 
p = finger.image2Tz(2);
switch (p) {
case FINGERPRINT_OK:
Serial.println("Image converted");
break;
case FINGERPRINT_IMAGEMESS:
Serial.println("Image too messy");
show("STUDENT ID:"+String(aid), "UNKNOWN ERROR" , "");
delay(4000);
return p;
case FINGERPRINT_PACKETRECIEVEERR:
Serial.println("Communication error");
return p;
case FINGERPRINT_FEATUREFAIL:
Serial.println("Could not find fingerprint features");
show("STUDENT ID:"+String(aid), "UNKNOWN ERROR" , "");
delay(4000);
return p;
case FINGERPRINT_INVALIDIMAGE:
Serial.println("Could not find fingerprint features");
show("STUDENT ID:"+String(aid), "UNKNOWN ERROR" , "");
delay(4000);
return p;
default:
Serial.println("Unknown error-4");
show("STUDENT ID:"+String(aid), "UNKNOWN ERROR" , "");
delay(4000);
return p;
}
 
// OK converted!
Serial.print("Creating model for #"); Serial.println(aid);
 
p = finger.createModel();
if (p == FINGERPRINT_OK) {
Serial.println("Prints matched!");

} else if (p == FINGERPRINT_PACKETRECIEVEERR) {
Serial.println("Communication error");
show("STUDENT ID:"+String(aid), "UNKNOWN ERROR" , "");
delay(4000);
return p;
} else if (p == FINGERPRINT_ENROLLMISMATCH) {
Serial.println("Fingerprints did not match");
show("STUDENT ID:"+String(aid), "UNKNOWN ERROR" , "");
delay(4000);
return p;
} else {
//Serial.println("Unknown error-5");
return p;
}
 
Serial.print("ID "); Serial.println(aid);
p = finger.storeModel(aid);
if (p == FINGERPRINT_OK) {
Serial.println("Stored!");
show("STUDENT ID:"+String(aid), "PRINT STORED!" , "");
delay(4000);
} else if (p == FINGERPRINT_PACKETRECIEVEERR) {
Serial.println("Communication error");
show("STUDENT ID:"+String(aid), "UNKNOWN ERROR" , "");
delay(4000);
return p;
} else if (p == FINGERPRINT_BADLOCATION) {
Serial.println("Could not store in that location");
show("STUDENT ID:"+String(aid), "UNKNOWN ERROR" , "");
delay(4000);
return p;
} else if (p == FINGERPRINT_FLASHERR) {
Serial.println("Error writing to flash");
show("STUDENT ID:"+String(aid), "UNKNOWN ERROR" , "");
delay(4000);
return p;
} else {
Serial.println("Unknown error-6");
return p;
}
}



uint8_t getFingerprintIDez() {



  uint8_t p = FINGERPRINT_NOFINGER;
  Serial.print("Place Finger on Sensor->");
  show("PLACE", "FINGER ON" , "SENSOR");
  while (p==FINGERPRINT_NOFINGER)
  {
    p = finger.getImage();
  }

  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      show("", "PRINT TAKEN" , "");
      delay(4000);
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      show("", "UNKNOWN ERROR" , "");
      delay(4000);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      show("", "UNKNOWN ERROR" , "");
      delay(4000);
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      show("", "UNKNOWN ERROR" , "");
      delay(4000);
      return p;
    default:
      Serial.println("Unknown error");
      show("", "UNKNOWN ERROR" , "");
      delay(4000);
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      show("", "UNKNOWN ERROR" , "");
      delay(4000);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      show("", "UNKNOWN ERROR" , "");
      delay(4000);
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      show("", "UNKNOWN ERROR" , "");
      delay(4000);
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      show("", "UNKNOWN ERROR" , "");
      delay(4000);
      return p;
    default:
      Serial.println("Unknown error");
      show("", "UNKNOWN ERROR" , "");
      delay(4000);
      return p;
  }
  
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    show("", "UNKNOWN ERROR" , "");
    delay(4000);
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    show("", "UNKNOWN ERROR" , "");
    delay(4000);
    return p;
  } else {
    Serial.println("Unknown error");
    show("", "UNKNOWN ERROR" , "");
    delay(4000);
    return p;
  }   
  
  // found a match!
  // Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  //Serial.print(" with confidence of "); Serial.println(finger.confidence); 
  delay(2000);
  show("STUDENT ID:", String(finger.fingerID) , "");
  delay(4000);
  return finger.fingerID;
}


uint8_t deleteFingerprint(uint8_t did) {
  uint8_t p = -1;

  p = finger.deleteModel(did);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
  }

  return p;
}