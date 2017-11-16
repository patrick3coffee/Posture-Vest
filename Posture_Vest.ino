void setup() {
  // setup serial communication for troubleshooting
  Serial.begin(9600);

  // setup pins
  pinMode(A4, INPUT_PULLUP);  // set button
  
  pinMode(A5, INPUT);         // belly sensor
  pinMode(A9, OUTPUT);        // belly vibrator

  /*pinMode(A2, INPUT);         // shoulder sensor
  pinMode(11, OUTPUT);*/        // shoulder vibrator

  /*pinMode(6, OUTPUT);         // charlieplex pins
  pinMode(A7, OUTPUT);
  pinMode(A8, OUTPUT);
  pinMode(10, OUTPUT);*/
  
  pinMode(15, OUTPUT);        // white lights
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(20, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:

}
