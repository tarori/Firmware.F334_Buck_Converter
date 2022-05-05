#include "main_cycle.hpp"
#include "utils.hpp"
#include "hrtim.h"
#include "opamp.h"
#include "adc.h"
#include "clock.hpp"
#include <stm32f3xx.h>

#include "pwm.hpp"
#include "pid_regulator.hpp"
#include "type3_regulator.hpp"
#include "lcd.hpp"

extern bool callback_start;

/*
wz1=0.75/sqrt(LoutCout)
wz2=1/sqrt(LoutCout)
wp0=min(wz1,wp2)
wp2=1/(Cout*ESR)
wp3=2PI*fsw/2
*/

TYPE3Regulator v_regulator(3450, 4600, 10000, 100000, 314000, dt, 0, vout_max);
PIDRegulator i_regulator(1.0f, 1000.0f, 0, dt, 0, vout_max);

alignas(4) uint16_t adc1_buf[3];
alignas(4) uint16_t adc2_buf[2];

GPIO_PIN lcd_e(GPIOA, GPIO_PIN_1), lcd_rs(GPIOA, GPIO_PIN_2);
GPIO_PIN lcd_d4(GPIOA, GPIO_PIN_0), lcd_d5(GPIOB, GPIO_PIN_7), lcd_d6(GPIOB, GPIO_PIN_5), lcd_d7(GPIOA, GPIO_PIN_12);
LCD lcd(lcd_rs, lcd_e, lcd_d4, lcd_d5, lcd_d6, lcd_d7);

namespace Control
{
float actual_voltage;
float actual_current;
float actual_voltage_filtered = 0.0f;
float actual_current_filtered = 0.0f;
float target_voltage = 0.0f;
float target_current = 0.0f;
float output_v;
float output_i;
float output;
bool emergency_occured = false;
}  // namespace Control

void main_loop()
{
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);

    HAL_OPAMP_Start(&hopamp2);
    HAL_HRTIM_WaveformCountStart_IT(&hhrtim1, HRTIM_TIMERID_TIMER_A);

    __disable_irq();
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc1_buf, sizeof(adc1_buf) / sizeof(uint16_t));
    hadc1.DMA_Handle->Instance->CCR &= ~(DMA_IT_TC | DMA_IT_HT);
    HAL_ADC_Start_DMA(&hadc2, (uint32_t*)adc2_buf, sizeof(adc2_buf) / sizeof(uint16_t));
    hadc2.DMA_Handle->Instance->CCR &= ~(DMA_IT_TC | DMA_IT_HT);
    __enable_irq();

    lcd.init();

    printf("Hello, I am working at %ldMHz\n", SystemCoreClock / 1000 / 1000);
    lcd.printf("Hello\nWorking at %ldMHz\n", SystemCoreClock / 1000 / 1000);
    delay_ms(1000);

    callback_start = true;

    while (1) {
        // printf("%.3f V  %.4f A\n", Control::actual_voltage_filtered, Control::actual_current_filtered);
        lcd.locate(0, 0);
        lcd.printf("T:%6.2fV %5.2fA", Control::target_voltage, Control::target_current);
        lcd.locate(0, 1);
        lcd.printf("A:%6.2fV %5.2fA", Control::actual_voltage_filtered, Control::actual_current_filtered);
        delay_ms(250);
    }
}

constexpr float voltage_step = 0.25f;
constexpr float current_step = 0.1f;

void callback_10ms()
{
    float voltage_volume = adc2_buf[1] / 4096.0f * 20.0f;
    float current_volume = adc1_buf[1] / 4096.0f * 8.1f;

    if (voltage_volume > Control::target_voltage + voltage_step) {
        Control::target_voltage += voltage_step;
    }
    if (voltage_volume < Control::target_voltage - voltage_step) {
        Control::target_voltage -= voltage_step;
    }


    if (current_volume > Control::target_current + current_step) {
        Control::target_current += current_step;
    }
    if (current_volume < Control::target_current - current_step) {
        Control::target_current -= current_step;
    }
}

constexpr float vref = 3.36f;
constexpr float shunt_resistance = 0.0301f;
constexpr float voltage_mul = vref / 4096 * (11.5f / 1.5f);
constexpr float current_mul = vref / 4096 / (11.5f / 1.5f) / shunt_resistance;
constexpr float voltage_offset = 0.0f;
constexpr float current_offset = -0.62f;
constexpr float voltage_shunt_injection = 0.0038f;

constexpr float emergency_voltage = 24.0f;
constexpr float emergency_current = 100.0f;
constexpr float filter_tau = 0.01f;

void callback_10us()
{
    if (Control::emergency_occured) {
        return;
    }

    Control::actual_voltage = (adc1_buf[0] + adc1_buf[2]) * 0.5f * voltage_mul + voltage_offset;
    Control::actual_current = adc2_buf[0] * current_mul + current_offset - Control::actual_voltage * voltage_shunt_injection;

    Control::actual_voltage_filtered += filter_tau * (Control::actual_voltage - Control::actual_voltage_filtered);
    Control::actual_current_filtered += filter_tau * (Control::actual_current - Control::actual_current_filtered);

    if (Control::actual_current > emergency_current || Control::actual_voltage > emergency_voltage) {
        pwm_free();
        Control::emergency_occured = true;
        return;
    }

    Control::output_v = v_regulator(Control::target_voltage - Control::actual_voltage);
    Control::output_i = i_regulator(Control::target_current - Control::actual_current);
    Control::output = std::min(Control::output_v, Control::output_i);
    v_regulator.set_actual_output(Control::output);
    pwm_set_duty(Control::output, true);
}
