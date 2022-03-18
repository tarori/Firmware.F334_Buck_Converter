#include "main_cycle.hpp"
#include "utils.hpp"
#include "hrtim.h"
#include "adc.h"
#include "clock.hpp"
#include <stm32f3xx.h>
#include "pwm.hpp"
#include "pid_regulator.hpp"
#include "lcd.hpp"

extern bool callback_start;

PIDRegulator v_regulator(0, -1.0f, 0, dt, 0, 24);
PIDRegulator i_regulator(0, -1.0f, 0, dt, 0, 24);

uint16_t voltage_adc_buf[1];
uint16_t current_adc_buf[1];

GPIO_PIN lcd_e(GPIOB, GPIO_PIN_5), lcd_rs(GPIOA, GPIO_PIN_0);
GPIO_PIN lcd_d4(GPIOA, GPIO_PIN_1), lcd_d5(GPIOA, GPIO_PIN_2), lcd_d6(GPIOA, GPIO_PIN_3), lcd_d7(GPIOA, GPIO_PIN_4);
LCD lcd(lcd_rs, lcd_e, lcd_d4, lcd_d5, lcd_d6, lcd_d7);

namespace Control
{
float actual_voltage;
float actual_current;
float target_voltage;
float target_current;
float output_v;
float output_i;
float output;
}  // namespace Control

void main_loop()
{
    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2);
    HAL_HRTIM_WaveformCountStart_IT(&hhrtim1, HRTIM_TIMERID_TIMER_A);

    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)voltage_adc_buf, 1);
    HAL_ADC_Start_DMA(&hadc2, (uint32_t*)current_adc_buf, 1);

    lcd.init();

    printf("Hello, I am working at %ld Hz\n", SystemCoreClock);
    lcd.printf("Hello\n");

    callback_start = true;

    while (1) {
    }
}

void callback_1ms()
{
    Control::target_voltage = 5.0f;
    Control::target_current = 1.0f;
}

void callback_10us()
{
    Control::actual_voltage = voltage_adc_buf[0] * 3.3f / 4096 * 11.0f;
    Control::actual_current = current_adc_buf[0] * 3.3f / 4096 / 11.0f * 0.01f;
    Control::output_v = v_regulator(Control::target_voltage - Control::actual_voltage);
    Control::output_i = i_regulator(Control::target_current - Control::actual_current);
    Control::output = std::min(Control::output_v, Control::output_i);
    pwm_set_duty(Control::output);
}
