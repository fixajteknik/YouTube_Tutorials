#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include <WiFi.h>
#include "secret.h"
#include "ThingSpeak.h"     // always include thingspeak header file after other header files and custom macros
char ssid[] = SECRET_SSID;  // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password
int keyIndex = 0;           // your network key Index number (needed only for WEP)
WiFiClient client;
unsigned long myChannelNumber = SECRET_CH_ID;
const char *myWriteAPIKey = SECRET_WRITE_APIKEY;

#include <esp_sleep.h>

int number = 0;

#include <math.h>

#define DEFAULT_VREF 1100  // ADC referans voltajını kalibre edin
#define NO_OF_SAMPLES 128  // Çoklu okumalar için örnek sayısı
static esp_adc_cal_characteristics_t *adc_chars;
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;


#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 60       /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR static int bootCount = 0;

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0: Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1: Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER: Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP: Serial.println("Wakeup caused by ULP program"); break;
    default: Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}


static void check_efuse(void) {
  // Check TP is burned into eFuse
  if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
    // printf("eFuse Two Point: Supported\n");
  } else {
    //printf("eFuse Two Point: NOT supported\n");
  }
  // Check Vref is burned into eFuse
  if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
    //printf("eFuse Vref: Supported\n");
  } else {
    //printf("eFuse Vref: NOT supported\n");
  }
}

static void print_char_val_type(esp_adc_cal_value_t val_type) {
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
    //printf("Characterized using Two Point Value\n");
  } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
    //printf("Characterized using eFuse Vref\n");
  } else {
    //printf("Characterized using Default Vref\n");
  }
}

float correction_function(float input) {
  // Giriş ve çıkış değerlerini eşleştiren bir dizi kontrol noktası tanımlayın
  const float points[][8] = {
    { 0.0f, 0.0f },
    { 0.012f, 0.30f },
    { 0.291f, 0.58f },
    { 3.2f, 3.2f },  // Giriş 3.2 ise çıkış 3.2
    { 5.313f, 5.212f },
    { 14.342f, 13.79f },
    { 25.801f, 24.46f },
    { 32.809f, 30.95f }  // Giriş 5.313 ise çıkış 5.212
                         // Daha fazla kontrol noktası eklenebilir...
  };

  // Giriş değeri kontrol noktalarından birine çok yakınsa, doğrudan eşleşen çıkışı döndürün
  for (int i = 0; i < sizeof(points) / sizeof(points[0]); ++i) {
    if (fabs(input - points[i][0]) < 0.001f) {
      return points[i][1];
    }
  }

  // Giriş değeri kontrol noktaları arasındaysa, lineer interpolasyon yapın
  for (int i = 1; i < sizeof(points) / sizeof(points[0]); ++i) {
    if (input < points[i][0]) {
      float ratio = (input - points[i - 1][0]) / (points[i][0] - points[i - 1][0]);
      return points[i - 1][1] + ratio * (points[i][1] - points[i - 1][1]);
    }
  }

  // Giriş değeri en yüksek kontrol noktasından büyükse, oranı artırarak son kontrol noktasını kullanın
  // Bu, belirli bir oran artışını varsayar - bu oranı ayarlamanız gerekebilir
  float last_input = points[sizeof(points) / sizeof(points[0]) - 1][0];
  float last_output = points[sizeof(points) / sizeof(points[0]) - 1][1];
  if (input > last_input) {
    float ratio = (input - last_input) / last_input;
    return last_output + (last_output * ratio * 0.1f);  // Örneğin, %10 oranında artış
  }

  // Eğer hiçbir koşula uymuyorsa, girişi olduğu gibi döndür
  return input;
}

bool wifiCon() {

  // Connect or reconnect to WiFi
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    Serial.println("\nConnected.");
    return true;
  } else {
    WiFi.begin(ssid, pass);
    Serial.print("bağlanıyor...");
    delay(5000);
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(SECRET_SSID);
      Serial.println("\nConnected.");
      return true;
    }
  }
  Serial.println("\n NOT Connected.");
  return false;
}

void setup() {
  Serial.begin(115200);
 // while (!Serial) { ; } //usb pc bağlantısını sonsuza kadar bekler takılı kalabilir
  delay(2000);

  // Check if Two Point or Vref are burned into eFuse
  check_efuse();

  // Configure ADC
  if (unit == ADC_UNIT_1) {
    adc1_config_width(width);
    // ADC çözünürlüğünü 12 bit olarak ayarla
    analogReadResolution(12);

    // Giriş zayıflatmasını (attenuation) 11 dB olarak ayarla
    analogSetAttenuation(ADC_11db);  // 0 ile 3.3V arası

    // Characterize ADC
    //adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));

    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
  }



  // WiFi bağlantısı başlatılıyor
  Serial.println("WiFi'ya baglaniyor...");

  wifiCon();

  if (WiFi.status() == WL_CONNECTED) {
    // Bağlantı kurulduğunda IP adresi yazdırılıyor
    Serial.println("");
    Serial.println("WiFi'ya baglandi.");
    Serial.print("IP Adresi: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi' YOK.");
  }

  ThingSpeak.begin(client);  // Initialize ThingSpeak

  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
}

void loop() {

  //Print the wakeup reason for ESP32
  print_wakeup_reason();
  uint32_t adc_reading = 0;
  // Multisampling
  for (int i = 0; i < NO_OF_SAMPLES; i++) {
    if (unit == ADC_UNIT_1) {
      adc_reading += analogRead(1);
      vTaskDelay(1);
    }
  }
  adc_reading /= NO_OF_SAMPLES;
  // Convert adc_reading to voltage in mV
  uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
  // Çarpma işlemini tam sayı olarak yapın ve sonucu float'a dönüştürün
  float scaled_voltage = (voltage * 11.622f) / 1000.0f;  // 11.622 ile çarpıp 1000'e bölme işlemi

  printf("Raw: %lu\tScaled Voltage: %.3f V\n", adc_reading, scaled_voltage);
  scaled_voltage = correction_function(scaled_voltage);
  printf("Raw: %lu\tScaled Voltage 3.2 : %.3f V\n", adc_reading, scaled_voltage);

  if (wifiCon()) {
    int x = ThingSpeak.writeField(myChannelNumber, 6, scaled_voltage, myWriteAPIKey);
    if (x == 200) {
      Serial.println("Channel update successful.");
    } else {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
  }


  Serial.println("Going to sleep now");
  vTaskDelay(1000);
  //Serial.flush();

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  esp_deep_sleep_start();
 // Serial.println("This will never be printed");
}
