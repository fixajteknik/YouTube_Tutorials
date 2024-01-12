#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/twai.h"

// TWAI Genel Ayarları
#define TX_GPIO_NUM 1 // Tx pin numarası
#define RX_GPIO_NUM 2 // Rx pin numarası
#define LED 42
#define RS_CAN 10

void app_main()
{

    gpio_reset_pin(42);
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

    twai_message_t rx_message;
    while (true)
    {
        // Mesaj alındığında
        if (twai_receive(&rx_message, pdMS_TO_TICKS(1000)) == ESP_OK)
        {
            if (rx_message.dlc_non_comp)
            {
                printf("The message's data length is larger than 8.\n");
            }
            else
            {
                printf("The message's data length is 8 or less.\n");
            }

            if (rx_message.identifier == 0x123)
            { // Check if identifier is 0x123
                gpio_set_level(LED, 1);
                vTaskDelay(pdMS_TO_TICKS(100));
                gpio_set_level(LED, 0);
                // printf("Received message: %.*s\n", rx_message.data_length_code, rx_message.data);

                uint32_t received_number = 0;
                received_number |= ((uint32_t)rx_message.data[0] << 24);
                received_number |= ((uint32_t)rx_message.data[1] << 16);
                received_number |= ((uint32_t)rx_message.data[2] << 8);
                received_number |= (uint32_t)rx_message.data[3];
                printf("Alınan Sayı: %lu\n", received_number);
            }
            else
            {
                printf("HATA- Received message identifier: %lu\n", rx_message.identifier);
            }
        }
    }
}
