#include <Wire.h>
#include <MPU6050.h>

//encoder
int count = 0;
int phaseA = 2;
int phaseB = 3;

// Khai báo extern các hàm từ imu.ino
extern void initIMU();
extern void updateIMU();
extern void publishIMU();
const int stepPin = 8; //chân STEP
const int dirPin = 9; // chân DIR hướng
const int pwmPin = 10; //chân pwm
const int brake = 11; // chân thắng
const int direction = 12; //chạy tiến hoặc lùi
const int enaPin = 7;


int currentDuty = 0; 
unsigned long lastPumpTime = 0;
unsigned long lastCommandTime = 0; //biến thời gian
unsigned long pwmResetTime = 1500; // giới hạn reset pwm
const unsigned long pumpInterval = 1000; 

bool lastDirection = true; // Biến nhớ trạng thái hướng trước đó

float caster_angle = 0.0; // truyền thông trái phải
float drive_position = 0.0; // truyền thxxxxông tiến lùi

bool ondrection = true;
bool offdrection = true;


unsigned long lastSend = 0;



bool limit_ena_high = false;
bool interrupt_pulse = false;
bool isStepping = false;
bool isTurningLeft = false;


unsigned long lastStepTime = 0;
int stepCount = 0;
const int totalSteps = 640;

float target_yaw = 0.0;
bool inyaw = false;


void setup() {
  // put your setup code here, to run once:
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  pinMode(brake, OUTPUT);
  pinMode(direction, OUTPUT);
  pinMode(phaseA, INPUT_PULLUP);
  pinMode(phaseB, INPUT_PULLUP);
  pinMode(enaPin, OUTPUT);
  digitalWrite(enaPin, LOW);  


  attachInterrupt(digitalPinToInterrupt(phaseA), encoder, RISING);


  initIMU();  // Khởi tạo IMU

  Serial.begin(115200);



}

void loop() {
  // put your main code here, to run repeatedly:
  handleSerialInput();  // ← Gọi hàm xử lý bàn phím từ file phụ
  //cmd();
  updateIMU();  // Cập nhật dữ liệu IMU

  updateCasterAngle(); // cập nhật góc hiện tại
  handleStepping(); // Hàm sử lý step và góc

  checkAutoResetPWM();
  if (millis() - lastSend >100){
    sendJointState();
    publishIMU();      // Gửi dữ liệu IMU

    lastSend = millis();
  }

}

void sendJointState(){
  Serial.print("JOINT:");
  Serial.print(caster_angle);
  Serial.print(",");
  Serial.println(drive_position);
  

}

void checkAutoResetPWM(){
  if((millis() - lastCommandTime > pwmResetTime) && currentDuty > 0){
    currentDuty = 23;
    analogWrite(pwmPin, 23);
    Serial.println("Pwm tự reset về 0");

    delay(400);

  }
}
void stopbrake(){
  digitalWrite(brake, HIGH);
  if (currentDuty > 23){
    currentDuty =23;
    analogWrite(pwmPin, 23);
    Serial.println(">>pwm về 0");
  }
  delay(300);
  digitalWrite(brake, LOW);


}
void runForward(){
  lastCommandTime = millis();
  if (ondrection){
    currentDuty = 23;

    delay(500);
    ondrection = false;
  }
  digitalWrite(direction, LOW);
  if(!lastDirection){
    stopbrake();
    currentDuty = 23;
    analogWrite(pwmPin, 23);
    Serial.println(">> reset PWM lùi->tiến");
  }
  pumpPWM();
  offdrection = true;

  lastDirection = true;
  drive_position += (5.0 / 30.0);

}

void runBackward(){
  lastCommandTime = millis();
  
  if (offdrection){
    currentDuty = 23;

    delay(500);
    offdrection = false;
  }
  digitalWrite(direction, HIGH);
  if(lastDirection){
    stopbrake();
    currentDuty = 23;
    analogWrite(pwmPin, 23);
    Serial.println(">> reset PWM Tiến -> Lùi");
  }
  ondrection = true;

  pumpPWM();
  lastDirection = false;
  drive_position -= (5.0 / 30.0);


}
void pumpPWM(){
  if (currentDuty < 30){ // 81 max
    currentDuty +=1;
    if (currentDuty > 30) currentDuty = 30;

  }
  analogWrite(pwmPin, currentDuty); // 0.51 tới 1.59
  float voltage = (5.0 * currentDuty)/255.0;
  Serial.println("PWM:");
  Serial.print(currentDuty);
  Serial.print("(");
  Serial.print(voltage, 2);
  Serial.print("v)");
  
}
// void runLeft(){
//   // if (caster_angle >= 90.0) return;
//   digitalWrite(dirPin, HIGH);
//   for (int i=0; i < 640; i++){
//     digitalWrite(stepPin, HIGH);
//     delayMicroseconds(1500);
//     digitalWrite(stepPin, LOW);
//     // Serial.println("trai");
//   }
//   // caster_angle += 10.0;
//   // if (caster_angle > 90) caster_angle =90.0;
//   // Serial.print("Goc quay trai hien tai: ");
//   // Serial.println(caster_angle);
// }

// void runRight(){
  // if (caster_angle <= - 90.0) return;
  digitalWrite(dirPin, LOW);
  for (int i=0; i<640; i++){

    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1500);
    digitalWrite(stepPin, LOW);
    // Serial.println("phai");
  }
  // caster_angle -= 10.0;
  // if (caster_angle < -90.0) caster_angle = -90.0;
  // Serial.print("Goc quay phai hien tai: ");
  // Serial.println(caster_angle);
// }
void runLeft() {
  if (caster_angle >= 90.0 || isStepping) return;

  Serial.println("Bắt đầu quay trái");
  digitalWrite(dirPin, HIGH);
  stepCount = 0;
  isStepping = true;
  isTurningLeft = true;
  interrupt_pulse = false;
}

void runRight() {
  if (caster_angle <= -90.0 || isStepping) return;

  Serial.println("Bắt đầu quay phải");
  digitalWrite(dirPin, LOW);
  stepCount = 0;
  isStepping = true;
  isTurningLeft = false;
  interrupt_pulse = false;
}
void runEna() {
  interrupt_pulse = true;
  digitalWrite(enaPin, HIGH);
  delay(500);
  digitalWrite(enaPin, LOW);
}

void handleStepping() {
  if (isStepping && !interrupt_pulse) {
    if (inyaw) {
      if ((isTurningLeft && caster_angle >= target_yaw) || (!isTurningLeft && caster_angle <= target_yaw)) {
        isStepping = false;
        inyaw = false;
        Serial.println("Góc ok, kích ena");
        runEna();
        return;
      }
    }

    unsigned long now = micros();
    if (now - lastStepTime >= 1500) {
      digitalWrite(stepPin, HIGH);
      digitalWrite(stepPin, LOW);
      stepCount++;
      lastStepTime = now;

      if (stepCount >= totalSteps && !inyaw) {
        isStepping = false;
        Serial.println("Hoàn thành quay.");
      }
    }
  }

  if (interrupt_pulse && isStepping) {
    isStepping = false;
    inyaw = false;
    Serial.println("Đã ngắt quay!");
  }
}

void encoder() {
  if (digitalRead(phaseB) == LOW) {
    count++;
  } else {
    count--;
  }
}

void updateCasterAngle() {
  caster_angle = (count / 500.0) * 90.0;
}

