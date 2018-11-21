#include <Arduino.h>
#include <OneWire.h>
//#include <DS18B20.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS_1 23
#define ONE_WIRE_BUS_2 22
OneWire oneWire1(ONE_WIRE_BUS_1);
OneWire oneWire2(ONE_WIRE_BUS_2);
DallasTemperature fireDS(&oneWire1);
DallasTemperature oilDS(&oneWire2);
int interval = 1000;//Общий интервал для задач
int interval_oil = 500;//Интервал для масла
int interval_fire = 900; //Интервал для снятие показания с датчика пламени
int interval_air = 90000000;//Остановка подачи воздуха и искры через 90000000 = 1500 мин(25 часов). Требуется перезапуск чипа!
int interval_light = 900;//Регулируется искра пол секунды
int interval_working = 10000;//Интервал обратного отсчета для проверок
int interval_var = 5000;
int freq = 100; // Частота ШИМ для искры
int PWMChannel = 0; // Канал для ШИМ
int resolution = 8; // Разрешение ШИМ
int SparkPin = 12; // Любой пин от GPIO0 до GPIO15
int number_of_tries = 4;
long time1;
long time2;
long time3;
long time4;
long time5;
long time6;
long time7;
long time8;
long time9;
long time10;
int fire_temp_max = 29;
int fire_temp_min = 24;
int oil_temp_max = 26;
int oil_temp_min = 24;
#define poplavok 35
#define plamya 34
#define maslo_heat 14
#define air 27
//#define   12
#define nasos 13
float fire_temp;
float oil_temp;
bool fire_check;
bool START = false;
bool ON = true;
//bool PUMP = true;
int count = 1;
void setup() {
  pinMode(maslo_heat, OUTPUT);//Нагрев масла
  digitalWrite(maslo_heat, 1);
  //pinMode(iskra, OUTPUT);//Вкл/Выкл искры
  pinMode(nasos, OUTPUT);//Вкл/Выкл насос подачи масла
  digitalWrite(nasos, 1);
  pinMode(air, OUTPUT);//Вкл.Выкл насос подачи масла на горелку
  digitalWrite(air, 1);
  pinMode(poplavok, INPUT);//Поплавок наверху = 0 и внизу = 1
  pinMode(plamya, INPUT);//Пламя есть = 1 или нет = 0
  ledcSetup(PWMChannel, freq, resolution); // Инициализация ШИМ
  ledcAttachPin(SparkPin, PWMChannel); // Инициализация ШИМ
  ledcWrite(PWMChannel, 0); // Нет искры
  //digitalWrite(poplavok, 1);
  //digitalWrite(plamya, 1);
  Serial.begin(9600);
  Serial.println("Fire And Oil Heating!");
  fireDS.begin();
  oilDS.begin();
}

void start_handle(void);
void get_temp_handle(void);
void get_fire_handle(void);
void air_control_handle(void);
void oil_temp_handle(void);
void oil_pump_handle(void);
void flash_lightning(void);
void fire_temp_handle(void);
void working_handle(void);
void var_handle(void);

void loop() {
  start_handle();
  get_temp_handle();
  oil_pump_handle();
  oil_temp_handle();
  air_control_handle();
  flash_lightning();
  get_fire_handle();
  fire_temp_handle();
  working_handle();
  var_handle();
}
//Старт программы,закачка масла и нагрев
void start_handle() {
  if (START) return;
  if (time9 + interval < millis()) {
    Serial.println("Checking oil scale!");
    bool oil_pump_scale = digitalRead(poplavok);
    if (oil_pump_scale == 0) {
      digitalWrite(nasos, 0);
      digitalWrite(maslo_heat, 1);
      Serial.println("Pushing oil for start!");
    } else if(oil_temp < oil_temp_max && oil_pump_scale == 1) {
      digitalWrite(nasos, 1);
      digitalWrite(maslo_heat, 0);
      Serial.println("Heat Meat Beat!");
    } else {
      START = true;
      Serial.println("Mortal Kombat Begin!");
    }
    time9 = millis();
  }
}
// Работа насоса
void oil_pump_handle() {
  if (!START) return;
  if (!ON) return;
  if (time3 + interval_oil < millis()) {
    Serial.println("Checking oil scale!");
    bool oil_pump_scale = digitalRead(poplavok);
    if (oil_pump_scale == 0) {
      digitalWrite(nasos, 0);
      digitalWrite(air, 1);
      Serial.println("Oil scale is low! Pushing oil!");
    } else {
      digitalWrite(nasos, 1);
      Serial.println("Oil scale is normal!");
    }
    time3 = millis();
  }
}
// Получение данных с датчика пламени
void get_fire_handle() {
  if (!START) return;
  if (!ON) return;
  //if (!OIL) return;
  if (time7 + interval_fire < millis()) {
    fire_check = digitalRead(plamya);
    Serial.println("Checking Fire");
    Serial.println(digitalRead(plamya));
    time7 = millis();
  }
}
// Остановка подачи воздуха
void air_control_handle() {
  if (!START) return;
  if (ON) return;
  //if (time5 + interval_air < millis()) {
    //Serial.println("Turn Off Spark and Air after 3 try");
    if (fire_check == 0) {
      Serial.println("Turn Off Spark and Air after 3 try");
      digitalWrite(air, 1);
      digitalWrite(maslo_heat, 1);
      digitalWrite(nasos, 1);
      //digitalWrite(iskra, 0);
      ledcWrite(PWMChannel, 0); // Нет искры
    }
    delay(interval_air);
    //time5 = millis();
  //}
}
// Работа подачи искры
void flash_lightning() {
  if (!START) return;
  if (!ON) return;
  //if (!PUMP) return;
  if (time4 + interval_light < millis()) {
    if (fire_check == 1) {
      //digitalWrite(air, 0);
      //digitalWrite(iskra, 0);
      ledcWrite(PWMChannel, 0); // Нет искры
      Serial.println("Fire On The Hall!");
    } else if (fire_temp > fire_temp_max) {
      //digitalWrite(iskra, 0);
      ledcWrite(PWMChannel, 0); // Нет искры
      Serial.println("TEMP HIGH! Flashing_OFF!");
      //digitalWrite(air, 1);
    } else {
      //digitalWrite(iskra, 1);
      ledcWrite(PWMChannel, 64); // Есть искра
      Serial.println("Flashing!");
    }
    time4 = millis();
  }
}
// Работа подачи воздуха
void fire_temp_handle() {
  if (!START) return;
  if (!ON) return;
  //if (!PUMP) return;
  if (time1 + interval < millis()) {
    if (fire_temp < fire_temp_min) {
      digitalWrite(air, 0);
      Serial.println("Air ON!");
      flash_lightning();
    } else if(fire_temp > fire_temp_max){
      digitalWrite(air, 1);
      Serial.println("Air OFF!");
      flash_lightning();
    }
    time1 = millis();
  }
}
// Нагрев масла
void oil_temp_handle() {
  if (!START) return;
  if (!ON) return;
  //if (!OIL) return;
  if (time2 + interval < millis()) {
    bool oil_pump_scale = digitalRead(poplavok);
    if (oil_temp < oil_temp_min && oil_pump_scale == 1) {
      digitalWrite(maslo_heat, 0);
      Serial.println("Heating oil ON!");
    } else if (oil_temp > oil_temp_max){
      digitalWrite(maslo_heat, 1);
      Serial.println("Heating oil OFF!");
    }
    time2 = millis();
  }
}
// Получение температур с датчиков теплоносителя и масла
void get_temp_handle() {
  if (!ON) return;
  if (time6 + interval < millis()) {
    fireDS.requestTemperatures();
    fire_temp = fireDS.getTempCByIndex(0);
    Serial.println("TIME: " + String(time1) + ", FIRE_TEMPERATURE: " + String(fire_temp));
    oilDS.requestTemperatures();
    oil_temp = oilDS.getTempCByIndex(0);
    Serial.println("TIME: " + String(time2) + ", OIL_TEMPERATURE: " + String(oil_temp));
    // обрыв датчика сразу отключит систему!
    if (fire_temp == -127 || oil_temp == -127) {
      START = true;
      ON = false;
    }
    time6 = millis();
  }
}
// Проверка работа всей системы
void working_handle() {
  if (!START) return;
  if (time8 + interval_working < millis()) {
    if (count >= number_of_tries) {
      ON = false;
      Serial.println("Four Tries");
    } /*else if (fire_temp == -127 || oil_temp == -127) {
      ON = false;
      Serial.println("fire or oil goes down!");
    }*/ else if (fire_check == 0 && fire_temp < fire_temp_max){
      Serial.println("Попытка №" + String(count) + "\n");
      count++;
    } else {
      count = 1;
      Serial.println("Checking working of the system!");
    }
    time8 = millis();
  }
}
// переменные
void var_handle() {
  if(time10 + interval_var < millis()) {
    number_of_tries = number_of_tries; // число попыток после которой выключается система, если нет огня
    oil_temp_max = oil_temp_max; // масло греть до 80 градусов
    oil_temp_min = oil_temp_min;
    fire_temp_max = fire_temp_max; // регулируемый порог 40-50 градусов, т.е. держать температуру в этом пределе
    fire_temp_min = fire_temp_min;

    time10 = millis();
  }
}
