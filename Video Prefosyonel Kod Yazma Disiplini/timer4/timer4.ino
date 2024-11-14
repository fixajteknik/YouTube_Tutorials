hw_timer_t* timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;

hw_timer_t* timer2 = NULL;
volatile SemaphoreHandle_t timerSemaphore2;

hw_timer_t* timer3 = NULL;
volatile SemaphoreHandle_t timerSemaphore3;

hw_timer_t* timer4 = NULL;
volatile SemaphoreHandle_t timerSemaphore4;


void ARDUINO_ISR_ATTR onTimer() {
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
}

void ARDUINO_ISR_ATTR onTimer2() {
  xSemaphoreGiveFromISR(timerSemaphore2, NULL);
}

void ARDUINO_ISR_ATTR onTimer3() {
  xSemaphoreGiveFromISR(timerSemaphore3, NULL);
}

void ARDUINO_ISR_ATTR onTimer4() {
  xSemaphoreGiveFromISR(timerSemaphore4, NULL);
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  timerSemaphore = xSemaphoreCreateBinary();
  timer = timerBegin(1000000);
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 1000000, true, 0);

  timerSemaphore2 = xSemaphoreCreateBinary();
  timer2 = timerBegin(1000000);
  timerAttachInterrupt(timer2, &onTimer2);
  timerAlarm(timer2, 2000000, true, 0);

  timerSemaphore3 = xSemaphoreCreateBinary();
  timer3 = timerBegin(1000000);
  timerAttachInterrupt(timer3, &onTimer3);
  timerAlarm(timer3, 3000000, true, 0);

  timerSemaphore4 = xSemaphoreCreateBinary();
  timer4 = timerBegin(1000000);
  timerAttachInterrupt(timer4, &onTimer4);
  timerAlarm(timer4, 5000000, true, 0);
}

void loop() {
  
  if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE) {
    Serial.println("timerSemaphore 1sn de ");
  }

  if (xSemaphoreTake(timerSemaphore2, 0) == pdTRUE) {
    Serial.println("timerSemaphore 2sn de ");
  }

  if (xSemaphoreTake(timerSemaphore3, 0) == pdTRUE) {
    Serial.println("timerSemaphore 3sn de ");
  }

  if (xSemaphoreTake(timerSemaphore4, 0) == pdTRUE) {
    Serial.println("timerSemaphore 5sn de ");
  }

}