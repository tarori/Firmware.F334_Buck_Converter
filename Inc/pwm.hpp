#include "utils.hpp"
#include <algorithm>

constexpr uint32_t pwm_period = 40960;
constexpr float pwm_max = 0.9f;
constexpr float vbus = 20.0f;
constexpr float dt = 10 * 0.000001f;

constexpr HRTIM_HandleTypeDef* hhrtim = &hhrtim1;

void pwm_set_duty(float voltage)
{
    float duty = std::clamp(voltage / vbus, 0.0f, pwm_max);
    uint32_t duty_int = duty * pwm_period;
    HAL_HRTIM_Set_Compare(hhrtim, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA1, duty_int);
    HAL_HRTIM_Enable_Output(hhrtim, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2);
}

void pwm_free()
{
    HAL_HRTIM_Disable_Output(hhrtim, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2);
    HAL_HRTIM_Set_Compare(hhrtim, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA1, 0);
}
