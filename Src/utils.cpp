#include "utils.hpp"
#include <cstdint>
#include <stdio.h>
#include <stm32f3xx.h>
#include "usart.h"

void init_common()
{
    setbuf(stdout, NULL);
}

extern "C" {
int _write(int file, char* ptr, int len)
{
    (void)file;
    HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, 100);
    return len;
}
}
