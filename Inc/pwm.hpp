#pragma once

#include "utils.hpp"
#include <algorithm>

constexpr uint32_t pwm_period = 15360;
constexpr float pwm_max = 0.9f;
constexpr float vbus = 20.0f;
constexpr float dt = 10 * 1e-6;
constexpr uint32_t deadtime = 4 * 75;
constexpr float vout_max = pwm_max * vbus;

constexpr HRTIM_HandleTypeDef* hhrtim = &hhrtim1;

static inline void pwm_set_duty(float voltage, bool enable_lo)
{
    float duty = std::clamp(voltage / vbus, 0.0f, pwm_max);
    uint32_t duty_int = duty * pwm_period + deadtime;
    HAL_HRTIM_Set_Compare(hhrtim, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA1, pwm_period - duty_int);
    if (enable_lo) {
        HAL_HRTIM_Enable_Output(hhrtim, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2);
    } else {
        HAL_HRTIM_Enable_Output(hhrtim, HRTIM_OUTPUT_TA2);
        HAL_HRTIM_Disable_Output(hhrtim, HRTIM_OUTPUT_TA1);
    }
}

static inline void pwm_free()
{
    HAL_HRTIM_Disable_Output(hhrtim, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2);
    HAL_HRTIM_Set_Compare(hhrtim, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA1, 0);
}
