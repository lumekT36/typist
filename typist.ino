// 4/14/23 - Personal secretary
// Keyboard is available only for HW with USB interface! (like ATmega32U4 - 5V SparkFun Pro Micro)
// SD card must be FAT16 256MB+ !

#include <SD.h>
#include <Keyboard.h>


#define chipSelect 10 // D10, A10
#define safetyPin  2  // D2 next to GND for easy jumper connection.

File file;


void setup() {
  randomSeed(analogRead(A2)); // floating / disconnected pin
  pinMode(safetyPin, INPUT_PULLUP);
  
  // Safety pin to prevent bricking the HW due flooding USB with data preventing it to be re-programmed!
  if (digitalRead(safetyPin) == HIGH)
    while(true);

  Keyboard.begin();

  // Serial.begin(115200);
  // while(!Serial);

  if (!SD.begin(chipSelect))
    throwError("SD card failed to initialize!");

  File root = SD.open("/");
  if (!root)
    throwError("Root directory failed to open!");
  
  file = openNextFile(root);
  if (!file)
    throwError("No more files in root directory!");

  typeStr("\n// " + String(file.name()) + " " + String(file.size()) + "\n");
}

void loop() {
  // If safety pin is disconnected pause processing.
  if (digitalRead(safetyPin) == HIGH)
    return;

  byte b = file.read();

  // On EOF hit enter, wait 10 seconds, and start over
  if (b == 255) {
    b = 10;
    file.seek(0);
    delay(10000);
  }

  // On ENTER handle all next non-printable characters (to fix autoformating of modern code editors)
  if (b == 10) {
    send(b); // Finish the ENTER (usually 13 & 10)

    do {    
      b = file.read();
    } while (b == 32 || b == 9); // Skip all spaces and tabs

    if (b == 41 || b == 125 || b == 93)
      send(8); // Backspace if next char is closing )}] to remove the auto indent
  }

  send(b);
}

File openNextFile(File dir) {
  File entry;

  while (true) {
    entry = dir.openNextFile();
    if (!entry || !entry.isDirectory())
      break;
  }

  return entry;
}

int getPause(byte b) {
  if (b == 32) // space
    return random(500, 1000);

  if (b == 10) // enter
    return random(100, 3000);

  // any other character
  return random(40, 400);
}

void send(byte b) {
  delay( getPause(b) );
  
  Keyboard.write(b);
  //Serial.println(b);
  //Serial.print(char(b));
}

void typeStr(String msg) {
  int l = msg.length();
  for (int i = 0; i < l; i++)
    send(msg[i]);
}

void throwError(String msg) {
  typeStr(msg);

  while(true); // Stop!
}
