/*!
Bu deneme uygulamasında E70 Lora modülünün parametre ayarlarını değişebilir
ve deneme amaçlı 2 modül arasında haberleşme sağlayabilirsinizi.
 
deneme amaçlı mesajlaşma atılırken RSSI değeri de gelecektir.
RSSI: Sintal gücünü göstermektedir. maximum e70 için 235 dir.
 
Bu kod gönderici (node) Tarafındaki ESP32 ye yüklenmelidir.
 
Mehmet YILDIZ 2023
https://fixaj.com/
 
*/

#include <HardwareSerial.h>
#define M0 32  //  3in1 PCB mizde pin 7
#define M1 33  //  3in1 PCB mizde pin 6
#define RX 27  //  Lora nın 3. pini RX in ESP32 de hangi pine bağlı olduğu
#define TX 35  //  Lora nın 4. pini TX in ESP32 de hangi pine bağlı olduğu

bool Ayarmode = true;  //true yapılınca lora modülünün parametre ayarları değişerek işe başlar
                       //false a getirilirse ayar vermeden direk mesajlajmayı başlatır.
                       //tavsiyem true da kalmasıdır. Her başlangıçta modülün parametre ayarlarınıda
                       //kontrol etmiş olursunuz bu şekilde.

HardwareSerial fixajSerial(1);

unsigned long kanalBekleme_sure = 0;
int kanalBekleme_bekleme = 3000;

unsigned long kanalBekleme_sure_RESTART = 0;
unsigned long kanalBekleme_bekleme_RESTART = 100000;  //node ağa 60sn içinde bağlanamazsa
                                                      //esp kendine reset atıyor. Birnevi watchdog gibi

byte e = 0;
String receivedData = "";

struct Signal {
  char type[30];
  byte temp[4];
} mesaj;

//............AYARLAR..............
const char *ayarlar2[] = { "AT+WMCFG=1",      // 0 coordinatör, 1 Node, 4 default
                           "AT+PIDCFG=1453",  //ağ şifresi
                           "AT+CNCFG=0",      //the channels are only 0,1,2,3,4,5,6 and 128.
                           "AT+TFOCFG=3",     // 0 boardcast gibi, 1 data+long adres, 3 data+ RSSI
                           "AT+TFICFG=0",     //0 input broadcast, 1 short adres + data, 2 long adres + data
                           "AT+RSCFG=63",     //Node ağa bağlanmazsa yeniden başlat (80-60=20sn sonra)
                           "AT+IOCFG=1",      //push pull 0 yaparsanız enerji verimlili adına ledleri kapatır.
                           "AT+PWCFG=1",      //iletim gücü 30dbm, 0 yapılınca maximum ama uzun kullandım modüle zararlı
                           "AT+TLCFG=3",      //güç tüketim modu maximum, default 2
                           "AT+TMCFG=0",      //Long Range Haberleşme
                           "AT+RSTART" };


void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB
  }
  delay(2000);

  fixajSerial.begin(115200, SERIAL_8N1, TX, RX);
  while (!fixajSerial) {
    ;  // wait for serial port to connect. Needed for native USB
  }
  delay(2000);

  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  digitalWrite(M0, 1);  // ayar modu , Uyku modu
  digitalWrite(M1, 1);
  delay(3000);
  Serial.println("uyku modu");
  pinMode(14, OUTPUT);  //led boşuna yanmasın diye, esp32 bug
  while (fixajSerial.available()) {
    fixajSerial.read();  //buffer ı boşalt
  }
  /*
  burada mantık şu önce mevcut ayarları getiriyor..
  sonra istenilen şekilde parametre ayarları yeniden veriliyor..
  sonra yeni ayarları ekrana yazdırıyor, verdiğiniz ayarlar sağlıklı şekilde kaydedilmiş mi
  kontrol edesiniz diye. Sonrasında Loop içerisinde mesajlaşma ve dinleme başlıyor.
  */

  //ayarDegis();
  if (ayarDegis2()) {
    Serial.println("Ayarlar doğru bir şekilde kayıt edildi.");
  } else {
    Serial.println("HATA - Ayarlar kaydedilemedi.");
  }
  mevcutAyarlar();

  digitalWrite(M0, 1);  //  coordinator mode m0=0, m1=0  |  normal mode m0=1, m1=0
  digitalWrite(M1, 0);
  delay(2000);
  // RESET();       //E70 parametre ayarlarını sıfırlar.

  Serial.println("Fixaj.com baslat...TEST OKUMA Node");
  delay(100);
}

void loop() {

  if (millis() > kanalBekleme_sure + kanalBekleme_bekleme) {
    kanalBekleme_sure = millis();
    e++;
    strcpy(mesaj.type, "Fixaj node 2");
    *((int *)mesaj.temp) = e;
    gonder(&mesaj);

    Serial.println("Mesaj Gönderildi");
  }

  while (fixajSerial.available()) {
    receive(&mesaj);
    Serial.print("type: ");
    Serial.println(mesaj.type);
    Serial.print("değer: ");
    Serial.println(*((int *)mesaj.temp));

    Serial.print("RSSI: ");
    while(fixajSerial.available()){
      Serial.print(fixajSerial.read(), DEC);
    }
    Serial.println(".");

    kanalBekleme_sure_RESTART = millis();  // yeniden başlatmayı durdur
  }

  //bazen node lar ağa bağlanamıyor 60sn sonra esp hard reset yapıcak.

  if (millis() > kanalBekleme_sure_RESTART + kanalBekleme_bekleme_RESTART) {
    kanalBekleme_sure_RESTART = millis();

    fixajSerial.print("++");
    delay(100);
    fixajSerial.print("+++");  // Ayarlar ekranına giriş
    delay(100);
    fixajSerial.println("AT+RSTART");
    delay(5000);

    // Restart ESP
    ESP.restart();

    Serial.println("ESP Yeniden Baslatiliyor...");
  }
}

/*
ayarDegis fonksiyonu parametre ayarlarını nasıl değiştireceğinizi 
gösterir. Örnek olarak bazı ayarlar yapılmıştır.
*/
/*
void ayarDegis() {
  if (Ayarmode) {
    fixajSerial.print("++");
    delay(100);
    fixajSerial.print("+++");  // Ayarlar ekranına giriş
    delay(100);
    fixajSerial.println("AT+WMCFG=1");  // 0 coordinatör, 1 Node, 4 default
    delay(100);
    fixajSerial.println("AT+PIDCFG=1453");  //ağ şifresi
    delay(100);
    fixajSerial.println("AT+CNCFG=0");  //the channels are only 0,1,2,3,4,5,6 and 128.
    delay(100);
    fixajSerial.println("AT+TFOCFG=3");  // 0 boardcast gibi, 1 data+long adres, 3 data+ RSSI
    delay(100);
    fixajSerial.println("AT+TFICFG=0");  //0 input broadcast, 1 short adres + data, 2 long adres + data
    delay(100);
    fixajSerial.println("AT+RSCFG=100");  //Node ağa bağlanmazsa yeniden başlat (80-60=20sn sonra)
    delay(100);
    fixajSerial.println("AT+IOCFG=1");  //push pull 0 yaparsanız enerji verimlili adına ledleri kapatır.
    delay(100);
    fixajSerial.println("AT+PWCFG=1");  //iletim gücü 30dbm, 0 yapılınca maximum ama uzun kullandım modüle zararlı
    delay(100);
    fixajSerial.println("AT+TLCFG=3");  //güç tüketim modu maximum, default 2
    delay(100);
    fixajSerial.println("AT+TMCFG=0");  //Long Range Haberleşme
    delay(100);

    fixajSerial.println("AT+RSTART");

    while (fixajSerial.available()) {
      Serial.write(fixajSerial.read());
    }

    Serial.println("Ayarlar degisti, Mevcut Ayarlar Getiriliyor..");
    delay(3000);
    mevcutAyarlar();
  }
}
*/
/*
mevcutAyarlar fonksiyonu E70 modülünün üstünde kayıtlı olan mevcut ayarlarını
ekrana yazdırır.
*/
void mevcutAyarlar() {
  // Ayarlar ekranına giriş
  fixajSerial.print("+");
  delay(100);
  fixajSerial.print("+++");
  delay(100);

  const char *ayarlar[] = { "AT+CNCFG=?",
                            "AT+WMCFG=?",
                            "AT+PWCFG=?",
                            "AT+TFOCFG=?",
                            "AT+TFICFG=?",
                            "AT+TMCFG=?",
                            "AT+PIDCFG=?",
                            "AT+DMCFG=?",
                            "AT+RSCFG=?",
                            "AT+UBCFG=?",
                            "AT+UPCFG=?",
                            "AT+IOCFG=?",
                            "AT+VER",
                            "AT+TLCFG=?",
                            "AT+DINFO=SELFS",
                            "AT+DINFO=SELFE" };

  const char *ayarlartr[] = { "Frekans Seçenekleri [Kanal]: 0,1,2,3,4,5,6 and 128 (her frekans 14s de olmayabilir) ,",
                              "Mod: 0- coordinatör, 1- Node, 4- default, ",
                              "Sinyal Gücü: 0- En yğksek [31.5 dbm ] Önerilen 1 olması [30dbm]",
                              "Gelen verinin Formatı:0-data, 1- data+ID adres, 3- data+RSSI, 5- Data+ID Adres+RSSI",
                              "Giden Verinin Formatı: 0- Broadcast, 2- ID adres+data",
                              "Gönderim Modu: 0- LORA, 1- GFSK, 0 olmalı",
                              "Ag sifresi 0-65535 arasi",
                              "Uyku modu aralığı 0-60 arası",
                              "Eğer node her hangi bir ağa bağlanmaz ise kendini belirli süre sonra resetlesin (süre 60 dan büyük olmalı)",
                              "Baud rate: 7-115200",
                              "Parity",
                              "0-push pull 1-open drain",
                              "Modülün Versiyonu",
                              "concurrency performans 0-low 1,2,3-highest (aynı anda veri gönderen nokta çok ise high yapın, enerji tüketimi artar)",
                              "Ağ daki sırası, bir ağa bağlı ise",
                              "Uzun adres yada id Adres bu adres her çipe özeldir değişmez" };

  for (int i = 0; i < sizeof(ayarlar) / sizeof(ayarlar[0]); i++) {

    fixajSerial.println(ayarlar[i]);
    delay(100);

    while (fixajSerial.available()) {
      Serial.write(fixajSerial.read());
    }

    Serial.println(ayarlartr[i]);
    Serial.println("");
  }

  fixajSerial.println("AT+EXIT");
  delay(200);
}

bool ayarDegis2() {
  // Ayarlar ekranına giriş
  fixajSerial.print("+");
  delay(100);
  fixajSerial.print("+++");
  delay(100);

  while (fixajSerial.available()) {
    receivedData = fixajSerial.readStringUntil('\n');  // Satır sonuna kadar olan veriyi oku
    receivedData.trim();
    if (receivedData == "Enter AT Mode") {
      Serial.println("AT Moduna Giriş Yapıldı");


      for (int i = 0; i < sizeof(ayarlar2) / sizeof(ayarlar2[0]); i++) {

        fixajSerial.println(ayarlar2[i]);
        delay(200);

        while (fixajSerial.available()) {
          Serial.write(fixajSerial.read());
        }
      }
      delay(5000);  // e70 yeniden başlatmak için gerekli.
      return true;
    }

    else {
      Serial.println("HATA- AT Modu Girişi Bekleniyor...");
      Serial.println(receivedData);
    }
  }
  fixajSerial.println("AT+EXIT");
  delay(100);

  return false;  //parametre ayarları verilemediği için ağa bağlanma hatası olacaktır.
}

void RESET() {
  if (Ayarmode) {
    fixajSerial.print("+");
    delay(100);
    fixajSerial.print("+++");  // Ayarlar ekranına giriş
    delay(100);
    fixajSerial.println("AT+DFCFG");  // Fabrika ayarları geri getir
    delay(100);

    Serial.println("Reset Atildi, Mevcut Ayarlar Getiriliyor..");
    delay(3000);
    mevcutAyarlar();
  }
}

void gonder(Signal *mdata) {
  fixajSerial.write((uint8_t *)mdata, sizeof(Signal));
}

bool receive(Signal *mdata) {
  return (fixajSerial.readBytes((uint8_t *)mdata, sizeof(Signal)) == sizeof(Signal));
}