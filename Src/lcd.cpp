#include "lcd.hpp"
#include "clock.hpp"

LCD::LCD(I2C_HandleTypeDef* hi2c) : hi2c(hi2c)
{
}

void LCD::init()
{
    delay_ms(50);
    writeByte(0x03, 0);
    delay_ms(5);
    writeByte(0x03, 0);
    delay_ms(1);
    writeByte(0x03, 0);
    delay_ms(1);
    writeByte(0x02, 0);
    writeCommand(0x28);
    writeCommand(0x0C);
    writeCommand(0x06);
    writeCommand(0x01);
    delay_ms(2);
}

void LCD::character(int column, int row, int c)
{
    int addr = address(column, row);
    writeCommand(addr);
    writeData(c);
}

void LCD::cls()
{
    writeCommand(0x01);
    delay_ms(2);
    locate(0, 0);
}

void LCD::setBacklight(uint8_t state)
{
    if (state) {
        backlight_state = 1;
    } else {
        backlight_state = 0;
    }
}

void LCD::locate(int column, int row)
{
    this->column = column;
    this->row = row;
}

int LCD::putc(char value)
{
    if (value == '\n') {
        column = 0;
        row++;
        if (row >= rows()) {
            row = 0;
        }
    } else {
        character(column, row, value);
        column++;
        if (column >= columns()) {
            column = 0;
            row++;
            if (row >= rows()) {
                row = 0;
            }
        }
    }
    return value;
}

void LCD::writeByte(uint8_t value, uint8_t rs)
{
    uint8_t data = value << D4_BIT;
    data |= rs << RS_BIT;
    data |= backlight_state << BL_BIT;  // Include backlight state in data
    data |= 1 << EN_BIT;
    HAL_I2C_Master_Transmit(hi2c, I2C_ADDR << 1, &data, 1, 100);
    delay_ms(1);
    data &= ~(1 << EN_BIT);
    HAL_I2C_Master_Transmit(hi2c, I2C_ADDR << 1, &data, 1, 100);
}

void LCD::writeCommand(uint8_t command)
{
    uint8_t upper_data = command >> 4;
    uint8_t lower_data = command & 0x0F;
    writeByte(upper_data, 0);
    writeByte(lower_data, 0);
    if (command == 0x01 || command == 0x02) {
        delay_ms(2);
    }
}

void LCD::writeData(uint8_t data)
{
    uint8_t upper_data = data >> 4;
    uint8_t lower_data = data & 0x0F;
    writeByte(upper_data, 1);
    writeByte(lower_data, 1);
}

int LCD::address(int column, int row)
{
    return 0x80 + (row * 0x40) + column;
}

int LCD::columns()
{
    return 16;
}

int LCD::rows()
{
    return 2;
}

int LCD::printf(const char* format, ...)
{
    char buff[33];
    va_list args;
    va_start(args, format);
    int count = vsnprintf(buff, sizeof(buff), format, args);
    va_end(args);
    for (int i = 0; i < count; ++i) {
        this->putc(buff[i]);
    }
    return count;
}
