#pragma once

#include <stm32f3xx.h>
#include <stdint.h>
#include <cstdarg>
#include <stdio.h>
#include <cstring>

class LCD
{
public:
    LCD(I2C_HandleTypeDef* hi2c);
    void init();
    void locate(int column, int row);
    void cls();
    void setBacklight(uint8_t state);
    int rows();
    int columns();
    int printf(const char* format, ...);

private:
    int putc(char value);
    int address(int column, int row);
    void writeByte(uint8_t value, uint8_t rs);
    void writeCommand(uint8_t command);
    void writeData(uint8_t data);
    void character(int column, int row, int c);

#define I2C_ADDR 0x3F  // I2C address of the PCF8574
#define RS_BIT 0       // Register select bit
#define EN_BIT 2       // Enable bit
#define BL_BIT 3       // Backlight bit
#define D4_BIT 4       // Data 4 bit
#define D5_BIT 5       // Data 5 bit
#define D6_BIT 6       // Data 6 bit
#define D7_BIT 7       // Data 7 bit

    I2C_HandleTypeDef* hi2c;
    int column;
    int row;
    uint8_t backlight_state = 0;
};
