/* UART asynchronous example, that uses separate RX and TX tasks

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"

static const int RX_BUF_SIZE = 1024;

#define TXD_PIN (GPIO_NUM_18)
#define RXD_PIN (GPIO_NUM_17)
#define M0 (GPIO_NUM_4)
#define M1 (GPIO_NUM_6)

void mevcutAyarlar(); // Fonksiyon prototipi

void init(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}
/*
int sendData(const char *logName, const char *data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}

*/
/*
static void tx_task(void *arg)
{
    static const char *TX_TASK_TAG = "TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    while (1) {
        sendData(TX_TASK_TAG, "Hello world");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
*/

static void tx_task(void *arg) //fonksiyon adı neden static çünkü başka dosyalardan çağırlamasın bu fonksiyon ile aynı isimde varsa karışmasın
{
    static const char *TX_TASK_TAG = "TX_TASK"; //neden pointer var? çünkü C de car array oluşturmak için. sadece string değil, string belleği yorar
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    while (1)
    {
        mevcutAyarlar();                        // AT komutlarını ve açıklamalarını gönder
        vTaskDelay(10000 / portTICK_PERIOD_MS); // 10 saniye bekle
    }
}

void mevcutAyarlar()
{

    uart_write_bytes(UART_NUM_1, "+", 1); //Arduino daki serial.write ile aynı
    vTaskDelay(100 / portTICK_PERIOD_MS);
    uart_write_bytes(UART_NUM_1, "+++", 3);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    const char *ayarlar[] = {"AT+CNCFG=?",
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
                             "AT+DINFO=SELFE"};

    const char *ayarlartr[] = {"Frekans Seçenekleri [Kanal]: 0,1,2,3,4,5,6 and 128 (her frekans 14s de olmayabilir) ,",
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
                               "Uzun adres yada id Adres bu adres her çipe özeldir değişmez"};

    for (int i = 0; i < sizeof(ayarlar) / sizeof(ayarlar[0]); i++)
    {
        uart_write_bytes(UART_NUM_1, ayarlar[i], strlen(ayarlar[i]));
        uart_write_bytes(UART_NUM_1, "\n", 1);
        vTaskDelay(100 / portTICK_PERIOD_MS);

        // Read data from UART.
        static const char *RX_TASK_TAG = "\n\nRX_TASK";
        const uart_port_t uart_num = UART_NUM_1;
        uint8_t data[256];
         int length = 0;
        ESP_ERROR_CHECK(uart_get_buffered_data_len(uart_num, (size_t *)&length)); //Serial.avaliable gibi STM32 de durum biraz farklı
        length = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS); //tek seferde buffer da ne varsa okur ve data arrayı ne saklar serial.read gibi tek okuma yapmaz.
        if (length > 0) 
        {
            data[length] = 0; //C de yazıldığı da string ekrana yazdırmak için son karaktere '\0' atılır
            ESP_LOGI(RX_TASK_TAG, "Read  %s", data);
            // ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
        }
        uart_flush(uart_num); //rx bufferı boşalt hiç bir şey kalmasın

        ESP_LOGI("AYARLAR", "%s", ayarlartr[i]);
        // vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    uart_write_bytes(UART_NUM_1, "AT+EXIT\n", strlen("AT+EXIT\n"));
    vTaskDelay(200 / portTICK_PERIOD_MS);
}
/*
static void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t *data = (uint8_t *)malloc(RX_BUF_SIZE + 1);
    while (1)
    {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS);
        if (rxBytes > 0)
        {
            data[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            // ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
        }
    }
    free(data);
}
*/
void app_main(void)
{

    gpio_reset_pin(M0);
    gpio_set_direction(M0, GPIO_MODE_OUTPUT);
    gpio_set_level(M0, 1);

    gpio_reset_pin(M1);
    gpio_set_direction(M1, GPIO_MODE_OUTPUT);
    gpio_set_level(M1, 1);

    init();
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    //  xTaskCreate(rx_task, "uart_rx_task", 1024 * 4, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(tx_task, "uart_tx_task", 1024 * 4, NULL, configMAX_PRIORITIES - 1, NULL); //task stackover flow olmaması için *4 bellek arttırıldı.
}
