#include <hardware/i2c.h>

extern "C" {
#include "MLX90640_I2C_Driver.h"

int MLX90640_I2CRead(uint8_t address, uint16_t reg, uint16_t words, uint16_t* data) {
    // The EEPROM dump is the largest read. I2C1 has a single owner in each lesson.
    static uint8_t bytes[MLX90640_EEPROM_DUMP_NUM * 2];
    if (words > MLX90640_EEPROM_DUMP_NUM) return -MLX90640_I2C_NACK_ERROR;

    const uint8_t command[]{uint8_t(reg >> 8), uint8_t(reg)};
    const int size = words * 2;
    if (i2c_write_timeout_us(i2c1, address, command, sizeof(command), true, 5000) != sizeof(command) ||
        i2c_read_timeout_us(i2c1, address, bytes, size, false, 60000) != size) {
        return -MLX90640_I2C_NACK_ERROR;
    }
    for (uint16_t i = 0; i < words; ++i) {
        data[i] = (uint16_t(bytes[2 * i]) << 8) | bytes[2 * i + 1];
    }
    return MLX90640_NO_ERROR;
}

int MLX90640_I2CWrite(uint8_t address, uint16_t reg, uint16_t value) {
    const uint8_t bytes[]{uint8_t(reg >> 8), uint8_t(reg), uint8_t(value >> 8), uint8_t(value)};
    return i2c_write_timeout_us(i2c1, address, bytes, sizeof(bytes), false, 5000) == sizeof(bytes)
        ? MLX90640_NO_ERROR : -MLX90640_I2C_NACK_ERROR;
}

int MLX90640_I2CGeneralReset(void) {
    const uint8_t command = 0x06;
    return i2c_write_timeout_us(i2c1, 0, &command, 1, false, 5000) == 1
        ? MLX90640_NO_ERROR : -MLX90640_I2C_NACK_ERROR;
}
}
