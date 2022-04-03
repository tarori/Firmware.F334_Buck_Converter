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

alignas(4) uint16_t adc1_buf[2];
alignas(4) uint16_t adc2_buf[2];

GPIO_PIN lcd_e(GPIOA, GPIO_PIN_1), lcd_rs(GPIOA, GPIO_PIN_2);
GPIO_PIN lcd_d4(GPIOA, GPIO_PIN_0), lcd_d5(GPIOB, GPIO_PIN_7), lcd_d6(GPIOB, GPIO_PIN_5), lcd_d7(GPIOA, GPIO_PIN_12);
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

    __disable_irq();
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc1_buf, 2);
    hadc1.DMA_Handle->Instance->CCR &= ~(DMA_IT_TC | DMA_IT_HT);
    HAL_ADC_Start_DMA(&hadc2, (uint32_t*)adc2_buf, 2);
    hadc2.DMA_Handle->Instance->CCR &= ~(DMA_IT_TC | DMA_IT_HT);
    __enable_irq();

    lcd.init();

    printf("Hello, I am working at %ldMHz\n", SystemCoreClock / 1000 / 1000);
    lcd.printf("Hello\nWorking at %ldMHz\n", SystemCoreClock / 1000 / 1000);

    callback_start = true;

    while (1) {
        printf("%.3f V  %.4f A\n", Control::actual_voltage, Control::actual_current);
        printf("%d  %d\n", adc1_buf[1], adc2_buf[1]);
        delay_ms(250);
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
constexpr float current_offset = -0.128f;

void callback_10us()
{
    Control::actual_voltage = adc1_buf[0] * voltage_mul + voltage_offset;
    Control::actual_current = adc2_buf[0] * current_mul + current_offset;

    Control::output_v = v_regulator(Control::actual_voltage - Control::target_voltage);
    Control::output_i = i_regulator(Control::actual_current - Control::target_current);
    Control::output = std::min(Control::output_v, Control::output_i);
    pwm_set_duty(Control::output);
}
