/*
** Öncelikle Hangi modül ile çalıştığınızı seçin e32 veya e22 arasında 
** Çalıştığınız modülün yanındaki "//" yoruma çek işaretini kaldırıp kullanmadığınız
** modülün başına "//" yazarak onu yoruma çekiniz. 
** https://shop.fixaj.com/
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 4);
//#define e32433t20d        //e32 modülünü kullanıyorsanız burayı aktif edin
#define e22900t22d  //e22 modülünü kullanıyorsanız burayı aktif edin

#ifdef e22900t22d
#include "LoRa_E22.h"
#define kutuphane LoRa_E22
#else
#include "LoRa_E32.h"
#define kutuphane LoRa_E32
#endif

////////////////////////////////     KART TANIMLAMALARI    /////////////////////////////////////////////////////////////
#if CONFIG_IDF_TARGET_ESP32  // ESP 32
#include <HardwareSerial.h>
#define ESP32AYAR
#define M0 32  //3in1 PCB mizde pin 7
#define M1 33  //3in1 PCB mizde pin 6
#define RX 27  //3in1 PCB mizde pin RX
#define TX 35  //3in1 PCB mizde pin TX

HardwareSerial fixSerial(1);  //Serial biri seçiyoruz.
kutuphane FixajSerial(TX, RX, &fixSerial, UART_BPS_RATE_9600);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#elif CONFIG_IDF_TARGET_ESP32S3  // S3
#include <HardwareSerial.h>
#define M0 4   //  3in1 PCB mizde pin 7
#define M1 6   //  3in1 PCB mizde pin 6
#define RX 18  //  esp32 s3 de Lora RX e bağlı
#define TX 17  //  esp32 s3 de Lora TX e bağlı
#define BOOT0 0
#define ROLE 3
HardwareSerial fixSerial(1);
kutuphane FixajSerial(TX, RX, &fixSerial, UART_BPS_RATE_9600);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#elif defined ARDUINO_AVR_NANO  // NANO
#include <SoftwareSerial.h>
#define M0 7
#define M1 6
#define RX 3
#define TX 4
SoftwareSerial fixSerial(RX, TX);  //PCB versiyon 4.3 den sonra bu şekilde olmalı
kutuphane FixajSerial(&fixSerial);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#error Hedef CONFIG_IDF_TARGET tanimlanmamis
#endif

#define Adres 22             //0--65000 arası bir değer girebilirsiniz. Diğer Modüllerden FARKLI olmalı
#define Kanal 20             //Frekans değeri Diğer Modüllerle AYNI olmalı.
#define GonderilecekAdres 1  //Mesajın gönderileceği LoRa nın adresi
#define Netid 63             //0--65000 arası bir değer girebilirsiniz. Diğer Modüllerle AYNI olmalı.
#define loraSifre 2025
#define MESAJTEKRAR 3

struct Signal {
  char sifre[15] = "Fixaj.com";
  char konum[15] = "Grand Otel";
  char Nokta[16] = "KAT3, ODA 303";
  int id = 1;
  bool durum = false;
  bool reset = false;
  byte data[10] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A };
  byte extra[4] = { 0x0B, 0x0C, 0x0D, 0x0E };
} data;

bool AlarmDurumu = false;


void setup() {
  lcd.begin();
  lcd.backlight();
  lcd.print("Bolu Faciasi");
  lcd.setCursor(0, 1);
  lcd.print("Allah Rahmet Eylesin");

  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(BOOT0, INPUT_PULLUP);
  pinMode(ROLE, OUTPUT);  
  digitalWrite(ROLE, HIGH);

  Serial.begin(115200);
  delay(2000);
  FixajSerial.begin();
  delay(2000);

#ifdef e22900t22d
  LoraE22Ayarlar();
  Serial.print("E22 900t22d ");
#else
  LoraE32Ayarlar();
  Serial.print("E32 433t20d ");
#endif

  digitalWrite(M0, LOW);
  digitalWrite(M1, LOW);

  delay(500);
  Serial.println("ile başlıyoruz.. Fixaj.com");


  LCDyazAlarmOFF();
}

void loop() {

  if (digitalRead(BOOT0) == LOW) {
    Serial.println("Düğmeye basıldı!");  // Ekrana mesaj yazdır
    delay(1000);                         // Gereksiz tekrarı önlemek için bekle

    if (AlarmDurumu) {
      delay(4500);
      if (digitalRead(BOOT0) == LOW) {
        Serial.println("Alarm RESET");

        AlarmDurumu = false;
        data.durum = false;
        LCDyazAlarmOFF();

        for (int i = 0; i < MESAJTEKRAR; i++) {
          ResponseStatus rs = FixajSerial.sendMessage(&data, sizeof(Signal));
          // Check If there is some problem of succesfully send
          Serial.printf("mesaj no: %d \r\n", i);
          Serial.println(rs.getResponseDescription());
          delay(3000);
        }
      }
    } else {
      if (digitalRead(BOOT0) == LOW) {
        Serial.println("Yangın Alarmı!");

        AlarmDurumu = true;
        data.durum = true;
        LCDYazAlarmON(data);

        for (int i = 0; i < MESAJTEKRAR; i++) {
          ResponseStatus rs = FixajSerial.sendMessage(&data, sizeof(Signal));
          // Check If there is some problem of succesfully send
          Serial.printf("mesaj no: %d \r\n", i);
          Serial.println(rs.getResponseDescription());
          delay(3000);
        }
      }
    }
  }
  while (FixajSerial.available() > 10 && !AlarmDurumu) {

    Serial.println("Mesaj geldi");

    ResponseStructContainer rsc = FixajSerial.receiveMessageRSSI(sizeof(Signal));
    // Is something goes wrong print error
    if (rsc.status.code != 1) {
      Serial.println(rsc.status.getResponseDescription());
    } else {
      // Print the data received
      Serial.println(rsc.status.getResponseDescription());
      struct Signal data = *(Signal*)rsc.data;

      if (strcmp(data.sifre, "Fixaj.com") == 0) {
        // Diğer değerleri yazdırma
        Serial.printf("Konum: %s\r\n", data.konum);
        Serial.printf("Adres: %s\r\n", data.Nokta);
        Serial.printf("ID: %d\r\n", data.id);
        Serial.printf("Durum: %s\r\n", data.durum ? "Aktif" : "Pasif");

        if (data.durum) {
          LCDYazAlarmON(data);
        } else {
          LCDyazAlarmOFF();
        }
        Serial.printf("Reset Durumu: %s\r\n", data.reset ? "Aktif" : "Pasif");
        // data[] dizisini yazdırma
        Serial.print("Data: ");
        for (int i = 0; i < 10; i++) {
          Serial.printf("0x%02X ", data.data[i]);
        }
        Serial.println();
        // extra[] dizisini yazdırma
        Serial.print("Extra: ");
        for (int i = 0; i < 4; i++) {
          Serial.printf("0x%02X ", data.extra[i]);
        }

        //Serial.print("Kordinat: ");
        //Serial.println(*(float*)(data.btn2), 6);
        // Serial.print("Büyük sayı: ");
        //Serial.println(*(int*)(data.btn3));
        Serial.print("RSSI: ");
        Serial.println(rsc.rssi, DEC);
      } else {
        Serial.println("Şifre HATALI");
      }
    }
    rsc.close();
  }
}

void LCDYazAlarmON(struct Signal gelenveri) {
  Serial.println("Alarm devrede Röleler Switch ON");
  lcd.clear();
  lcd.print("Fixaj Alarm Sistemi");
  lcd.setCursor(0, 1);
  lcd.print("YANGIN ALARMI");
  lcd.setCursor(0, 2);
  lcd.print("OTEL: ");
  lcd.print(gelenveri.konum);
  lcd.setCursor(0, 3);
  lcd.print("Konum: ");
  lcd.print(gelenveri.Nokta);

  digitalWrite(ROLE, LOW);
}

void LCDyazAlarmOFF() {
  lcd.clear();
  lcd.print("Fixaj Alarm Sistemi");
  lcd.setCursor(0, 2);
  lcd.print("NORMAL");
  Serial.println("NORMAL");

  digitalWrite(ROLE, HIGH);
}


#ifdef e22900t22d
void LoraE22Ayarlar() {

  digitalWrite(M0, LOW);
  digitalWrite(M1, HIGH);

  ResponseStructContainer c;
  c = FixajSerial.getConfiguration();
  Configuration configuration = *(Configuration*)c.data;

  //DEĞİŞEBİLEN AYARLAR
  // Üstte #define kısmında ayarlayınız
  configuration.ADDL = lowByte(Adres);
  configuration.ADDH = highByte(Adres);
  configuration.NETID = Netid;
  configuration.CHAN = Kanal;

  //SEÇENEKLİ AYARLAR
  configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;  //Veri Gönderim Hızı 2,4 varsayılan
  //configuration.SPED.airDataRate = AIR_DATA_RATE_000_03;  //Veri Gönderim Hızı 0,3 En uzak Mesafe
  //configuration.SPED.airDataRate = AIR_DATA_RATE_111_625; //Veri Gönderim Hızı 62,5 En Hızlı

  configuration.OPTION.subPacketSetting = SPS_240_00;  //veri paket büyüklüğü 240 byte Varsayılan
  //configuration.OPTION.subPacketSetting = SPS_064_10; //veri paket büyüklüğü 64 byte
  //configuration.OPTION.subPacketSetting = SPS_032_11;  //veri paket büyüklüğü 32 en hızlısı

  configuration.OPTION.transmissionPower = POWER_22;  //Geönderim Gücü max Varsayılan
  //configuration.OPTION.transmissionPower = POWER_13;
  //configuration.OPTION.transmissionPower = POWER_10;  //Geönderim Gücü min

  //GELİŞMİŞ AYARLAR
  configuration.SPED.uartBaudRate = UART_BPS_9600;
  configuration.SPED.uartParity = MODE_00_8N1;
  configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
  configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;

  //configuration.TRANSMISSION_MODE.WORTransceiverControl = WOR_TRANSMITTER;
  configuration.TRANSMISSION_MODE.WORTransceiverControl = WOR_RECEIVER;

  configuration.TRANSMISSION_MODE.enableRSSI = RSSI_ENABLED;
  //configuration.TRANSMISSION_MODE.enableRSSI = RSSI_DISABLED;

  //configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
  configuration.TRANSMISSION_MODE.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;

  configuration.TRANSMISSION_MODE.enableRepeater = REPEATER_DISABLED;
  //configuration.TRANSMISSION_MODE.enableRepeater = REPEATER_ENABLED;

  configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
  // configuration.TRANSMISSION_MODE.enableLBT = LBT_ENABLED;

  configuration.CRYPT.CRYPT_H = highByte(loraSifre);
  configuration.CRYPT.CRYPT_L = lowByte(loraSifre);

  // Ayarları KAYDET ve SAKLA
  ResponseStatus rs = FixajSerial.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  Serial.println(rs.getResponseDescription());
  Serial.println(rs.code);
  c.close();
}

#else
void LoraE32Ayarlar() {
  digitalWrite(M0, HIGH);
  digitalWrite(M1, HIGH);

  ResponseStructContainer c;
  c = FixajSerial.getConfiguration();
  Configuration configuration = *(Configuration*)c.data;

  //DEĞİŞEBİLEN AYARLAR
  // Üstte #define kısmında ayarlayınız
  configuration.ADDL = lowByte(Adres);
  configuration.ADDH = highByte(Adres);
  configuration.CHAN = Kanal;

  //SEÇENEKLİ AYARLAR
  configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;  //Veri Gönderim Hızı 2,4 varsayılan
  //configuration.SPED.airDataRate = AIR_DATA_RATE_000_03;  //Veri Gönderim Hızı 0,3 En uzak Mesafe
  //configuration.SPED.airDataRate = AIR_DATA_RATE_101_192; //Veri Gönderim Hızı 19,2 En Hızlı

  configuration.OPTION.transmissionPower = POWER_20;  //Geönderim Gücü max Varsayılan
  //configuration.OPTION.transmissionPower = POWER_10;  //Geönderim Gücü min
  //configuration.OPTION.transmissionPower = POWER_30; // E32 30d modülleri için bunu aktif et

  //GELİŞMİŞ AYARLAR
  configuration.SPED.uartBaudRate = UART_BPS_9600;
  configuration.SPED.uartParity = MODE_00_8N1;
  configuration.OPTION.fec = FEC_0_OFF;
  //configuration.OPTION.fec = FEC_1_ON;
  //configuration.OPTION.fixedTransmission = FT_FIXED_TRANSMISSION;
  configuration.OPTION.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;  //Broadcast yayın için burayı aktif ediyoruz.
  configuration.OPTION.wirelessWakeupTime = WAKE_UP_250;
  configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;

  // Ayarları KAYDET ve SAKLA
  ResponseStatus rs = FixajSerial.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  Serial.println(rs.getResponseDescription());
  Serial.println(rs.code);
  c.close();
}

#endif