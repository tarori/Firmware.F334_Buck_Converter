#include "main_cycle.hpp"
#include "utils.hpp"
#include "hrtim.h"
#include "adc.h"
#include "dac.h"
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
TYPE3Regulator i_regulator(3450, 4600, 1000, 100000, 314000, dt, 0, vout_max);

alignas(4) uint16_t adc1_buf[2];
alignas(4) uint16_t adc2_buf[2];

GPIO_PIN lcd_e(GPIOA, GPIO_PIN_1), lcd_rs(GPIOA, GPIO_PIN_2);
GPIO_PIN lcd_d4(GPIOA, GPIO_PIN_0), lcd_d5(GPIOB, GPIO_PIN_7), lcd_d6(GPIOB, GPIO_PIN_5), lcd_d7(GPIOA, GPIO_PIN_12);
LCD lcd(lcd_rs, lcd_e, lcd_d4, lcd_d5, lcd_d6, lcd_d7);

namespace Control
{
volatile float actual_voltage;
volatile float actual_current;
volatile float actual_voltage_filtered = 0.0f;
volatile float actual_current_filtered = 0.0f;
volatile float target_voltage = 0.0f;
volatile float target_current = 0.0f;
volatile float dac_voltage = 0.0f;
volatile float output_v;
volatile float output_i;
volatile float output;
volatile bool emergency_occured = false;
}  // namespace Control

void main_loop()
{
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);

    HAL_DAC_Start(&hdac1, DAC1_CHANNEL_2);
    uint32_t code = 0;
    while(1) {
        HAL_DAC_SetValue(&hdac1, DAC1_CHANNEL_2, DAC_ALIGN_12B_R, code);
        code = (code + 1) % 4096;
        printf("%d\n", code);
        delay_ms(1);
    }

    HAL_HRTIM_WaveformCountStart_IT(&hhrtim1, HRTIM_TIMERID_MASTER);
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
        static uint32_t lcd_clear_count = 0;
        if (lcd_clear_count++ % 100 == 0) {
            lcd.cls();
        }
        if (Control::emergency_occured) {
            printf("Emergency\n");
            continue;
        }

        printf("%.3f V %.3f V\n", Control::dac_voltage, Control::actual_voltage);
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
    /*
    float voltage_volume = adc2_buf[1] / 4095.0f * 20.0f;
    float current_volume = adc1_buf[1] / 4095.0f * 6.1f;

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
    */

   Control::target_current = 3.0f;
   Control::target_voltage = 3.3f;
}

constexpr float vref = 3.30f;
constexpr float voltage_div = 11.0f / 1.0f;
constexpr float voltage_gain = 11.0f;
constexpr float shunt_resistance = 0.010f;
constexpr float current_mul = vref / 4095 / (10.382f / 0.382f) / shunt_resistance;
constexpr float current_offset = 160.6f;

constexpr float emergency_voltage = 24.0f;
constexpr float emergency_current = 100.0f;
constexpr float filter_tau = 0.01f;

__attribute__((long_call, section(".ccmram"))) void callback_10us()
{
    if (Control::emergency_occured) {
        return;
    }

    Control::dac_voltage = ((voltage_gain / voltage_div * Control::target_voltage) + vref / 2) / (voltage_gain + 1);
    HAL_DAC_SetValue(&hdac1, DAC1_CHANNEL_2, DAC_ALIGN_12B_R, Control::dac_voltage / vref * 4095.0f);

    Control::actual_voltage = voltage_gain * (Control::dac_voltage - adc2_buf[0] * vref / 4095.0f) + Control::target_voltage;

    Control::actual_current = (adc1_buf[0] - current_offset) * current_mul;

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
    // pwm_set_duty(Control::output, true);
    pwm_set_duty(10.0f, true);
}
