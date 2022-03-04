#include "utils.hpp"
#include <cstdint>
#include <stdio.h>
#include <stm32f3xx.h>
#include "usart.h"

void init_common()
{
    setbuf(stdout, NULL);
}

void HAL_HRTIM_Set_Compare(HRTIM_HandleTypeDef* hhrtim, uint32_t TimerIdx, uint32_t PWMChannel, uint32_t Pulse)
{
    switch (PWMChannel) {
    case HRTIM_OUTPUT_TA1:
    case HRTIM_OUTPUT_TB1:
    case HRTIM_OUTPUT_TC1:
    case HRTIM_OUTPUT_TD1:
    case HRTIM_OUTPUT_TE1: {
        hhrtim->Instance->sTimerxRegs[TimerIdx].CMP1xR = Pulse;
        break;
    }

    case HRTIM_OUTPUT_TA2:
    case HRTIM_OUTPUT_TB2:
    case HRTIM_OUTPUT_TC2:
    case HRTIM_OUTPUT_TD2:
    case HRTIM_OUTPUT_TE2: {
        hhrtim->Instance->sTimerxRegs[TimerIdx].CMP2xR = Pulse;
        break;
    }
    }
}

void HAL_HRTIM_Enable_Output(HRTIM_HandleTypeDef* hhrtim, uint32_t Outputs)
{
    hhrtim->Instance->sCommonRegs.OENR |= Outputs;
}

void HAL_HRTIM_Disable_Output(HRTIM_HandleTypeDef* hhrtim, uint32_t Outputs)
{
    hhrtim->Instance->sCommonRegs.OENR &= ~Outputs;
}

extern "C" {
int _write(int file, char* ptr, int len)
{
    (void)file;
    HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, 100);
    return len;
}
}
