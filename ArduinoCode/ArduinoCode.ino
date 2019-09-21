#include <SoftwareSerial.h>

#define ProjectTitle          "=== Project ==="
#define ProjectValue          "Smart Street Latern"

#define VersionTitle          "=== Version ==="
#define VersionValue          __DATE__

#define BrandTitle            "=== Team Name ==="
#define BrandValue            "IEEE Student Branch, TEI Of Central Macedonia"

#define TeamTitle             "=== Team Members ==="
#define TeamValue             " * Kokozidis Pavlos\n * Kostelidis Iordanis\n * Salpigkti Alexandra \n * Stasinou Myrsini \n * Palioura Alexandra \n * Mitsopoulou Katerina \n * Theoxaridou Elpida-Vasilikis"

#define serialRate            38400
#define hc05DataRate          38400
#define hc05ATRate            38400

#define syncButton            2
#define syncLed               8
bool masterLatern   =         false;
bool synchedLatern  =         false;

#define slaveMac              "2017,3,130997"

#define hc05Vcc               3
#define hc05Key               4
#define hc05Rx                6
#define hc05Tx                5

SoftwareSerial                HC05(hc05Rx, hc05Tx);

#define carGreenLed           9
#define carYellowLed          10
#define carRedLed             11

#define pedestrianGreenLed    12
#define pedestrianRedLed      13

#define carRedSecs            10 * 1000
#define carYellowSecs         5 * 1000
#define carGreenSecs          10 * 1000

#define maxBTDevs 10


void setupSerial() {
  Serial.begin(serialRate);
}


void bootMessages() {
  Serial.println(F("================BOOT================"));
  Serial.println(F("------------------------------------"));
  Serial.println(F(ProjectTitle)); Serial.println(F(ProjectValue));
  Serial.println(F(VersionTitle)); Serial.println(F(VersionValue));
  Serial.println(F(BrandTitle)); Serial.println(F(BrandValue));
  Serial.println(F(TeamTitle)); Serial.println(F(TeamValue));
  Serial.println(F("---------------- END ----------------"));
}

void setupCar() {
  Serial.println(F("Init Car Latern"));
  pinMode(carRedLed, OUTPUT);
  pinMode(carYellowLed, OUTPUT);
  pinMode(carGreenLed, OUTPUT);
  Serial.println(F("OK Car Latern"));
}

void setupPedestrian() {
  Serial.println(F("Init Pedestrian Latern"));
  pinMode(pedestrianRedLed, OUTPUT);
  pinMode(pedestrianGreenLed, OUTPUT);
  Serial.println(F("OK Pedestrian Latern"));
}

void setupHC05() {
  analogWrite(hc05Key, 0);
  analogWrite(hc05Vcc, 255);
  beginHC05(hc05DataRate);
}

void beginHC05(double rate) {
  Serial.print(F("Start HC05 with this Baud ")); Serial.println(rate);
  HC05.begin(rate);
  delay(700);
}

void endHC05() {
  HC05.end();
  delay(700);
}

void setATMode(bool state) {
  double selectedRate;

  if (state) {
    Serial.print("Enable ");
    selectedRate = hc05ATRate;
    analogWrite(hc05Key, 255);
  } else {
    Serial.print("Disable ");
    selectedRate = hc05DataRate;
    analogWrite(hc05Key, 0);
  }

  Serial.println("AT Mode");

  endHC05();

  analogWrite(hc05Vcc, 0);
  delay(5000);
  analogWrite(hc05Vcc, 255);

  beginHC05(selectedRate);

}

void runATCommand(char command[], int delaySecs) {
  Serial.print("Running this command "); Serial.println(command);
  HC05.println("AT");
  delay(delaySecs * 1000);

  while (HC05.available()) {
    Serial.write(HC05.read());
  }

}

void setAllLaterns(bool state) {
  digitalWrite(carRedLed, state);
  digitalWrite(carYellowLed, state);
  digitalWrite(carGreenLed, state);

  digitalWrite(pedestrianRedLed, state);
  digitalWrite(pedestrianGreenLed, state);
}

void setSyncLed(bool state) {
  digitalWrite(syncLed, state);
}

bool syncWithSlave() {

  setATMode(true);
  initMaster();
  delay(1000);
  setATMode(false);
  delay(1000);

  HC05.println("A");
  delay(200);

  while (HC05.read() != 'B') {
    // Wait
  }

  HC05.println("C");
  Serial.println("I Sent C");

  return true;
}

void checkSyncButton() {
  bool syncButtonState = digitalRead(syncButton);
  if (syncButtonState) {
    Serial.println("Latern is master");
    masterLatern = true;
    synchedLatern = syncWithSlave();
    setSyncLed(synchedLatern);
  }
}

bool syncWithMaster() {
  HC05.println("B");

  delay(200);

  bool haveACK = false;

  Serial.println("Wait for ACK");
  while (HC05.read() != 'C') {
    // Wait
  }

  return true;
}

void checkSyncHC05() {
  if (HC05.available()) {
    if (HC05.read() == 'A') {
      masterLatern = false;
      synchedLatern = syncWithMaster();
      setSyncLed(synchedLatern);
    }
  }
}

void asynchronizedLoop() {
  while (!synchedLatern) {
    setAllLaterns(true);
    delay(1000);
    setAllLaterns(false);
    delay(1000);

    checkSyncButton();
    if (!masterLatern) {
      checkSyncHC05();
    }
  }
}

void setCar(char state) {

  digitalWrite(carRedLed, LOW);
  digitalWrite(carYellowLed, LOW);
  digitalWrite(carGreenLed, LOW);

  switch (state) {
    case 'r': {
        digitalWrite(carRedLed, HIGH);
        break;
      }
    case 'y': {
        digitalWrite(carYellowLed, HIGH);
        break;
      }
    case 'g': {
        digitalWrite(carGreenLed, HIGH);
        break;
      }
  }
}

void setPedestrian(char state) {

  digitalWrite(pedestrianRedLed, LOW);
  digitalWrite(pedestrianGreenLed, LOW);

  switch (state) {
    case 'r': {
        digitalWrite(pedestrianRedLed, HIGH);
        break;
      }
    case 'g': {
        digitalWrite(pedestrianGreenLed, HIGH);
        break;
      }
  }
}

int scanForDevs() {
  setATMode(true);
  initINQ();
  int maxDelay = runINQ();  
  setATMode(false);

  return maxDelay;
}

void resync() {

  digitalWrite(syncLed, LOW);

  Serial.println("Resync");

  if (masterLatern) {
    resyncWithSlave();
  } else {
    resyncWithMaster();
  }


  digitalWrite(syncLed, HIGH);
  Serial.println("Resynced");
}

void resyncWithMaster() {

  Serial.println("Resync with master");

  setATMode(true);
  initSlave();
  delay(1000);
  setATMode(false);
  delay(1500);

  while (HC05.read() != 'D') {
    // wait
  }

  HC05.print('E');

  Serial.println("Wait for ACK");

  while (HC05.read() != 'F') {
    // Wait
  }

}

void resyncWithSlave() {

  Serial.println("Resync with slave");


  setATMode(true);
  initMaster();
  delay(1000);
  setATMode(false);
  delay(1500);

  HC05.print('D');

  while (HC05.read() != 'E') {
    // Wait
  }

  HC05.print('F');



}

unsigned long extendedPedestrianGreenSecs;

int talkToOtherAndFindTheMaxAsMaster(int extendedPedestrianGreenSecs) {
    setATMode(true);
    delay(500);
    initMaster();
    delay(1000);
    setATMode(false);
    delay(1500);

    Serial.println("I want the slave's category");

    HC05.print('Q');
    delay(2000);

    char tmp = HC05.read();
    
    while(isntValidCategory(tmp)) {
      tmp = HC05.read();

      Serial.print(tmp);
    }

    int otherTime = categoryToSecs(tmp);

    int maxTime = extendedPedestrianGreenSecs;

    if(otherTime > maxTime) {
      maxTime = otherTime;
    }
    
    // Find the max and send
    HC05.print(secsToCategory(maxTime));
    delay(1000);
    while(HC05.read() != 'O') {
      // Wait
    }

    delay(1000);
    return maxTime;

}

int categoryToSecs(char category) {
  int result = 0;
  
  switch(category) {
    case 'A': {
      result = 0;
      break;
    }
    case 'B': {
      result = 10;
      break;
    }
    case 'C': {
      result = 20;
      break;
    }
  }

  return result;
}

bool isntValidCategory(char tmp) {
  if(tmp == 'A' || tmp == 'B' || tmp == 'C') {
    return false;
  } else {
    return true;
  }
}

char secsToCategory(int secs) {
  if(secs == 0) {
    return 'A';
  }

  if(secs == 10) {
    return 'B';
  }

  if(secs == 20) {
    return 'C';
  }

  return 0;
}

int talkToOtherAndFindTheMaxAsSlave(int extendedPedestrianGreenSecs) {
    setATMode(true);
    initSlave();
    delay(1000);
    setATMode(false);
    delay(1500);

    while(HC05.read() != 'Q') {
      // Wait
    }

    char cat = secsToCategory(extendedPedestrianGreenSecs);

    Serial.println(cat);

    HC05.print(cat);
    delay(1000);

    char tmp = HC05.read();
    
    while(isntValidCategory(tmp)) {
      tmp = HC05.read();
    }

    HC05.print('O');
    delay(1000);
    
    return categoryToSecs(tmp);
    
}

int talkToOtherAndFindTheMax(int extendedPedestrianGreenSecs) {
  if(masterLatern) {
    return talkToOtherAndFindTheMaxAsMaster(extendedPedestrianGreenSecs);
    
  } else {
    return talkToOtherAndFindTheMaxAsSlave(extendedPedestrianGreenSecs);
  }
}

void synchronizedLoop() {
  while (true) {
    extendedPedestrianGreenSecs = 0;

    // Make car led green, make pedestrian led red and scan for near bt devices
    setPedestrian('r');
    delay(2000);
    setCar('g');
    extendedPedestrianGreenSecs = scanForDevs();

    // find the real extended
    extendedPedestrianGreenSecs = talkToOtherAndFindTheMax(extendedPedestrianGreenSecs);
    
    delay(carGreenSecs);
    resync();
    // Make car led yellow and check for synch
    setCar('y');
    delay(carYellowSecs);
    // Make car led red and make pedestrian led green for pedestrianGreenSecs + extendedPedestrianGreenSecs
    setCar('r');
    delay(1000);
    setPedestrian('g');
    Serial.print("Delay="); Serial.print(extendedPedestrianGreenSecs);
    delay(carRedSecs + (extendedPedestrianGreenSecs * 1000));
  }
}

void setup() {
  setupSerial();

  bootMessages();

  setupCar();
  setupPedestrian();
  setupHC05();

    setATMode(true);
    initSlave();
    setATMode(false);

     runTests();

}

void runTests() {
  Serial.println("================TEST================");

  Serial.println("Light, Red, Car");
  setPinState(carRedLed, true);
  Serial.println("Dark, Red, Car");
  setPinState(carRedLed, false);

  Serial.println("Light, Yellow, Car");
  setPinState(carYellowLed, true);
  Serial.println("Dark, Yellow, Car");
  setPinState(carYellowLed, false);

  Serial.println("Light, Green, Car");
  setPinState(carGreenLed, true);
  Serial.println("Dark, Green, Car");
  setPinState(carGreenLed, false);

  Serial.println("Light, Red, Pedestrian");
  setPinState(pedestrianRedLed, true);
  Serial.println("Dark, Red, Pedestrian");
  setPinState(pedestrianRedLed, false);

  Serial.println("Light, Green, Pedestrian");
  setPinState(pedestrianGreenLed, true);
  Serial.println("Dark, Green, Pedestrian");
  setPinState(pedestrianGreenLed, false);
  Serial.println("================END================");
}

void setPinState(int pin, bool state) {
  digitalWrite(pin, state);
  delay(1000);
}

void loop() {


    if (synchedLatern) {
      Serial.println("Latern is synched");
      synchronizedLoop();
    } else {
      Serial.println("Latern is asynched");
      asynchronizedLoop();
    }
}

void initMaster() {
  Serial.println("RUN AT+ORGL");
  HC05.println("AT+ORGL");
  delay(200);
  while (HC05.available()) {
    Serial.write(HC05.read());
  }

  Serial.println("RUN AT+NAME");
  HC05.println("AT+NAME=IEEEMaster");
  delay(200);
  while (HC05.available()) {
    Serial.write(HC05.read());
  }

  Serial.println("RUN AT+ROLE");
  HC05.println("AT+ROLE=1");
  delay(200);
  while (HC05.available()) {
    Serial.write(HC05.read());
  }

  Serial.println("RUN AT+BIND");
  HC05.print("AT+BIND=");
  HC05.println(slaveMac);
  delay(200);
  while (HC05.available()) {
    Serial.write(HC05.read());
  }

  Serial.println("RUN AT+CMODE");
  HC05.println("AT+CMODE=0");
  delay(200);
  while (HC05.available()) {
    Serial.write(HC05.read());
  }
}

void initSlave() {
  Serial.println("RUN AT+ORGL");
  HC05.println("AT+ORGL");
  delay(200);
  while (HC05.available()) {
    Serial.write(HC05.read());
  }

  Serial.println("RUN AT+NAME");
  HC05.println("AT+NAME=IEEESlave");
  delay(200);
  while (HC05.available()) {
    Serial.write(HC05.read());
  }

  Serial.println("RUN AT+ROLE");
  HC05.println("AT+ROLE=0");
  delay(200);
  while (HC05.available()) {
    Serial.write(HC05.read());
  }
}

char devs[11][32];
unsigned short devIndex = 0;
unsigned short charIndex = 0;

char testDev[] = "CC9F:7A:26F868";
char testDevDelay = 10;

void initINQ() {
  HC05.println("AT+CMODE=1");
  delay(50);
  while (HC05.available()) {
    Serial.write(HC05.read());
  }
  delay(100);

  HC05.println("AT+ROLE=1");
  delay(100);
  while (HC05.available()) {
    Serial.write(HC05.read());
  }
  delay(100);

  HC05.println("AT+INQM=1,10,10");
  delay(50);
  while (HC05.available()) {
    Serial.write(HC05.read());
  }
  delay(100);

  HC05.println("AT+CLASS=0");
  delay(50);
  while (HC05.available()) {
    Serial.write(HC05.read());
  }
  delay(100);
}

int runINQ() {
  HC05.println("AT+INIT");
  delay(100);
  while (HC05.available()) {
    Serial.write(HC05.read());
  }
  delay(500);

  HC05.println("AT+INQ");
  delay(500);

  bool foundO = false;
  bool foundK = false;

  char tmpChar;

  Serial.println("Start parsing");

  devIndex = 0;
  charIndex = 0;

   for(int line=0;line<11;line++) {
    for (int pos = 0; pos < 32; pos++) {
      devs[line][pos] = ' ';
    }
   }

  while (!(foundO && foundK)) {
    if (HC05.available()) {
      tmpChar = HC05.read();

      Serial.print(tmpChar);

      if (tmpChar == '\n') {
        devIndex++;
        charIndex = 0;
        continue;
      }

      devs[devIndex][charIndex] = tmpChar;
      charIndex++;



      if (tmpChar == 'O') {
        foundO = true;
        continue;
      }

      if (tmpChar == 'K' && foundO) {
        foundK = true;
      } else {
        foundO = false;
        foundK = false;
      }
    }
  }

  delay(1000);

  Serial.println("End Parsing");

  // Remove the latest "device record", is the ok response
  for (int i = 0; i < 32; i++) {
    devs[10][i] = ' ';
  }

  // Clear 1 record

  char dev[32];
  char devIndex;

  int maxExtend = 0;

  for (int device = 0; device < 10; device++) {

    // Clear dev
    for (int pos = 0; pos < 32; pos++) {
      dev[pos] = ' ';
    }

    devIndex = 0;
    for (int i = 5; i < 32; i++) {
      dev[devIndex] = devs[device][i];
      devIndex++;
    }

    Serial.print("Cleared No"); Serial.print(device); Serial.print("=>");
    Serial.println(dev);

    bool matched = false;
    for(int i=0;i<6;i++) {
     matched = dev[i] == testDev[i];
    }

    if(matched) {
      if(testDevDelay >= maxExtend) {
        maxExtend = testDevDelay;
      }
    }
    
  }

  Serial.print("Max Extend=>");
  Serial.println(maxExtend);


  return maxExtend;

}
