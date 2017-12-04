#include <SimpleTimer.h>

SimpleTimer masterTimer;

#define BODY_SENSORS  1
#define BUTTON_PIN    A4
#define VIBE_PIN      11
#define RED_PIN       12
#define BLUE_PIN      14
#define GREEN_PIN     13
#define INDICATOR_LUX 40

// Global variables
bool vibeState, ledState, alertState;
int currentReading,
    previousReading,
    threshold,
    sensorTimerId,
    vibeTimerId,
    ledTimerId,
    whiteLeds[6];

int sensors[BODY_SENSORS][4] = {      // { pin, deviation, threshold, reading }
  {A5, -3, 0, 0}                     // belly sensor
};

void setup() {

  // setup serial communication for troubleshooting
  Serial.begin(9600);
  Serial.println("Posture Vest");

  // setup pins
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // set button
  pinMode(VIBE_PIN, OUTPUT);    // vibrator

  for ( int sensor = 0; sensor < BODY_SENSORS; sensor++) {
    pinMode(sensors[sensor][0], INPUT);
  }


  /*pinMode(6, OUTPUT);         // charlieplex pins
    pinMode(A7, OUTPUT);
    pinMode(A8, OUTPUT);
    pinMode(10, OUTPUT);*/

  // RGB Pins
  pinMode(RED_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);

  // white lights
  for (int led = 15; led <= 20; led++) {
    pinMode(led, OUTPUT);
    digitalWrite(led, HIGH);
    delay(50);
    digitalWrite(led, LOW);
    whiteLeds[ led - 15 ] = led;
  }

  sensorTimerId = masterTimer.setInterval(3000, beginAlert);
  vibeTimerId = masterTimer.setInterval(300, toggleVibe);
  ledTimerId = masterTimer.setInterval(100, toggleLed);
  masterTimer.enable(sensorTimerId);
  readSensors();
  setThresholds();
}

void loop() {
  masterTimer.run();

  // verify and enforce
  readSensors();
  bool aboveThreshold = sensorsAboveThreshold();

  if (aboveThreshold) {
    masterTimer.restartTimer(sensorTimerId);
    stopAlert();
  }

  // check for adjustments
  if (buttonPressed()) {
    setThresholds();
  }
}

/*
   Sensing Functions
*/

bool buttonPressed() {
  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == HIGH) {
    return false;
  }
  else {
    delay(10);      // debounce
    while (digitalRead(BUTTON_PIN) == LOW) {
      // wait for button release
      delay(10);
    }
    //Serial.println("button pressed");
    analogWrite(BLUE_PIN, INDICATOR_LUX);
    delay(50);
    digitalWrite(BLUE_PIN, LOW);
    return true;
  }
}

void readSensors() {
  for (int sensor = 0; sensor < BODY_SENSORS; sensor++) {
    int readingSum, i;
    for (i = 0; i < 30; i++) {
      readingSum += analogRead(sensors[sensor][0]);
      delay(1);
    }
    sensors[sensor][3] = readingSum / i;
  }
}

void printSensors() {
  Serial.print("sensor readings: ");
  for (int sensor = 0; sensor < BODY_SENSORS; sensor++) {
    Serial.print( sensors[sensor][3] );
    Serial.print("  ");
  }
  Serial.println(' ');
}

/*
   Threshold functions
*/

bool sensorsAboveThreshold() {
  for (int sensor = 0; sensor < BODY_SENSORS; sensor++) {
    if (sensors[sensor][3] < sensors[sensor][2]) {
      return false;
    }
  }
  return true;
}

void setThresholds() {
  printSensors();
  Serial.print("New Thresholds: ");
  for (int sensor = 0; sensor < BODY_SENSORS; sensor++) {
    sensors[sensor][2] = sensors[sensor][3] - sensors[sensor][1];
    Serial.print(sensors[sensor][2]);
    Serial.print("  ");
  }
  Serial.println(" ");
  stopAlert();
}

/*
   Alerting
*/

void beginAlert() {
  if (!alertState) {
    Serial.println("begin alert");
    alertState = true;
    printSensors();
  }
  startVibeAlert();
  startLedAlert();
}

void stopAlert() {
  if (alertState) {
    Serial.println("stop alert");
    alertState = false;
    printSensors();
  }
  stopVibeAlert();
  stopLedAlert();
  masterTimer.restartTimer(sensorTimerId);
}

/*
   Vibrator functions
*/

void startVibeAlert() {
  //Serial.println("vibe on");
  toggleVibe();
  masterTimer.enable(vibeTimerId);
}

void stopVibeAlert() {
  //Serial.println("vibe off");
  masterTimer.disable(vibeTimerId);
  digitalWrite(VIBE_PIN, LOW);
  vibeState = false;
}

void toggleVibe() {
  if (vibeState) {
    digitalWrite(VIBE_PIN, LOW);
  }
  else {
    digitalWrite(VIBE_PIN, HIGH);
  }
  vibeState = !vibeState;
}

/*
   LED functions
*/


void startLedAlert() {
  toggleLed();
  masterTimer.enable(ledTimerId);
  //Serial.println("LED on");
  digitalWrite(GREEN_PIN, LOW);
}

void stopLedAlert() {
  masterTimer.disable(ledTimerId);
  //digitalWrite(whiteLeds[0], LOW);
  digitalWrite(RED_PIN, LOW);
  analogWrite(GREEN_PIN, INDICATOR_LUX);
  ledState = false;
  //Serial.println("LED off");
}

void toggleLed() {
  if (ledState) {
    //digitalWrite(whiteLeds[0], LOW);
    digitalWrite(RED_PIN, LOW);
  }
  else {
    //digitalWrite(whiteLeds[0], HIGH);
    analogWrite(RED_PIN, INDICATOR_LUX);
  }
  ledState = !ledState;
}
