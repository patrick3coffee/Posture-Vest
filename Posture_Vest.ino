#include <SimpleTimer.h>

#define BODY_SENSORS  1
#define BUTTON_PIN    A4
#define VIBE_PIN      A3
#define RED_PIN       12
#define BLUE_PIN      14
#define GREEN_PIN     13
#define LIGHT_SENSOR  A2
#define INDICATOR_LUX 255
#define VIBE_OFFSET   9

// Global variables
SimpleTimer masterTimer;

bool vibeState, ledState, alertState;
int currentReading,
    previousReading,
    sensorTimerId,
    vibeTimerId,
    ledTimerId,
    whiteLeds[6];

int sensors[BODY_SENSORS][4] = {      // { pin, deviation, threshold, reading }
  {A5, -4, 0, 0}                     // belly sensor
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
  vibeTimerId = masterTimer.setInterval(500, toggleVibe);
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
      showColor(6);
      // wait for button release
      delay(10);
    }
    //Serial.println("button pressed");
    showColor(5);
    return true;
  }
}

void readSensors() {
  for (int sensor = 0; sensor < BODY_SENSORS; sensor++) {
    int readingSum, i;
    for (i = 0; i < 100; i++) {
      readingSum += analogRead(sensors[sensor][0]);
      delay(1);
      if (i % 50) {
        masterTimer.run();
      }
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
  int adjustedThreshold;
  for (int sensor = 0; sensor < BODY_SENSORS; sensor++) {
    if (vibeState) {
      adjustedThreshold = VIBE_OFFSET + sensors[sensor][2];
    }
    else {
      adjustedThreshold = sensors[sensor][2];
    }
    if (sensors[sensor][3] < adjustedThreshold) {
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
  masterTimer.run();
  delay(300);
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

int getBrightness(){
  int brightness, reading = analogRead(LIGHT_SENSOR);
  brightness = map(reading, 0, 1024, 0, INDICATOR_LUX);
  return brightness;
}

void startLedAlert() {
  toggleLed();
  masterTimer.enable(ledTimerId);
  //Serial.println("LED on");
  showColor(0);
}

void stopLedAlert() {
  masterTimer.disable(ledTimerId);
  //digitalWrite(whiteLeds[0], LOW);
  showColor(4);
  ledState = false;
  //Serial.println("LED off");
}

void toggleLed() {
  if (ledState) {
    //digitalWrite(whiteLeds[0], LOW);
    showColor(0);
  }
  else {
    //digitalWrite(whiteLeds[0], HIGH);
    showColor(5);
  }
  ledState = !ledState;
}

void showColor(int id) {
  switch (id) {
    case 0:     //black
      applyColor(0, 0, 0);
      break;
    case 1:     //red
      applyColor(255, 0, 0);
      break;
    case 2:     //green
      applyColor(0, 255, 0);
      break;
    case 3:     //blue
      applyColor(0, 0, 255);
      break;
    case 4:     //sea green
      applyColor(150, 255, 50);
      break;
    case 5:     //amber
      applyColor(255, 40, 10);
      break;
    case 6:     //violet
      applyColor(255, 10, 150);
      break;
    default:
      applyColor(0, 0, 0);
      break;
  }
}

void applyColor(int red, int green, int blue) {
  int newRed, newGreen, newBlue, ambient;
  ambient = getBrightness();
  newRed = map(red, 0, 255, 0, ambient);
  analogWrite(RED_PIN, newRed);
  newGreen = map(green, 0, 255, 0, ambient);
  analogWrite(GREEN_PIN, newGreen);
  newBlue = map(blue, 0, 255, 0, ambient);
  analogWrite(BLUE_PIN, newBlue);
}

