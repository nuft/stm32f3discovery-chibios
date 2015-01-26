#ifndef L3GD20_H
#define L3GD20_H

#ifdef __cplusplus
extern "C" {
#endif

#include <hal.h>
#include <stdint.h>

#define L3GD20_RANGE_250DPS     0
#define L3GD20_RANGE_500DPS     1
#define L3GD20_RANGE_2000DPS    2

#define L3GD20_DATA_RATE_95HZ   0
#define L3GD20_DATA_RATE_190HZ  1
#define L3GD20_DATA_RATE_380HZ  2
#define L3GD20_DATA_RATE_760HZ  3

typedef struct {
    uint32_t range: 2;
    uint32_t rate: 2;
    union {
        SPIDriver *spi;
    } driver;
} l3gd20_t;

void l3gd20_init_using_spi(l3gd20_t *dev, SPIDriver *spi);
void l3gd20_setup(l3gd20_t *dev, uint8_t range, uint8_t rate);
bool l3gd20_ping(l3gd20_t *dev);

/** Read the sensor data.
 * dev sensor object.
 * gyro x, y and z angular rate output in rad/s.
 * temp temperature output in deg Kelvin (optional, can be NULL)
 */
void l3gd20_read(l3gd20_t *dev, float *gyro, float *temp);

#ifdef __cplusplus
}
#endif

#endif /* L3GD20_H */