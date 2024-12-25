#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ESP32Servo.h>

// Wi-Fi dan Firebase
#define SSID "iPhone Dilaa" // Nama Wi-Fi
#define PASSWORD "11111111" // Password Wi-Fi
#define API_KEY "AIzaSyCiT6RENeD5SRtaPBDEvI5zwtwtCA4Xefo" // API Key Firebase
#define DATABASE_URL "https://bismillahptc-2bf27-default-rtdb.firebaseio.com/" // URL Database Firebase

// PIN SENSOR INFRARED
#define IR_SENSOR_3_PIN 22 // Sensor Infrared SAMPAH SEDANG (LOGAM)
#define IR_SENSOR_4_PIN 23 // Sensor Infrared SAMPAH SEDANG (LOGAM)
#define IR_SENSOR_5_PIN 21 // Sensor Infrared SAMPAH PENUH (LOGAM)
#define IR_SENSOR_6_PIN 19 // Sensor Infrared SAMPAH PENUH (LOGAM)
#define IR_SENSOR_7_PIN 33 // Sensor Infrared SAMPAH SEDANG (NON-LOGAM)
#define IR_SENSOR_8_PIN 32 // Sensor Infrared SAMPAH SEDANG (NON-LOGAM)
#define IR_SENSOR_9_PIN 25 // Sensor Infrared SAMPAH PENUH (NON-LOGAM)
#define IR_SENSOR_10_PIN 26 // Sensor Infrared SAMPAH PENUH (NON-LOGAM)

// SENSOR LOGAM
const int sensorInduktifPin = 34; // Pin 
const int threshold = 3550;       // Ambang batas 

// SENSOR NON-LOGAM
#define NON_METAL_SENSOR_PIN 35 

// SERVO
Servo myServoLogam;         // Objek Servo untuk LOGAM
Servo myServoNonLogam;      // Objek Servo untuk NON-LOGAM
const int servoPinLogam = 18;    // Pin Servo untuk LOGAM
const int servoPinNonLogam = 5;  // Pin Servo untuk NON-LOGAM
int servoOpenAngle = 90;    // Sudut buka servo
int servoCloseAngle = 0;    // Sudut tutup servo

// BUZZER 
#define BUZZER_LOGAM_PIN 12 // Pin buzzer LOGAM
#define BUZZER_NON_LOGAM_PIN 13 // Pin buzzer NON-LOGAM

// Firebase instance
FirebaseData fbdo;      // Untuk mengatur data dari Firebase
FirebaseAuth auth;      // Untuk autentikasi pengguna ke Firebase
FirebaseConfig config;  // Untuk menyimpan pengaturan koneksi ke Firebase

void setup() {
  Serial.begin(115200);

  // Mengatur pin sensor infrared sebagai input
  pinMode(IR_SENSOR_3_PIN, INPUT);
  pinMode(IR_SENSOR_4_PIN, INPUT);
  pinMode(IR_SENSOR_5_PIN, INPUT);
  pinMode(IR_SENSOR_6_PIN, INPUT);
  pinMode(IR_SENSOR_7_PIN, INPUT);
  pinMode(IR_SENSOR_8_PIN, INPUT);
  pinMode(IR_SENSOR_9_PIN, INPUT);
  pinMode(IR_SENSOR_10_PIN, INPUT);

  // Mengatur pin untuk sensor logam dan non-logam
  pinMode(sensorInduktifPin, INPUT);
  pinMode(NON_METAL_SENSOR_PIN, INPUT);

  // Mengatur Servo untuk logam dan non-logam
  myServoLogam.attach(servoPinLogam);    // Servo untuk LOGAM
  myServoNonLogam.attach(servoPinNonLogam); // Servo untuk NON-LOGAM
  myServoLogam.write(servoCloseAngle);   // Servo logam mulai dalam posisi tertutup
  myServoNonLogam.write(servoCloseAngle); // Servo non-logam mulai dalam posisi tertutup

  // Mengatur Buzzer
  pinMode(BUZZER_LOGAM_PIN, OUTPUT);
  pinMode(BUZZER_NON_LOGAM_PIN, OUTPUT);

  // Menghubungkan ke Wi-Fi
  Serial.println("Memulai koneksi Wi-Fi...");
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Menghubungkan ...");
    delay(300);
  }
  Serial.print("Terhubung ke Wi-Fi dengan IP: ");
  Serial.println(WiFi.localIP());

  // Firebase configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  Serial.println("Menghubungkan ke Firebase...");

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("SignUp berhasil");
  } else {
    Serial.printf("Error SignUp: %s\n", config.signer.signupError.message.c_str());
  }
  Firebase.begin(&config, &auth);
  Serial.println("Firebase dimulai.");
}

void loop() {
  // BAGIAN 1: BACAAN SENSOR INFRARED
  int irStatus3 = digitalRead(IR_SENSOR_3_PIN);
  int irStatus4 = digitalRead(IR_SENSOR_4_PIN);
  int irStatus5 = digitalRead(IR_SENSOR_5_PIN);
  int irStatus6 = digitalRead(IR_SENSOR_6_PIN);
  int irStatus7 = digitalRead(IR_SENSOR_7_PIN);
  int irStatus8 = digitalRead(IR_SENSOR_8_PIN);
  int irStatus9 = digitalRead(IR_SENSOR_9_PIN);
  int irStatus10 = digitalRead(IR_SENSOR_10_PIN);

  String statusMetal, statusNonMetal;
  if (irStatus5 == LOW || irStatus6 == LOW) {
    statusMetal = "SAMPAH PENUH";
  } else if (irStatus3 == LOW || irStatus4 == LOW) {
    statusMetal = "SAMPAH SEDANG";
  } else {
    statusMetal = "SAMPAH KOSONG";
  }

  if (irStatus9 == LOW || irStatus10 == LOW) {
    statusNonMetal = "SAMPAH PENUH";
  } else if (irStatus7 == LOW || irStatus8 == LOW) {
    statusNonMetal = "SAMPAH SEDANG";
  } else {
    statusNonMetal = "SAMPAH KOSONG";
  }

  Serial.println("-------------------------------------------------");
  Serial.print("Status Sampah Logam: ");
  Serial.println(statusMetal);
  Serial.print("Status Sampah Non-Logam: ");
  Serial.println(statusNonMetal);

  // KIRIM DATA KE FIREBASE
  if (Firebase.RTDB.setString(&fbdo, "/DATA_SENSOR/STATUS_LOGAM", statusMetal)) {
    Serial.println("Status logam berhasil dikirim ke Firebase");
  } else {
    Serial.print("Gagal mengirim status logam ke Firebase: ");
    Serial.println(fbdo.errorReason());
  }
  if (Firebase.RTDB.setString(&fbdo, "/DATA_SENSOR/STATUS_NON_LOGAM", statusNonMetal)) {
    Serial.println("Status non-logam berhasil dikirim ke Firebase");
  } else {
    Serial.print("Gagal mengirim status non-logam ke Firebase: ");
    Serial.println(fbdo.errorReason());
  }


// BAGIAN 2: BACAAN SENSOR LOGAM DAN NON-LOGAM
int sensorValue = analogRead(sensorInduktifPin);
int nonMetalDetected = digitalRead(NON_METAL_SENSOR_PIN);

bool logamTerdeteksi = (sensorValue < threshold);
bool nonLogamTerdeteksi = (nonMetalDetected == LOW);

// MENAMPILKAN STATUS SENSOR
Serial.print("Nilai Sensor Logam: ");
Serial.println(sensorValue);

if (logamTerdeteksi && nonLogamTerdeteksi) {
  Serial.println("Logam dan Non-Logam Terdeteksi!");

  if (statusMetal == "SAMPAH PENUH") {
    for (int i = 0; i < 3; i++) {
      digitalWrite(BUZZER_LOGAM_PIN, HIGH);
      delay(500);
      digitalWrite(BUZZER_LOGAM_PIN, LOW);
      delay(500);
    }
  }

  if (statusNonMetal == "SAMPAH PENUH") {
    for (int i = 0; i < 3; i++) {
      digitalWrite(BUZZER_NON_LOGAM_PIN, HIGH);
      delay(500);
      digitalWrite(BUZZER_NON_LOGAM_PIN, LOW);
      delay(500);
    }
  }

  if (statusMetal != "SAMPAH PENUH") {
    myServoLogam.write(servoOpenAngle);  // Buka tutup LOGAM
  } else {
    myServoLogam.write(servoCloseAngle);  // Tutup LOGAM jika penuh
  }

  if (statusNonMetal != "SAMPAH PENUH") {
    myServoNonLogam.write(servoCloseAngle);  // Buka tutup NON-LOGAM
  } else {
    myServoNonLogam.write(servoOpenAngle);  // Tutup NON-LOGAM jika penuh
  }

  delay(3000);
  myServoLogam.write(servoCloseAngle);
  myServoNonLogam.write(servoOpenAngle);
} else {
  if (logamTerdeteksi) {
    Serial.println("Sampah Logam Terdeteksi");
    if (statusMetal == "SAMPAH PENUH") {
      for (int i = 0; i < 3; i++) {
        digitalWrite(BUZZER_LOGAM_PIN, HIGH);
        delay(500);
        digitalWrite(BUZZER_LOGAM_PIN, LOW);
        delay(500);
      }
    } else {
      myServoLogam.write(servoOpenAngle);
      delay(3000);
      myServoLogam.write(servoCloseAngle);
    }
  }

  if (nonLogamTerdeteksi) {
    Serial.println("Sampah Non-Logam Terdeteksi");
    if (statusNonMetal == "SAMPAH PENUH") {
      for (int i = 0; i < 3; i++) {
        digitalWrite(BUZZER_NON_LOGAM_PIN, HIGH);
        delay(500);
        digitalWrite(BUZZER_NON_LOGAM_PIN, LOW);
        delay(500);
      }
    } else {
      myServoNonLogam.write(servoCloseAngle);
      delay(3000);
      myServoNonLogam.write(servoOpenAngle);
    }
  }
}

  // KIRIM STATUS DETEKSI LOGAM DAN NON-LOGAM TERDETEKSI KE FIREBASE
  if (logamTerdeteksi && nonLogamTerdeteksi) {
  Serial.println("Logam dan Non-Logam Terdeteksi Bersamaan");
  if (Firebase.RTDB.setString(&fbdo, "/DATA_SENSOR/DETEKSI_LOGAM_NONLOGAM", "TERDETEKSI BERSAMA")) {
    Serial.println("Status logam dan non-logam terdeteksi bersamaan berhasil dikirim ke Firebase");
  } else {
    Serial.print("Gagal mengirim status logam dan non-logam terdeteksi bersamaan ke Firebase: ");
    Serial.println(fbdo.errorReason());
  }
} else {
  Serial.println("Logam dan Non-Logam Tidak Terdeteksi Bersamaan");
  if (Firebase.RTDB.setString(&fbdo, "/DATA_SENSOR/DETEKSI_LOGAM_NONLOGAM", "TIDAK TERDETEKSI BERSAMA")) {
    Serial.println("Status logam dan non-logam tidak terdeteksi bersamaan berhasil dikirim ke Firebase");
  } else {
    Serial.print("Gagal mengirim status logam dan non-logam tidak terdeteksi bersamaan ke Firebase: ");
    Serial.println(fbdo.errorReason());
  }
}
  
  // KIRIM STATUS DETEKSI LOGAM KE FIREBASE
  if (sensorValue < threshold) {
    if (Firebase.RTDB.setString(&fbdo, "/DATA_SENSOR/DETEKSI_LOGAM", "TERDETEKSI")) {
      Serial.println("Status deteksi logam berhasil dikirim ke Firebase");
    } else {
      Serial.print("Gagal mengirim status deteksi logam ke Firebase: ");
      Serial.println(fbdo.errorReason());
    }
  } else {
    if (Firebase.RTDB.setString(&fbdo, "/DATA_SENSOR/DETEKSI_LOGAM", "TIDAK TERDETEKSI")) {
      Serial.println("Status deteksi logam berhasil dikirim ke Firebase");
    } else {
      Serial.print("Gagal mengirim status deteksi logam ke Firebase: ");
      Serial.println(fbdo.errorReason());
    }
  }

  // KIRIM STATUS DETEKSI NON-LOGAM KE FIREBASE
  if (nonMetalDetected == LOW) {
    if (Firebase.RTDB.setString(&fbdo, "/DATA_SENSOR/DETEKSI_NON_LOGAM", "TERDETEKSI")) {
      Serial.println("Status deteksi non-logam berhasil dikirim ke Firebase");
    } else {
      Serial.print("Gagal mengirim status deteksi non-logam ke Firebase: ");
      Serial.println(fbdo.errorReason());
    }
  } else {
    if (Firebase.RTDB.setString(&fbdo, "/DATA_SENSOR/DETEKSI_NON_LOGAM", "TIDAK TERDETEKSI")) {
      Serial.println("Status deteksi non-logam berhasil dikirim ke Firebase");
    } else {
      Serial.print("Gagal mengirim status deteksi non-logam ke Firebase: ");
      Serial.println(fbdo.errorReason());
    }
  }
  
  delay(500);
}
