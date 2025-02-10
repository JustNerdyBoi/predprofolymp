const int dirPin = 2;
const int stepPin = 3;
const int buttonPin = 5;
const int stepsPerRevolution = 200;
const int length = 130;
const int motorStepDelay = 1250;

int myPins[length];

struct Point {
  int index;
  int value;
};
Point sortedData[130];      // Массив для хранения индексов и значений
Point detectedPoints[130];  // Массив для хранения обнаруженных точек
int detectedCount = 0;      // Счётчик обнаруженных точек

int buttonState = 0;
#define PIN_PHOTO_SENSOR A0

#define MIN_DISTANCE 10
#define BRIGHTNESS_PERCENT 15

void setup() {
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  digitalWrite(dirPin, LOW);
  digitalWrite(stepPin, LOW);
  Serial.begin(9600);
}
void loop() {
  buttonState = digitalRead(buttonPin);
  while (buttonState == 0) {
    buttonState = digitalRead(buttonPin);
  }

  digitalWrite(dirPin, HIGH);
  for (int t = 0; t < length; t++) {
    int val = analogRead(PIN_PHOTO_SENSOR);
    myPins[t] = val - 734;
    for (int i = 0; i < stepsPerRevolution / 8; i++) {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(motorStepDelay);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(motorStepDelay);
    }
  }
  Serial.println("====================================================================================");

  // Заполнение sortedData индексами и значениями из myPins
  for (int i = 0; i < 130; i++) {
    sortedData[i].index = i;
    sortedData[i].value = myPins[i];
  }

  // Сортировка sortedData по значению в порядке убывания (пузырьковая сортировка)
  for (int i = 0; i < 129; i++) {
    for (int j = i + 1; j < 130; j++) {
      if (sortedData[i].value < sortedData[j].value) {
        Point temp = sortedData[i];
        sortedData[i] = sortedData[j];
        sortedData[j] = temp;
      }
    }
  }

  // Поиск detectedPoints
  for (int i = 0; i < 130; i++) {
    // Проверка, соответствует ли точка условию яркости
    if ((float)sortedData[i].value * 100 / sortedData[0].value >= (100 - BRIGHTNESS_PERCENT)) {
      bool hasIntersections = false;
      // Проверка на пересечение с уже обнаруженными точками
      for (int j = 0; j < detectedCount; j++) {
        if (abs(detectedPoints[j].index - sortedData[i].index) < MIN_DISTANCE) {
          hasIntersections = true;
          break;
        }
      }
      // Если пересечений нет, добавляем точку в detectedPoints
      if (!hasIntersections) {
        detectedPoints[detectedCount++] = sortedData[i];
      }
    }
  }

  // Сортировка detectedPoints по индексу (пузырьковая сортировка)
  for (int i = 0; i < detectedCount - 1; i++) {
    for (int j = i + 1; j < detectedCount; j++) {
      if (detectedPoints[i].index > detectedPoints[j].index) {
        Point temp = detectedPoints[i];
        detectedPoints[i] = detectedPoints[j];
        detectedPoints[j] = temp;
      }
    }
  }

  for (int i = 0; i < detectedCount; i++) {
    Serial.print(detectedPoints[i].index);
    Serial.print(" ");
    Serial.println(detectedPoints[i].value);
  }

  // Расчет расстояний между соседними точками
  int distances[detectedCount - 1];
  for (int i = 0; i < detectedCount - 1; i++) {
    distances[i] = detectedPoints[i + 1].index - detectedPoints[i].index + 1;
  }

  // Расчет среднего расстояния
  int sumDistances = 0;
  for (int i = 0; i < detectedCount - 1; i++) {
    sumDistances += distances[i];
  }
  float d = (float)sumDistances / (detectedCount - 1);

  // Расчет результата
  float a = 1.0 / 80.0;  // ширина щели
  float L = 500.0;       // расстояние до экрана
  float result = (a * d / L) * 1000000.0;

  // Вывод результата в монитор порта
  Serial.print("Длина волны: ");
  Serial.println(result);

  digitalWrite(dirPin, LOW);
  for (int t = 0; t < length; t++) {
    for (int i = 0; i < stepsPerRevolution / 8; i++) {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(motorStepDelay);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(motorStepDelay);
    }
  }
}
