#include <WiFiNINA.h>
#include <math.h>

// wifi ip, ssid and password, change as needed
char ssid[] = "L221";
char pass[] = "zero1234";
WiFiClient client;
IPAddress server(192,168,218,122);

// initialize arduino pins
const int IN1 = 17;
const int IN2 = 19;
const int IN3 = 3;
const int IN4 = 2;

const int PWMA = 16;
const int PWMB = 5;

const int LEYE = 9;
const int REYE = 10;

const int US_TRIG=6;
const int US_ECHO=7;

const int LENC = 13;
const int RENC = 11;

// initialize global variables and constants

volatile int LCNTR = 0;
volatile int RCNTR = 0;

const double c1 = 0.5;
const double c2 = 0.25;
const double c3 = 0.1;

const int SETPOINT = 15;
const double CIRC = M_PI * 6.7;

unsigned long currTime;
unsigned long prevTime = 0;
unsigned long revTime = 0;

volatile int LCTR = 0;

float deltaT = 0;
float elapsedTime = 0;

int propError = 0;
int prevError = 0;

int distance = 0;
float instV = 0;
int prevDist = 0;
bool start = false;
int count = 0;

int pwm = 0;


bool OverSpeLimit(unsigned int spe);

// checks if buggy is going over speed limit
bool OverSpeLimit(unsigned int spe) {
  if (spe > 255) {
    return true;
  }
  return false;
}
// stops buggy
void ResetMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// buggy go forward and adjusts speed by passing in pwm
void Forward(unsigned int spe) {
  if (OverSpeLimit(spe)) return;
  analogWrite(PWMA, spe);
  analogWrite(PWMB, spe);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN4, HIGH);
}
// buggy go backwards and adjusts speed by passing in pwm
void Backward(unsigned int spe) {
  if (OverSpeLimit(spe)) return;
  analogWrite(PWMA, spe);
  analogWrite(PWMA, spe);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
}

// buggy turns and adjusts each wheel's speed by passing in pwm
void Turn(unsigned int leftspeed, unsigned int rightspeed) {
  if (OverSpeLimit(leftspeed) || OverSpeLimit(rightspeed)) return;
//  ResetMotor();
  analogWrite(PWMA, leftspeed);
  analogWrite(PWMB, rightspeed);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN4, HIGH);
}

// checks whether the infrared sees a non-black object, and acts accordingly
void InfraRed(bool left, bool right, int pwm) {
  if (left && right) {
    return;
  } else if (left) {
    Turn(pwm, 0);
  } else if (right) {
    Turn(0, pwm);
  } else {
    // ResetMotor();
    return;
  }
}
// finds distance of objects using ultrasonic sensor
long UltrasonicPoll (long duration){
   digitalWrite(US_TRIG, LOW);
   delayMicroseconds(2);
   digitalWrite(US_TRIG, HIGH);
   delayMicroseconds(10);
   digitalWrite(US_TRIG, LOW);
   pinMode(US_ECHO, INPUT);
   duration = pulseIn(US_ECHO, HIGH);
   return duration;
   }

// bronze challenge ultrasonic control flow
void UltrasonicFlow(){
   long duration, cm;
   duration = UltrasonicPoll(duration);
   distance = duration * 0.034/2;
   count = 0;
    while (distance <= 3) {
      ResetMotor();
      if (count == 0) {
        client.write("Object detected at ");
        client.print(distance);
        client.write(" centimetres, stopping\n");
      }
      delay(100);
      duration = UltrasonicPoll(duration);
      distance = duration*0.034/2;
      count++;
  }
}

// checks what the processing sends, to see if the buggy should be on or off
bool ToggleOn(char c, bool prev) {
  if (c == 'w') {
    return true;
  }
  if (c == 's') {
    return false;
  }
  return prev;
}

// Interrupts for infrared left eye that are currently commented out
void LEYEISR() {
  if (digitalRead(LEYE) && digitalRead(REYE)) return;
  Turn(pwm/4, pwm * 0.75);
}

// Interrupts for infrared right eye that are currently commented out
void REYEISR() {
  if (digitalRead(LEYE) && digitalRead(REYE)) return;
  Turn(pwm * 0.75, pwm/4);
}

// function that calculates PID using input parameter
double CalcPID(int input) {
  currTime = millis();
  elapsedTime = (float)(currTime - prevTime);
  elapsedTime = elapsedTime/1.0e3;

  propError = SETPOINT - input;

  double itgError = itgError + ((double)propError * elapsedTime);
  double devError = (propError - prevError) / elapsedTime;

  double totError = (c1 * propError) + (c2 * itgError) + (c3 * devError);

  prevError = propError;
  prevTime = currTime;
  return totError;
}

// function that calculates instantaneous velocity using wheel encoders
double CalcInstVelocity() {
  return  (CIRC / 4) / (deltaT);
}

// interrupt that calls CalcInstVelocity and updates buggy velocity
void LENC_Counter() {
  currTime = millis();
  deltaT = (currTime - revTime)/1.0e3;
  if (LCTR == 120) {
      instV = CalcInstVelocity();
      revTime = currTime;
      LCTR = 0;
  } else {
    LCTR++;
  }
}


// function for PID control flow in silver challenge
double PIDCtrlFlow() {
  long duration;
  duration = UltrasonicPoll(duration);
  distance = duration * 0.034/2;
  count = 0;
  while (distance <= 6) {
    ResetMotor();
    delay(100);
    duration = UltrasonicPoll(duration);
    distance = duration*0.034/2;
    count++;
  }
  double err = CalcPID(distance);
  float targetV = (distance - prevDist) / elapsedTime;
  String str_dist = String(distance);
  str_dist += " " + String(instV) + " " + String(targetV);
  if ( (millis() % 10) == 0){
    client.print(str_dist);
    Serial.println(str_dist);
  }
  prevDist = distance;
  return err;
}

// setup function
void setup() {

  Serial.begin(9600);
  WiFi.begin(ssid, pass);
  IPAddress ip = WiFi.localIP();
  client.connect(server, 5203);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  
  pinMode(LEYE, INPUT);
  pinMode(REYE, INPUT);
  
  pinMode(US_TRIG, OUTPUT);
  pinMode(US_ECHO, INPUT);

  pinMode(LENC, INPUT_PULLUP);
  pinMode(RENC, INPUT_PULLUP);


  client.write("Buggy ready!\n");
  // attachInterrupt(digitalPinToInterrupt(LEYE), LEYEISR, LOW); // interrupt implementation for infrared left eye
  // attachInterrupt(digitalPinToInterrupt(REYE), REYEISR, LOW); // interrupt implementation for infrared right eye
  attachInterrupt(digitalPinToInterrupt(LENC), LENC_Counter, RISING);
  // attachInterrupt(digitalPinToInterrupt(RENC), RENC_Counter, RISING); // right encoder interrupt for gold (that was not to be)
  

}

// main function
void loop() {
  start = ToggleOn(client.read(), start);
  if (start) {
    double err = PIDCtrlFlow();
    err = err / elapsedTime;
    pwm = 100 - err;
    if (pwm > 255) {
      pwm = 192;
    }
    Forward(pwm);
    InfraRed(digitalRead(LEYE), digitalRead(REYE), pwm);
    delay(100);
  } else {
    ResetMotor();
  }
}
