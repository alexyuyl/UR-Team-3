#include <NewPing.h>

bool leftMode = true;

//Motor Setup
#define left_in1 9
#define left_in2 6
#define right_in1 11
#define right_in2 10


int defined_speed = 100; // For bluetooth control
int turn_defined_speed = 50;
int forward_defined_speed = 75;
int left_defined_speed = 50;
int right_defined_speed = 50;
int backward_defined_speed = 50;

int forward_defined_time = 250;
int left_defined_time = 250;
int right_defined_time = 250;
int backward_defined_time = 250;

//Ultrasonic Sensor Setup
#define TRIGGER_PIN_1 2
#define ECHO_PIN_1 3
#define TRIGGER_PIN_2 4
#define ECHO_PIN_2 5
#define TRIGGER_PIN_3 12
#define ECHO_PIN_3 A1
#define MAX_DISTANCE 200

NewPing front_ultrasonic(TRIGGER_PIN_1, ECHO_PIN_1, MAX_DISTANCE);
NewPing left_ultrasonic(TRIGGER_PIN_2, ECHO_PIN_2, MAX_DISTANCE);

double duration_1;
int left_distance = 0;
int front_distance = 0;
int right_distance = 0;
int front_defined_distance = 20;
int left_defined_distance = 20;
int right_defined_distance = 20;
int tooclose_defined_distance = 10;

bool front_has_wall = false;
bool left_has_wall = true;
bool right_has_wall = true;
bool left_tooclose = false;
bool right_tooclose = true;


//Time delay setup
unsigned long previousMillis = 0;

//Maze
bool mazeSolved = true;

//Movement of Robot
void brake(){
  analogWrite(right_in1,255);
  analogWrite(right_in2,255);
  analogWrite(left_in1,255);
  analogWrite(left_in2,255);  
}

// the connection of the motors is not well set uo and may need further re-soldering
void moveForward(int speed){
  analogWrite(left_in1,speed);
  analogWrite(left_in2,0);
  analogWrite(right_in1,speed);
  analogWrite(right_in2,0);
}

void turnLeft(int speed){
  analogWrite(left_in1,0);
  analogWrite(left_in2,speed);
  analogWrite(right_in1,speed);
  analogWrite(right_in2,0);

}

void turnRight(int speed){
  analogWrite(left_in1,speed);
  analogWrite(left_in2,0);
  analogWrite(right_in1,0);
  analogWrite(right_in2,speed);
} 

void moveBackward(int speed){
  analogWrite(left_in1,0);
  analogWrite(left_in2,speed);  
  analogWrite(right_in1,0);
  analogWrite(right_in2,speed);
}

void moveForward(unsigned long time, int speed){
  while(true){
    moveForward(speed);
    if(millis() - previousMillis > time){
      previousMillis += time;
      brake();
      break;
    }
  }
}

void turnLeft(unsigned long time,int speed){
  while(true){
    turnLeft(speed);
    if(millis() - previousMillis > time){
      previousMillis += time;
      break;
    }
  }
  brake();
}

void turnRight(unsigned long time, int speed){
  while(true){
    turnRight(speed);
    if(millis() - previousMillis > time){
      previousMillis += time;
      break;  
    }  
  }
  brake();
} 

void moveBackward(unsigned long time, int speed){
  while(true){
    moveBackward(speed);
    if(millis() - previousMillis > time){
      previousMillis += time;
      break;
    }
  }
  brake();
}

void startMaze(){
  mazeSolved = false;
}

void setMazeCompleted(){
  mazeSolved = true;
}
//Bluetooth Connection and Command Setup
byte command;
void executeCommand(int command){
  Serial.println(command);
  switch(command){
    case 0:
      break;
    case 255:
      brake();
      break;
    case 251:
      moveForward(defined_speed);
      break;
    case 252:
      turnLeft(turn_defined_speed);
      break;
    case 253:
      turnRight(turn_defined_speed);
      break;
    case 254:
      moveBackward(defined_speed);
      break;
    case 246:
      startMaze();
      break;
    case 247:
      setMazeCompleted();
      break;
    case 248:
      Serial.println("test hi");
      break;
    case 245:
      moveForward(1000,defined_speed);
      break;
    default: // when speed is sent
      defined_speed = command;
      break;
  }
}

void check_distance(){
  digitalWrite(TRIGGER_PIN_3,LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN_3,HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN_3, LOW);
  delayMicroseconds(2);
  duration_1 = pulseIn(A1, HIGH);
  right_distance = (duration_1)*0.034/2;
  front_distance = front_ultrasonic.ping_cm();
  left_distance = left_ultrasonic.ping_cm();
  Serial.print("Left: ");
  Serial.println(left_distance);
  Serial.print("Front: ");
  Serial.println(front_distance);
  Serial.print("Right: ");
  Serial.println(right_distance);
}

void check_wall(){
  if(front_distance<front_defined_distance){
    front_has_wall = true;
  } else {
    front_has_wall = false;
  }
  if(left_distance<left_defined_distance){
    left_has_wall = true;  
  } else {
    left_has_wall = false;
  }
  if(right_distance<right_defined_distance){
    right_has_wall = true;
  } else {
    right_has_wall = false;
  }
  if(left_distance - right_distance > tooclose_defined_distance){
    right_tooclose = true;
  } else right_tooclose = false;
  if(right_distance - left_distance > tooclose_defined_distance){
    left_tooclose = true;
  } else left_tooclose = false;
}

void deviate_check(){
  /*if(left_tooclose){
    turnRight(25);
  }
  if(right_tooclose){
    turnLeft(25);
  } */
}
void maze(){
  if(leftMode){
    if(!front_has_wall){ // First rule: if there is road forward
      moveForward(forward_defined_speed);
      deviate_check();
    } else if (!left_has_wall){ // Second rule: if there is road leftward
      Serial.print("count started");
      turnLeft(left_defined_time,left_defined_speed);
      Serial.print("count finished");
      moveForward(forward_defined_time,forward_defined_speed);
      } else { //Third rule: if there is no road for left and forward
      Serial.print("count started");
      turnRight(right_defined_time,right_defined_speed);
      Serial.print("count finished");
    }
  } else {
    if(!front_has_wall){ // First rule: if there is road to left
      moveForward(forward_defined_speed);
    } else if (!right_has_wall){ // Second rule: if there is road forward
      brake();
      turnRight(right_defined_time,right_defined_speed);
      moveForward(forward_defined_time,forward_defined_speed);
      } else { //Third rule: if there is no road for left and forward
      brake();
      turnLeft(left_defined_time,left_defined_speed);
    }
  }
}

void setup() {
  Serial.begin(9600);
  // pinMode of ultrasonic sensors are defined in NewPing library itself
  pinMode(right_in1,OUTPUT);
  pinMode(right_in2,OUTPUT);
  pinMode(left_in1,OUTPUT);
  pinMode(left_in2,OUTPUT);
  pinMode(TRIGGER_PIN_3, OUTPUT);
  pinMode(ECHO_PIN_3, INPUT);
}

void loop() {
  if(Serial.available()){
    command = Serial.read();    
    executeCommand(command);
  }

  if(!mazeSolved){ // Zero rule: if the maze is not completed
    check_distance();
    check_wall();
    maze();
  }
}
