#include "main_cycle.hpp"
#include "utils.hpp"
#include "hrtim.h"
#include "opamp.h"
#include "adc.h"
#include "clock.hpp"
#include <stm32f3xx.h>
#include "pwm.hpp"
#include "pid_regulator.hpp"
#include "lcd.hpp"

extern bool callback_start;

PIDRegulator v_regulator(-0.1f, -100.0f, -0.0f, dt, 0, vbus);
PIDRegulator i_regulator(-10.0f, -10000.0f, 0, dt, 0, vbus);

volatile uint16_t voltage_adc_buf;
volatile uint16_t current_adc_buf;

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
    HAL_OPAMP_Start(&hopamp2);
    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2);
    HAL_HRTIM_WaveformCountStart_IT(&hhrtim1, HRTIM_TIMERID_TIMER_A);

    // HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&voltage_adc_buf, 1);
    // HAL_ADC_Start_DMA(&hadc2, (uint32_t*)&current_adc_buf, 1);
    HAL_ADC_Start(&hadc1);
    HAL_ADC_Start(&hadc2);

    lcd.init();

    printf("Hello, I am working at %ld Hz\n", SystemCoreClock);
    lcd.printf("Hello\n");

    callback_start = true;
    HAL_ADC_Start(&hadc1);

    while (1) {
        printf("%.3f V  %.4f A\n", Control::actual_voltage, Control::actual_current);
        delay_ms(1000);
    }
}

void callback_1ms()
{
    Control::target_voltage = 5.0f;
    Control::target_current = 0.3f;
}

constexpr float vref = 3.3f;
constexpr float shunt_resistance = 0.1f;
constexpr float voltage_mul = vref / 4096 * (11.5f / 1.5f);
constexpr float current_mul = vref / 4096 / (11.5f / 1.5f) / shunt_resistance;
constexpr float voltage_offset = 0.375f;
constexpr float current_offset = -0.12f;

void callback_10us()
{
    voltage_adc_buf = HAL_ADC_GetValue(&hadc1);
    current_adc_buf = HAL_ADC_GetValue(&hadc2);

    Control::actual_voltage = voltage_adc_buf * voltage_mul + voltage_offset;
    Control::actual_current = current_adc_buf * current_mul + current_offset;

    Control::output_v = v_regulator(Control::actual_voltage - Control::target_voltage);
    Control::output_i = i_regulator(Control::actual_current - Control::target_current);
    Control::output = std::min(Control::output_v, Control::output_i);
    pwm_set_duty(Control::output);
}
