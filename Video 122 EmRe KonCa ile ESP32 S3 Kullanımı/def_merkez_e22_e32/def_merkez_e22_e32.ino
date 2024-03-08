/*
** Öncelikle Hangi modül ile çalıştığınızı seçin e32 veya e22 arasında 
** Çalıştığınız modülün yanındaki "//" yoruma çek işaretini kaldırıp kullanmadığınız
** modülün başına "//" yazarak onu yoruma çekiniz. 
** Arduino IDE Seçilen karta göre otamatik PCB seçilecektir. Seçilen PCB ye göre
** Pin tanımlamaları otomatik gelecektir.
** https://shop.fixaj.com/
** for overseas sales
** https://store.fixaj.com/
*/

#define LoRa_E32_DEBUG  //debug mesajları ekrana yazdırmak için gerekli

#define e32433t20d  //e32 modülünü kullanıyorsanız burayı aktif edin
//#define e22900t22d      //e22 modülünü kullanıyorsanız burayı aktif edin

#ifdef e22900t22d
#include "LoRa_E22.h"
#define kutuphane LoRa_E22
#define Netid 63  //0--65000 arası bir değer girebilirsiniz. Diğer Modüllerle AYNI olmalı.
#else
#include "LoRa_E32.h"
#define kutuphane LoRa_E32
#endif

enum mPCB {
  s3,
  esp32,
  nano
};

/*
Kart tanımlarımını değiştirmeyiniz.
*/

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
mPCB selectedOption = esp32;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#elif CONFIG_IDF_TARGET_ESP32S3  // S3
#include <HardwareSerial.h>
#define M0 4                    //  3in1 PCB mizde pin 7
#define M1 6                    //  3in1 PCB mizde pin 6
#define RX 18                   //  esp32 s3 de Lora RX e bağlı
#define TX 17                   //  esp32 s3 de Lora TX e bağlı

HardwareSerial fixSerial(1);
kutuphane FixajSerial(TX, RX, &fixSerial, UART_BPS_RATE_9600);
mPCB selectedOption = s3;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#elif defined ARDUINO_AVR_NANO  // NANO
#include <SoftwareSerial.h>
#define M0 7
#define M1 6
#define RX 3
#define TX 4
SoftwareSerial fixSerial(RX, TX);  //PCB versiyon 4.3 den sonra bu şekilde olmalı
kutuphane FixajSerial(&fixSerial);
mPCB selectedOption = nano;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#error Hedef CONFIG_IDF_TARGET tanimlanmamis
#endif

//BURADAN SONRA KOD DA İSTEDİĞİNİZ DEĞİŞİKLİĞİ YAPABİLİRSİNİZ

#define Adres 2              //0--65000 arası bir değer girebilirsiniz. Diğer Modüllerden FARKLI olmalı
#define Kanal 20             //Frekans değeri Diğer Modüllerle AYNI olmalı.
#define GonderilecekAdres 1  //Mesajın gönderileceği LoRa nın adresi

struct Signal {
  char sifre[15];
  char konum[15];
  bool btn1;
  byte btn2[10];
  byte btn3[4];
} data;

#define RolePin 3
#define BuzzerPin 21

void setup() {
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);

  pinMode(RolePin, OUTPUT);
  pinMode(BuzzerPin, OUTPUT);

  Serial.begin(115200);
  delay(2500);
  FixajSerial.begin();

#ifdef e22900t22d
  LoraE22Ayarlar();
  DEBUG_PRINT("E22 900t22d ");
#else
  LoraE32Ayarlar();
  DEBUG_PRINT("E32 433t20d ");
#endif

  digitalWrite(M0, LOW);
  digitalWrite(M1, LOW);

  delay(500);
  DEBUG_PRINTLN("ile başlıyoruz.. Fixaj.com");

  switch (selectedOption) {
    case s3:
      DEBUG_PRINTLN("S3 Board Seçildi");
      break;
    case esp32:
      DEBUG_PRINTLN("ESP32 3in1 PCB Seçildi");
      break;
    case nano:
      DEBUG_PRINTLN("NANO 3in1 PCB Seçildi");
      break;
    default:
      DEBUG_PRINTLN("Henüz Geliştirmediğimiz bir PCB Seçildi");
      break;
  }
}

void loop() {
  while (FixajSerial.available() > 1) {

    ResponseStructContainer rsc = FixajSerial.receiveMessage(sizeof(Signal));
    struct Signal data = *(Signal*)rsc.data;
    rsc.close();

    if (strcmp(data.sifre, "Fixaj.com") == 0) {
      DEBUG_PRINTLN("Sifre doğru");
      DEBUG_PRINT("id: ");
      DEBUG_PRINT(*(int*)(data.btn3));
      DEBUG_PRINT(" Konum: ");
      DEBUG_PRINT(data.konum);
      DEBUG_PRINT(", buton durumu: ");
      DEBUG_PRINTLN(data.btn1);

      if (data.btn1) {
        digitalWrite(RolePin, LOW);
        //digitalWrite(BuzzerPin, HIGH);
        //delay(1000);
        //digitalWrite(BuzzerPin, LOW);
      } else {
        digitalWrite(RolePin, HIGH);
        digitalWrite(BuzzerPin, LOW);
      }
    }
  }
}

// PARAMETRE AYARLARI VERME FONKSİYONLARI
// FİXAJ PCBLERİ KULLANIYORSANIZ USB STİCK GEREK YOK BU FONKSİYON LORA PARAMETRE
// AYARLARINI VERECEKTİR.


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

  //configuration.OPTION.transmissionPower = POWER_22;  //Geönderim Gücü max Varsayılan
  configuration.OPTION.transmissionPower = POWER_13;
  //configuration.OPTION.transmissionPower = POWER_10;  //Geönderim Gücü min

  //GELİŞMİŞ AYARLAR
  configuration.SPED.uartBaudRate = UART_BPS_9600;
  configuration.SPED.uartParity = MODE_00_8N1;
  configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
  configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;

  //configuration.TRANSMISSION_MODE.WORTransceiverControl = WOR_TRANSMITTER;
  configuration.TRANSMISSION_MODE.WORTransceiverControl = WOR_RECEIVER;

  //configuration.TRANSMISSION_MODE.enableRSSI = RSSI_ENABLED;
  configuration.TRANSMISSION_MODE.enableRSSI = RSSI_DISABLED;

  configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
  //configuration.TRANSMISSION_MODE.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;

  configuration.TRANSMISSION_MODE.enableRepeater = REPEATER_DISABLED;
  //configuration.TRANSMISSION_MODE.enableRepeater = REPEATER_ENABLED;

  configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
  // configuration.TRANSMISSION_MODE.enableLBT = LBT_ENABLED;

  // Ayarları KAYDET ve SAKLA
  ResponseStatus rs = FixajSerial.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  DEBUG_PRINTLN(rs.getResponseDescription());
  DEBUG_PRINTLN(rs.code);
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

  //configuration.OPTION.transmissionPower = POWER_20;  //Geönderim Gücü max Varsayılan
  configuration.OPTION.transmissionPower = POWER_10;  //Geönderim Gücü min
  //configuration.OPTION.transmissionPower = POWER_30; // E32 30d modülleri için bunu aktif et

  //GELİŞMİŞ AYARLAR
  configuration.SPED.uartBaudRate = UART_BPS_9600;
  configuration.SPED.uartParity = MODE_00_8N1;
  configuration.OPTION.fec = FEC_0_OFF;
  //configuration.OPTION.fec = FEC_1_ON;
  configuration.OPTION.fixedTransmission = FT_FIXED_TRANSMISSION;
  //configuration.OPTION.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;
  configuration.OPTION.wirelessWakeupTime = WAKE_UP_250;
  configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;

  // Ayarları KAYDET ve SAKLA
  ResponseStatus rs = FixajSerial.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  DEBUG_PRINTLN(rs.getResponseDescription());
  DEBUG_PRINTLN(rs.code);
  c.close();
}

#endif
