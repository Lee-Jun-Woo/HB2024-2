#include <Keypad.h>
#include <Servo.h>

// 핀 설정
const int trigPin = 4;
const int echoPin = 5;
const int ledPin = 3;
const int servoPin = 2;

// 거리 및 상태 변수
long duration;
int distance;
bool isPersonNearby = false;
bool changePasswordMode = false;
bool isConfirmed = false; // 비밀번호 인증 상태

// 서보모터 객체 생성
Servo servo;

// 키패드 설정
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6};    // 행 핀
byte colPins[COLS] = {13, 12, 11, 10}; // 열 핀

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// 초기 비밀번호 및 입력 변수
String defaultPassword = "123A";
String inputPassword = "";
String newPassword = "";

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  servo.attach(servoPin);
  servo.write(0); // 서보모터 초기 위치

  Serial.begin(9600);
}

void loop() {
  detectPerson();

  if (isPersonNearby) {
    if (!changePasswordMode) {
      analogWrite(ledPin, 50); // LED 약하게 켜기
    }

    char key = keypad.getKey();

    if (key) {
      handleKeypadInput(key);
    }
  } else {
    resetSystem();
  }

  delay(10);
}

void detectPerson() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  isPersonNearby = (distance <= 20);
}

void handleKeypadInput(char key) {
  if (changePasswordMode) {
    handlePasswordChange(key);
  } else {
    handlePasswordEntry(key);
  }
}

void handlePasswordEntry(char key) {
  if (key == '*') { // 비밀번호 확인 버튼
    if (inputPassword == defaultPassword) {
      Serial.println("Password Correct. Door Opened.");
      openDoor();
    } else {
      Serial.println("Password Incorrect.");
    }
    resetInput();
  } else if (key == 'D') { // 비밀번호 변경 모드 진입
    changePasswordMode = true;
    analogWrite(ledPin, 255); // LED 강하게 켜기
    Serial.println("Entering Password Change Mode.");
    resetInput();
  } else {
    inputPassword += key;
    Serial.println(inputPassword);
  }
}

void handlePasswordChange(char key) {
  if (!isConfirmed) { // 현재 비밀번호 확인 단계
    if (key == '*') { // 확인 버튼
      if (inputPassword == defaultPassword) {
        Serial.println("Current Password Correct. Enter New Password.");
        isConfirmed = true;  // 인증 완료
        inputPassword = "";  // 입력 초기화 후 새 비밀번호 대기
      } else {
        Serial.println("Incorrect Password. Exiting Password Change Mode.");
        changePasswordMode = false;
        analogWrite(ledPin, 50); // LED 원상복구
        resetInput();
      }
    } else {
      inputPassword += key;
      Serial.println(inputPassword);
    }
  } else { // 새로운 비밀번호 입력 단계
    if (key == '*') { // 새 비밀번호 입력 완료
      defaultPassword = inputPassword; // 비밀번호 갱신
      Serial.println("Password Changed Successfully.");
      changePasswordMode = false;
      isConfirmed = false;  // 인증 상태 초기화
      analogWrite(ledPin, 50); // LED 원상복구
      resetInput();
    } else {
      inputPassword += key;
      Serial.println(inputPassword);
    }
  }
}

void openDoor() {
  servo.write(90); // 서보모터 90도 회전
  delay(5000);     // 5초 유지
  servo.write(0);  // 서보모터 원위치
}

void resetInput() {
  inputPassword = ""; // 입력 초기화
}

void resetSystem() {
  analogWrite(ledPin, 0); // LED 끄기
  changePasswordMode = false;
  isConfirmed = false; // 인증 상태 초기화
  isPersonNearby = false;
  resetInput();
}
