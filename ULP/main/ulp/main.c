/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* ULP-RISC-V example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   This code runs on ULP-RISC-V  coprocessor
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "ulp_riscv.h"
#include "ulp_riscv_utils.h"
#include "ulp_riscv_gpio.h"

static bool gpio_level = false;

/* this variable will be exported as a public symbol, visible from main CPU: */
bool gpio_level_previous = false;
int gpio_level_previous_a = 0;

int main(void)
{
    static int a = 0;

    //bu tanımlama ledi çalıştırmaya yetti akım tüketimi 4mA- ledisz 0.4mA
    ulp_riscv_gpio_init(GPIO_NUM_4);
    ulp_riscv_gpio_output_enable(GPIO_NUM_4);
    ulp_riscv_gpio_set_output_mode(GPIO_NUM_4, RTCIO_MODE_OUTPUT);

    //1sn blink testi
    ulp_riscv_gpio_output_level(GPIO_NUM_4, 1);
    ulp_riscv_delay_cycles(1000 * ULP_RISCV_CYCLES_PER_MS);
    ulp_riscv_gpio_output_level(GPIO_NUM_4, 0);
    ulp_riscv_delay_cycles(1000 * ULP_RISCV_CYCLES_PER_MS);

    //seri monitore a değerini yazdırıyoruz uyanınca
    gpio_level_previous_a = a;

    if (a++ > 10)
    {
        //a değeri main de tanımlanan sürede arttırlır ve 11 olunca main cpu yu kaldırıyor.
        ulp_riscv_wakeup_main_processor();
        ulp_riscv_delay_cycles(1000 * ULP_RISCV_CYCLES_PER_MS);
        a = 0;
    }
    // burada bir while olmadığı için ulp sürekli açık kalmıyor, enerjı verimliliği
    /* ulp_riscv_halt() is called automatically when main exits */
    return 0;
}
