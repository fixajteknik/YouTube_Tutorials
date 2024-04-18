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

#include "soc/rtc_cntl_reg.h"

static bool gpio_level = false;

/* this variable will be exported as a public symbol, visible from main CPU: */
bool gpio_level_previous = false;

int main(void)
{

    ulp_riscv_gpio_init(GPIO_NUM_4);
    ulp_riscv_gpio_input_enable(GPIO_NUM_4);
    ulp_riscv_gpio_pulldown_disable(GPIO_NUM_4);
    ulp_riscv_gpio_pullup_disable(GPIO_NUM_4);

    gpio_level = (bool)ulp_riscv_gpio_get_level(GPIO_NUM_4);
    gpio_level_previous = gpio_level;

    if (gpio_level)
    {
        gpio_level_previous = 1;
    }
    else
    {
        gpio_level_previous = 0;
    }

    if (gpio_level)
    {
        if (!(READ_PERI_REG(RTC_CNTL_LOW_POWER_ST_REG) & RTC_CNTL_MAIN_STATE_IN_IDLE))
        {
            ulp_riscv_wakeup_main_processor();
            ulp_riscv_delay_cycles(100 * ULP_RISCV_CYCLES_PER_MS);
        }
    }

    /* ulp_riscv_halt() is called automatically when main exits */
    return 0;
}
