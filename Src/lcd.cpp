#include "lcd.hpp"
#include "clock.hpp"

LCD::LCD(GPIO_PIN rs, GPIO_PIN e, GPIO_PIN d4, GPIO_PIN d5, GPIO_PIN d6, GPIO_PIN d7) : rs(rs), e(e)
{
    bus[0] = d4;
    bus[1] = d5;
    bus[2] = d6;
    bus[3] = d7;
}

void LCD::init()
{
    e = 1;  // command mode
    rs = 0;
    delay_ms(15);  // Wait 15ms to ensure powered up

    // send "Display Settings" 3 times (Only top nibble of 0x30 as we've got 4-bit bus)
    for (int i = 0; i < 3; i++) {
        writeByte(0x03);
        delay_us(1640);  // this command takes 1.64ms, so wait for it
    }
    writeByte(0x02);  // 4-bit mode
    delay_us(40);     // most instructions take 40us

    writeCommand(0x28);  // Function set 001 BW N F - -
    writeCommand(0x0C);
    writeCommand(0x06);  // Cursor Direction and Display Shift : 0000 01 CD S (CD 0-left, 1-right S(hift) 0-no, 1-yes
    cls();
}

void LCD::character(int column, int row, int c)
{
    int a = address(column, row);
    writeCommand(a);
    writeData(c);
}

void LCD::cls()
{
    writeCommand(0x01);  // cls, and set cursor to 0
    delay_us(1640);      // This command takes 1.64 ms
    locate(0, 0);
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

void LCD::writeBus(uint8_t value)
{
    bus[0] = value & 0x01;
    bus[1] = value & 0x02;
    bus[2] = value & 0x04;
    bus[3] = value & 0x08;
}

void LCD::writeByte(uint8_t value)
{
    writeBus(value >> 4);
    delay_us(40);
    e = 0;
    delay_us(40);
    e = 1;
    writeBus(value & 0x0f);
    delay_us(40);
    e = 0;
    delay_us(40);
    e = 1;
}

void LCD::writeCommand(uint8_t command)
{
    rs = 0;
    writeByte(command);
}

void LCD::writeData(uint8_t data)
{
    rs = 1;
    writeByte(data);
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
