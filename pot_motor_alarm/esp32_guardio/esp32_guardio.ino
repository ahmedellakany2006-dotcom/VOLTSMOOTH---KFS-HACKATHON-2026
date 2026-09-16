#include <ESP32Servo.h>

// --- تعريف المنافذ ---
#define POT_PIN        34   // منفذ البوتنشميتر
#define SERVO_PIN      26   // منفذ السيرفو
#define RELAY_PIN      4   // منفذ الريلاي (التحكم في الموتور)
#define BUZZER_PIN     25   // منفذ البازر 
#define RED_LED_PIN    19   // منفذ اللمبة الحمراء (الإنذار)
#define GREEN_LED_PIN  18   // منفذ اللمبة الخضراء (الأمان)

// --- إعدادات حالات الريلاي ---
// لو الريلاي شغال بالعكس وقت التجربة (بيفصل في الأمان ويشتغل في الخطر)، بدل HIGH و LOW هنا
#define RELAY_ON       LOW  
#define RELAY_OFF      HIGH 

Servo relayServo;

// حد الإنذار (لو البوتنشميتر عدى 2000 هيضرب إنذار)
const int ALARM_THRESHOLD = 2000; 

// متغيرات لعمل فلاش بدون delay
unsigned long previousMillis = 0;
bool alarmState = false;

void setup() {
  Serial.begin(115200);

  // إعداد المنافذ
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(POT_PIN, INPUT);

  // إعداد السيرفو
  ESP32PWM::allocateTimer(0);
  relayServo.setPeriodHertz(50);
  relayServo.attach(SERVO_PIN, 500, 2400);

  // تشغيل الموتور (أو الريلاي) كوضع افتراضي للأمان
  digitalWrite(RELAY_PIN, RELAY_ON);

  Serial.println("VoltSmooth Control Test (Dry Run) Ready!");
  Serial.println("Turn the Potentiometer to test limits...");
}

void loop() {
  // 1. قراءة البوتنشميتر
  int potValue = analogRead(POT_PIN);
  
  // 2. اتخاذ القرار
  if (potValue > ALARM_THRESHOLD) {
    // -----------------------------------------
    // 🚨 حالة الخطر (الإنذار والفصل)
    // -----------------------------------------
    relayServo.write(90);                 // السيرفو يتحرك لـ 90 درجة (محاكاة العزل)
    digitalWrite(RELAY_PIN, RELAY_OFF);   // الريلاي يفصل فجأة (هتسمع تكتكة)
    digitalWrite(GREEN_LED_PIN, LOW);     // إطفاء الأخضر

    // فلاش للمبة الحمراء وصوت للبازر كل 200 ملي ثانية
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 200) {
      previousMillis = currentMillis;
      alarmState = !alarmState; 
      
      digitalWrite(RED_LED_PIN, alarmState);
      
      if (alarmState) {
        tone(BUZZER_PIN, 1000); 
      } else {
        noTone(BUZZER_PIN);
      }
    }

  } else {
    // -----------------------------------------
    // ✅ الحالة الطبيعية (الأمان والتشغيل)
    // -----------------------------------------
    relayServo.write(0);                  // إرجاع السيرفو لـ 0 درجة (الوضع الطبيعي)
    digitalWrite(RELAY_PIN, RELAY_ON);    // الريلاي يلقط (هتسمع تكتكة رجوع)
    
    digitalWrite(RED_LED_PIN, LOW);       // إطفاء الأحمر
    noTone(BUZZER_PIN);                   // كتم البازر
    digitalWrite(GREEN_LED_PIN, HIGH);    // إضاءة الأخضر بثبات
  }

  // إرسال القراءة للسيريال عشان الداشبورد تشوفها
  Serial.print("POT: ");
  Serial.println(potValue);

  // تأخير بسيط لاستقرار القراءة
  delay(50); 
}