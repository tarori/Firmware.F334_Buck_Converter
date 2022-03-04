#pragma once

#include <stm32f3xx.h>
#include <stdint.h>
#include <cstdarg>
#include <stdio.h>
#include <cstring>

class GPIO_PIN
{
private:
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;

public:
    GPIO_PIN() = default;
    GPIO_PIN(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) : GPIOx(GPIOx), GPIO_Pin(GPIO_Pin)
    {
    }
    void write(bool state)
    {
        if (state) {
            HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
        }
    }
    bool read()
    {
        return HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_SET;
    }
    void operator=(bool state)
    {
        this->write(state);
    }
    bool operator()()
    {
        return this->read();
    }
};

class LCD
{
public:
    LCD(GPIO_PIN rs, GPIO_PIN e, GPIO_PIN d4, GPIO_PIN d5, GPIO_PIN d6, GPIO_PIN d7);

    void init();
    void locate(int column, int row);
    void cls();
    int rows();
    int columns();
    int printf(const char* format, ...);

private:
    int putc(char value);
    int address(int column, int row);
    void writeByte(uint8_t value);
    void writeCommand(uint8_t command);
    void writeData(uint8_t data);
    void character(int column, int row, int c);
    void writeBus(uint8_t value);

    GPIO_PIN rs, e;
    GPIO_PIN bus[4];
    int column;
    int row;
};
