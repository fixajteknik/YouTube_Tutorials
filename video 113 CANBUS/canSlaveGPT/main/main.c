#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/twai.h"
#include <string.h>

// TWAI Genel Ayarları
#define TX_GPIO_NUM 1 // Tx pin numarası
#define RX_GPIO_NUM 2 // Rx pin numarası
#define LED 42
#define RS_CAN 10

// TWAI mesajı
#define EXAMPLE_TAG "TWAI Master"

/*
const twai_message_t message = {
    .identifier = 0x555, // Mesaj ID
    .data_length_code = 5,
    .data = {'h', 'e', 'l', 'l', 'o'}};
*/

void app_main()
{
    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    gpio_reset_pin(RS_CAN);
    gpio_set_direction(RS_CAN, GPIO_MODE_OUTPUT);

    gpio_set_level(RS_CAN, 0);

    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // TWAI sürücüsünü başlat
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_ERROR_CHECK(twai_start());

    /*
        const char *message = "mUSTAFA";
        twai_message_t twai_msg;
        twai_msg.identifier = 0x123;                 // Örnek bir tanımlayıcı
        twai_msg.data_length_code = strlen(message); // Mesaj uzunluğu
        memcpy(twai_msg.data, message, twai_msg.data_length_code);

    */

    uint32_t number = 145363;
    twai_message_t twai_msg;
    twai_msg.identifier = 0x123; // Örnek bir tanımlayıcı

    // Sayıyı byte array'ine dönüştürmek
    uint8_t data[4];
    data[0] = (number >> 24) & 0xFF; // En yüksek değerli byte
    data[1] = (number >> 16) & 0xFF;
    data[2] = (number >> 8) & 0xFF;
    data[3] = number & 0xFF; // En düşük değerli byte

    // Mesaj uzunluğunu ve veriyi ayarlamak
    twai_msg.data_length_code = sizeof(data);
    memcpy(twai_msg.data, data, twai_msg.data_length_code);
    
    while (true)
    {
        printf("mesaj gönderildi\n");
        ESP_ERROR_CHECK(twai_transmit(&twai_msg, pdMS_TO_TICKS(1000)));
        gpio_set_level(LED, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(LED, 0);
        vTaskDelay(pdMS_TO_TICKS(3000)); // 3 saniye bekle
    }
}
