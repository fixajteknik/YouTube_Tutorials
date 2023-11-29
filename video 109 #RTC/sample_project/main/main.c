#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "time.h"
#include "sys/time.h"
#include "soc/rtc.h"
#include "driver/rtc_io.h"
#include "freertos/xtensa_timer.h"
#include "esp32s3/rom/rtc.h"

static const char *TAG = "Deep Sleep with Compile Time";
// RTC hafızasında saklanacak değişkeni tanımla
RTC_DATA_ATTR static int boot_count = 0;

void set_compile_time()
{
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));

    strptime(__DATE__ " " __TIME__, "%b %d %Y %H:%M:%S", &tm);
    time_t t = mktime(&tm);

    struct timeval now = {.tv_sec = t};
    settimeofday(&now, NULL);
}

void print_rtc_slow_clk_frequency()
{
    uint32_t clk_freq_hz = rtc_clk_slow_freq_get_hz();
    ESP_LOGI(TAG, "RTC Slow Clock Frequency: %lu Hz", clk_freq_hz);
}

void print_rtc_slow_clk_src()
{
    soc_rtc_slow_clk_src_t clk_src = rtc_clk_slow_src_get();

    switch (clk_src)
    {
    case SOC_RTC_SLOW_CLK_SRC_RC_SLOW:
        ESP_LOGI(TAG, "RTC Slow Clock Source: Internal RC oscillator");
        break;
    case SOC_RTC_SLOW_CLK_SRC_XTAL32K:
        ESP_LOGI(TAG, "RTC Slow Clock Source: External 32kHz XTAL");
        break;
    default:
        ESP_LOGI(TAG, "RTC Slow Clock Source: Unknown");
        break;
    }
}

void check_rtc_clock_source()
{

    if (rtc_clk_32k_enabled())
    {
        ESP_LOGI(TAG, "RTC rtc_clk_32k_enabled");
    }
    else
    {
        ESP_LOGI(TAG, "RTC NO enabled");
    }

    print_rtc_slow_clk_src();

    print_rtc_slow_clk_frequency();
}

void print_current_time()
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "Current time: %s", strftime_buf);
}

void app_main()
{
    boot_count++;

    ESP_LOGI(TAG, "Boot count: %d", boot_count);

    if (boot_count == 1)
    {
        // Sadece ilk başlatmada saati ayarla
        set_compile_time();
        check_rtc_clock_source();
    }

    while (1)
    {
        // Print the current time
        vTaskDelay(200);
        ESP_LOGI(TAG, "uyanık");
        print_current_time();
        // The device will restart after deep sleep
        // Execution will resume from here after restart

        // Stay awake for 30 seconds
        vTaskDelay(1000);

        // Print the current time

        // Enter deep sleep for 1 minute
        ESP_LOGI(TAG, "Entering deep sleep for 10 sec");
        esp_sleep_enable_timer_wakeup(10000000);
        esp_deep_sleep_start();
    }
}
