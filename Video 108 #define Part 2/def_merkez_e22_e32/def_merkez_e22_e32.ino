/*
** Öncelikle Hangi modül ile çalıştığınızı seçin e32 veya e22 arasında 
** Çalıştığınız modülün yanındaki "//" yoruma çek işaretini kaldırıp kullanmadığınız
** modülün başına "//" yazarak onu yoruma çekiniz. 
** https://shop.fixaj.com/
*/

#define e32433t20d        //e32 modülünü kullanıyorsanız burayı aktif edin
//#define e22900t22d      //e22 modülünü kullanıyorsanız burayı aktif edin

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
#define M0 4                    //  3in1 PCB mizde pin 7
#define M1 6                    //  3in1 PCB mizde pin 6
#define RX 18                   //  esp32 s3 de Lora RX e bağlı
#define TX 17                   //  esp32 s3 de Lora TX e bağlı

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

#define Adres 2              //0--65000 arası bir değer girebilirsiniz. Diğer Modüllerden FARKLI olmalı
#define Kanal 20             //Frekans değeri Diğer Modüllerle AYNI olmalı.
#define GonderilecekAdres 1  //Mesajın gönderileceği LoRa nın adresi
#define Netid 63             //0--65000 arası bir değer girebilirsiniz. Diğer Modüllerle AYNI olmalı.


struct Signal {
  char sifre[15] = "Fixaj.com";
  char konum[15];
  bool btn1;
  byte btn2[10];
  byte btn3[4];
} data;


void setup() {
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);

  Serial.begin(115200);
  while (!Serial) { ; }
  delay(200);
  FixajSerial.begin();

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
}

void loop() {
  while (FixajSerial.available() > 1) {

    ResponseStructContainer rsc = FixajSerial.receiveMessage(sizeof(Signal));
    struct Signal data = *(Signal*)rsc.data;
    rsc.close();

    if (strcmp(data.sifre, "Fixaj.com") == 0) {
      Serial.println("Sifre doğru");
      Serial.print("Konum: ");
      Serial.println(data.konum);
      Serial.print("buton durumu: ");
      Serial.println(data.btn1);
      Serial.print("Kordinat: ");
      Serial.println(*(float*)(data.btn2), 6);
      Serial.print("Büyük sayı: ");
      Serial.println(*(int*)(data.btn3));
    }
  }
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
  configuration.OPTION.fixedTransmission = FT_FIXED_TRANSMISSION;
  //configuration.OPTION.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;
  configuration.OPTION.wirelessWakeupTime = WAKE_UP_250;
  configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;

  // Ayarları KAYDET ve SAKLA
  ResponseStatus rs = FixajSerial.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  Serial.println(rs.getResponseDescription());
  Serial.println(rs.code);
  c.close();
}

#endif
