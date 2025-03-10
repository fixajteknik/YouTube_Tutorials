/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* ULP riscv DS18B20 1wire temperature sensor example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include "esp_sleep.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"
#include "soc/rtc_periph.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "ulp_riscv.h"
#include "ulp_main.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[] asm("_binary_ulp_main_bin_end");

static void init_ulp_program(void);

void app_main(void)
{
    /**
     * usb-jtag da arada bir buffer olmadığı, direk pc ile esp32 bağlı olduğu için
     * arada bir bekleme olmaz ise uyanınca seriporta bir mesaj yazamıyor
     * 1sn main cpu kendine gelmesini bekliyorum.
     * chatgpt ve gemini ye göre en doğru bekleme yöntemi pdMS şeklinde olanmış
    */
    vTaskDelay(pdMS_TO_TICKS(1000));
    /**
     * pin tanımlamasını ulp tarafında yaptım belki burada da çalışırdı ama
     * ulp tarafında enable komutu var burada yok
     * ulp_riscv_gpio_output_enable(GPIO_NUM_4); 
    */

    /* Initialize selected GPIO as RTC IO, enable input, disable pullup and pulldown
    rtc_gpio_init(GPIO_NUM_4);
    rtc_gpio_set_direction(GPIO_NUM_4, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_dis(GPIO_NUM_4);
    rtc_gpio_pullup_dis(GPIO_NUM_4);
    rtc_gpio_hold_en(GPIO_NUM_4);
*/
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    /* not a wakeup from ULP, load the firmware */
    if (cause != ESP_SLEEP_WAKEUP_ULP)
    {
        printf("Not a ULP-RISC-V wakeup, initializing it! \n");
        init_ulp_program();
    }

    /* ULP Risc-V read and detected a change in GPIO_0, prints */
    /**
     * uyanınca ULP üzerinden a değişkenini yazıdıryor ekrana.
     * ulpden değişken çağırırken ulp deki adının başına "ulp_" eklemen gerekir
     * nasıl oluyor tam analamadım ama bu şekilde ulp deki değişkene erişiliyor.
    */
    if (cause == ESP_SLEEP_WAKEUP_ULP)
    {
        printf("ULP-RISC-V woke up a= %ld\n", ulp_gpio_level_previous_a);
        //   printf("ULP-RISC-V read changes in GPIO_0 current is: %s \n",
        //      (bool)(ulp_gpio_level_previous == 0) ? "Low" : "High" );
    }

    /* Go back to sleep, only the ULP Risc-V will run */
    printf("Entering in deep sleep\n\n");

    /* Small delay to ensure the messages are printed */

    /**
     * bu bekleme olmaz ise ekrana yazıdrmada hata alınıyor
     * ancak halende "None" diye kırmızı bir uyarı mevcut
     * neyin nesidir anlamadım
    */
    vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_ERROR_CHECK(esp_sleep_enable_ulp_wakeup());
    esp_deep_sleep_start();
}

static void init_ulp_program(void)
{
    esp_err_t err = ulp_riscv_load_binary(ulp_main_bin_start, (ulp_main_bin_end - ulp_main_bin_start));
    ESP_ERROR_CHECK(err);

    /* The first argument is the period index, which is not used by the ULP-RISC-V timer
     * The second argument is the period in microseconds, which gives a wakeup time period of: 20ms
     */

    /**
     * bu kod sayesinde ulp tarafı mainden çıkıp halt edince bile ulp bu bekleme süresinden sonra
     * yeniden kendiliğinden start vs demeden başlıyor
     * lifecycle da önemli olacak
    */
    ulp_set_wakeup_period(0, 20000);

    /* Start the program */
    err = ulp_riscv_run();
    ESP_ERROR_CHECK(err);
}
