#include "LedControl.h"

// declare all required pins
const byte pbPin = 2;  // Pushbutton
const byte Xpin = A0;
const byte Ypin = A1;

const byte DATA = 12;
const byte CLOCK = 11;
const byte CS = 10;  // currently selected device pin
const byte NUM_DEVICES = 1;  // The number of devices being controlled

int Xmin, Xmax, Ymin, Ymax, X, Y, Xmapped, Ymapped;
bool wrapAround = false; // determines if the display wraps around or stops at the edge
bool ledblinkState = true;
int buttonState = 0;

int currentLed[2] = {0, 0}; // current led position
int currentJoystick[2] = {512, 512}; // current led position
int LedArr[8][8] = {{0, 0, 0, 0, 0, 0, 0, 0}, 
                        {0, 0, 0, 0, 0, 0, 0, 0}, 
                        {0, 0, 0, 0, 0, 0, 0, 0}, 
                        {0, 0, 0, 0, 0, 0, 0, 0}, 
                        {0, 0, 0, 0, 0, 0, 0, 0}, 
                        {0, 0, 0, 0, 0, 0, 0, 0}, 
                        {0, 0, 0, 0, 0, 0, 0, 0}, 
                        {0, 0, 0, 0, 0, 0, 0, 0}};

LedControl matrix = LedControl(DATA, CLOCK, CS, NUM_DEVICES);  // Initialize led matrix array library






//Calibrate joystick by setting movement upper and lower bounds
void joystickAutoCalibrate() {
  Serial.println("Move joystick to all four corners and click when done");
  Xmin = Xmax = analogRead(Xpin); // Read and set initial values
  Ymin = Ymax = analogRead(Ypin);

  while (digitalRead(pbPin) == HIGH) {
    X = analogRead(Xpin);  // Read X joystick pot
    if (X < Xmin) {
      Xmin = X; // Save the lowest X value read
    }
    if (X > Xmax) {
      Xmax = X; //Save the highest X value read
    }
    Y = analogRead(Ypin);  // Read Y joystick pot
    if (Y < Ymin) {
      Ymin = Y; // Save the lowest Y value read
    }
    if (Y > Ymax) {
      Ymax = Y; // Save the highest X value read
    }
  }

  Serial.print("Xmin: ");
  Serial.print(Xmin);
  Serial.print(", Xmax: ");
  Serial.print(Xmax);

  Serial.print("  =  Ymin: ");
  Serial.print(Ymin);
  Serial.print(", Ymax: ");
  Serial.println(Xmax);
}

// get direction and intensity of joystick movement
void getDirection() {
  Xmapped = map(analogRead(Xpin), Xmin, Xmax, 0, 1023);
  Ymapped = map(analogRead(Ypin), Ymin, Ymax, 0, 1023);

  // detect movrment in the x axis
  if (Xmapped < currentJoystick[0]) {
    moveLed('L', map((currentJoystick[0] - Xmapped), 0, 513, 0, 4));
  } else if (Xmapped > currentJoystick[0]) {
    moveLed('R', map((Xmapped - currentJoystick[0]), 0, 513, 0, 4));
  }
  // detect movrment in the x axis
  if (Ymapped < currentJoystick[1]) {
    moveLed('U', map((currentJoystick[1] - Ymapped), 0, 513, 0, 4));
  } else if (Ymapped > currentJoystick[1]) {
    moveLed('D', map((Ymapped - currentJoystick[1]), 0, 513, 0, 4));
  }
}

// dictates where the Led moves
void moveLed(char ledDir, int intensity) {
  if (intensity == 0) {
    return;
  }

  matrix.clearDisplay(0);
  drawLeds();

  switch (ledDir) {
    case 'R':
      currentLed[1]++;
      break;
    case 'L':
      currentLed[1]--;
      break;
    case 'U':
      currentLed[0]--;
      break;
    case 'D':
      currentLed[0]++;
      break;
    default:
      Serial.print("error");
  }

  // set matrix as continuous or edge bound
  if (wrapAround) {
    if (currentLed[0] > 7) {
      currentLed[0] = 0;
    } else if (currentLed[0] < 0) {
      currentLed[0] = 7;
    }

    if (currentLed[1] > 7) {
      currentLed[1] = 0;
    } else if (currentLed[1] < 0) {
      currentLed[1] = 7;
    }
  } else {
    if (currentLed[0] > 7) {
      currentLed[0] = 7;
    } else if (currentLed[0] < 0) {
      currentLed[0] = 0;
    }

    if (currentLed[1] > 7) {
      currentLed[1] = 7;
    } else if (currentLed[1] < 0) {
      currentLed[1] = 0;
    }
  }
}

// sets LED as on or off on LED matrix
void printLed(bool state) {
  matrix.setLed(0, currentLed[0], currentLed[1], state);
}

void drawLeds() {
  matrix.clearDisplay(0);
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (LedArr[i][j] == 1) {
        matrix.setLed(0, i, j, true);
      }else {
        matrix.setLed(0, i, j, false);
      }
    }   
  }  
}











void setup() {
  pinMode(pbPin, INPUT_PULLUP);  // Initialize pushbutton
  Serial.begin(9600);
  Serial.print("");

  matrix.shutdown(0, false);  // Wake up the first display from power-saving mode
  matrix.setIntensity(0, 15);  // Set maximum brightness on the first display for the LEDS (0-15)
  matrix.clearDisplay(0);

  printLed(true); // set LED initial position
  joystickAutoCalibrate();
  delay(2000);
}

void loop() {
  Serial.println("Ready...");
  unsigned int paintTime = 0;
 
  while (digitalRead(pbPin) == HIGH) {
    unsigned int futureTime = millis() + 200;
    while (millis() < futureTime) { // read joystick direction while blinking LED
      buttonState = 0;
      getDirection();
      printLed(ledblinkState);
      delay(50);
    }
    ledblinkState = !ledblinkState;

    Serial.print("X = "); // print LED coordinates
    Serial.print(currentLed[0]);

    Serial.print(", Y = ");
    Serial.print(currentLed[1]);

    Serial.print(", PB = ");
    Serial.println(digitalRead(pbPin));
  }

  if (buttonState == 0 && (millis() - paintTime) > 100) {
    if (LedArr[currentLed[0]][currentLed[1]] == 1) {
      LedArr[currentLed[0]][currentLed[1]] = 0;
    }else {
      LedArr[currentLed[0]][currentLed[1]] = 1;
    }
    
    buttonState = 1;
    paintTime = millis();
  }
  drawLeds();
}
