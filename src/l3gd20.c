#include <hal.h>
#include "l3gd20.h"

#define L3GD20_RA_WHO_AM_I          0x0F // r
#define L3GD20_RA_CTRL_REG1         0x20 // rw
#define L3GD20_RA_CTRL_REG2         0x21 // rw
#define L3GD20_RA_CTRL_REG3         0x22 // rw
#define L3GD20_RA_CTRL_REG4         0x23 // rw
#define L3GD20_RA_CTRL_REG5         0x24 // rw
#define L3GD20_RA_REFERENCE         0x25 // r
#define L3GD20_RA_OUT_TEMP          0x26 // r
#define L3GD20_RA_STATUS_REG        0x27 // r
#define L3GD20_RA_OUT_X_L           0x28 // r
#define L3GD20_RA_OUT_X_H           0x29 // r
#define L3GD20_RA_OUT_Y_L           0x2A // r
#define L3GD20_RA_OUT_Y_H           0x2B // r
#define L3GD20_RA_OUT_Z_L           0x2C // r
#define L3GD20_RA_OUT_Z_H           0x2D // r
#define L3GD20_RA_FIFO_CTRL_REG     0x2E // rw
#define L3GD20_RA_FIFO_SRC_REG      0x2F // r
#define L3GD20_RA_INT1_CFG          0x30 // rw
#define L3GD20_RA_INT1_SRC          0x31 // r
#define L3GD20_RA_INT1_TSH_XH       0x32 // rw
#define L3GD20_RA_INT1_TSH_XL       0x33 // rw
#define L3GD20_RA_INT1_TSH_YH       0x34 // rw
#define L3GD20_RA_INT1_TSH_YL       0x35 // rw
#define L3GD20_RA_INT1_TSH_ZH       0x36 // rw
#define L3GD20_RA_INT1_TSH_ZL       0x37 // rw
#define L3GD20_RA_INT1_DURATION     0x38 // rw

#define L3GD20_CTRL_REG1_DR1        (1<<7)
#define L3GD20_CTRL_REG1_DR0        (1<<6)
#define L3GD20_CTRL_REG1_BW1        (1<<5)
#define L3GD20_CTRL_REG1_BW0        (1<<4)
#define L3GD20_CTRL_REG1_PD         (1<<3)
#define L3GD20_CTRL_REG1_ZEN        (1<<2)
#define L3GD20_CTRL_REG1_XEN        (1<<1)
#define L3GD20_CTRL_REG1_YEN        (1<<0)

#define L3GD20_CTRL_REG1_RATE(x)    ((x)<<6)

#define L3GD20_CTRL_REG2_HPM1       (1<<5)
#define L3GD20_CTRL_REG2_HPM0       (1<<4)
#define L3GD20_CTRL_REG2_HPCF3      (1<<3)
#define L3GD20_CTRL_REG2_HPCF2      (1<<2)
#define L3GD20_CTRL_REG2_HPCF1      (1<<1)
#define L3GD20_CTRL_REG2_HPCF0      (1<<0)

#define L3GD20_CTRL_REG2_HP(x)      ((x)<<0)

#define L3GD20_CTRL_REG3_I1_INT1    (1<<7)
#define L3GD20_CTRL_REG3_I1_BOOT    (1<<6)
#define L3GD20_CTRL_REG3_H_LACTIVE  (1<<5)
#define L3GD20_CTRL_REG3_PP_OD      (1<<4)
#define L3GD20_CTRL_REG3_I2_DRDY    (1<<3)
#define L3GD20_CTRL_REG3_I2_WTM     (1<<2)
#define L3GD20_CTRL_REG3_I2_ORUN    (1<<1)
#define L3GD20_CTRL_REG3_I2_EMPTY   (1<<0)

#define L3GD20_CTRL_REG4_BDU        (1<<7)
#define L3GD20_CTRL_REG4_BLE        (1<<6)
#define L3GD20_CTRL_REG4_FS1        (1<<5)
#define L3GD20_CTRL_REG4_FS0        (1<<4)
#define L3GD20_CTRL_REG4_SIM        (1<<0)

#define L3GD20_CTRL_REG4_RANGE(x)   ((x)<<4)

#define L3GD20_CTRL_REG5_BOOT       (1<<7)
#define L3GD20_CTRL_REG5_FIFO_EN    (1<<6)
#define L3GD20_CTRL_REG5_HPEN       (1<<4)
#define L3GD20_CTRL_REG5_INT1_SEL1  (1<<3)
#define L3GD20_CTRL_REG5_INT1_SEL0  (1<<2)
#define L3GD20_CTRL_REG5_OUT_SEL1   (1<<1)
#define L3GD20_CTRL_REG5_OUT_SEL0   (1<<0)

#define L3GD20_STATUS_REG_ZYXOR     (1<<7)
#define L3GD20_STATUS_REG_ZOR       (1<<6)
#define L3GD20_STATUS_REG_YOR       (1<<5)
#define L3GD20_STATUS_REG_XOR       (1<<4)
#define L3GD20_STATUS_REG_ZYXDA     (1<<3)
#define L3GD20_STATUS_REG_ZDA       (1<<2)
#define L3GD20_STATUS_REG_YDA       (1<<1)
#define L3GD20_STATUS_REG_XDA       (1<<0)

#define L3GD20_FIFO_CTRL_REG_FM2    (1<<7)
#define L3GD20_FIFO_CTRL_REG_FM1    (1<<6)
#define L3GD20_FIFO_CTRL_REG_FM0    (1<<5)
#define L3GD20_FIFO_CTRL_REG_WTM4   (1<<4)
#define L3GD20_FIFO_CTRL_REG_WTM3   (1<<3)
#define L3GD20_FIFO_CTRL_REG_WTM2   (1<<2)
#define L3GD20_FIFO_CTRL_REG_WTM1   (1<<1)
#define L3GD20_FIFO_CTRL_REG_WTM0   (1<<0)

#define L3GD20_FIFO_CTRL_REG_FM_BYPASS_MODE             (0)
#define L3GD20_FIFO_CTRL_REG_FM_FIFO_MODE               (1<<5)
#define L3GD20_FIFO_CTRL_REG_FM_STREAM_MODE             (2<<5)
#define L3GD20_FIFO_CTRL_REG_FM_STREAM_TO_FIFO_MODE     (3<<5)
#define L3GD20_FIFO_CTRL_REG_FM_BYPASS_TO_STREAM_MODE   (4<<5)

#define L3GD20_FIFO_SRC_REG_WTM     (1<<7)
#define L3GD20_FIFO_SRC_REG_OVRN    (1<<6)
#define L3GD20_FIFO_SRC_REG_EMPTY   (1<<5)
#define L3GD20_FIFO_SRC_REG_FSS4    (1<<4)
#define L3GD20_FIFO_SRC_REG_FSS3    (1<<3)
#define L3GD20_FIFO_SRC_REG_FSS2    (1<<2)
#define L3GD20_FIFO_SRC_REG_FSS1    (1<<1)
#define L3GD20_FIFO_SRC_REG_FSS0    (1<<0)

#define L3GD20_INT1_CFG_AND_OR      (1<<7)
#define L3GD20_INT1_CFG_LIR         (1<<6)
#define L3GD20_INT1_CFG_ZHIE        (1<<5)
#define L3GD20_INT1_CFG_ZLIE        (1<<4)
#define L3GD20_INT1_CFG_YHIE        (1<<3)
#define L3GD20_INT1_CFG_YLIE        (1<<2)
#define L3GD20_INT1_CFG_XHIE        (1<<1)
#define L3GD20_INT1_CFG_XLIE        (1<<0)

#define L3GD20_INT1_SRC_IA          (1<<6)
#define L3GD20_INT1_SRC_ZH          (1<<5)
#define L3GD20_INT1_SRC_ZL          (1<<4)
#define L3GD20_INT1_SRC_YH          (1<<3)
#define L3GD20_INT1_SRC_YL          (1<<2)
#define L3GD20_INT1_SRC_XH          (1<<1)
#define L3GD20_INT1_SRC_XL          (1<<0)


#define L3GD20_WHO_AM_I_VALUE 0b11010100

// I2C not implemented
// #define L3GD20_I2C_ADDRESS_SDO_LOW  0b1101010 // SDO pin = GND
// #define L3GD20_I2C_ADDRESS_SDO_HIGH 0b1101011 // SDO pin = VCC

#define DEG_TO_RADIAN(x)        (x * 2 * 3.1415926535898f / 360)
#define CELSIUS_TO_KELVIN(x)    (x + 273.15)

const float l3gd20_gain[] = {
    DEG_TO_RADIAN(0.00875f),
    DEG_TO_RADIAN(0.0175f),
    DEG_TO_RADIAN(0.07f)};

static uint8_t l3gd20_reg_read(l3gd20_t *dev, uint8_t reg)
{
    uint8_t val = 0;
    spiSelect(dev->driver.spi);
    reg |= 0x80;
    spiSend(dev->driver.spi, 1, &reg);
    spiReceive(dev->driver.spi, 1, &val);
    spiUnselect(dev->driver.spi);
    return val;
}

static void l3gd20_reg_read_multi(l3gd20_t *dev, uint8_t reg, uint8_t *buf, uint8_t len)
{
    spiSelect(dev->driver.spi);
    reg |= 0xC0;
    spiSend(dev->driver.spi, 1, &reg);
    spiReceive(dev->driver.spi, len, buf);
    spiUnselect(dev->driver.spi);
}

static void l3gd20_reg_write(l3gd20_t *dev, uint8_t reg, uint8_t val)
{
    uint8_t buf[] = {reg, val};
    spiSelect(dev->driver.spi);
    spiSend(dev->driver.spi, 2, buf);
    spiUnselect(dev->driver.spi);
}

static void l3gd20_reg_write_multi(l3gd20_t *dev, uint8_t reg, uint8_t *buf, uint8_t len)
{
    reg |= 0x40;
    spiSelect(dev->driver.spi);
    spiSend(dev->driver.spi, 1, &reg);
    spiSend(dev->driver.spi, len, buf);
    spiUnselect(dev->driver.spi);
}

void l3gd20_init_using_spi(l3gd20_t *dev, SPIDriver *spi)
{
    dev->driver.spi = spi;
    dev->range = 0;
}

void l3gd20_setup(l3gd20_t *dev, uint8_t range, uint8_t rate)
{
    uint8_t ctrl[5];

    dev->range = range;
    dev->rate = rate;

    ctrl[0] = L3GD20_CTRL_REG1_ZEN | L3GD20_CTRL_REG1_XEN | L3GD20_CTRL_REG1_YEN |
              L3GD20_CTRL_REG1_RATE(rate) | L3GD20_CTRL_REG1_BW1 |
              L3GD20_CTRL_REG1_BW0 | L3GD20_CTRL_REG1_PD;
    ctrl[1] = 0;
    // enable dataready interrupt
    ctrl[2] = L3GD20_CTRL_REG3_I2_DRDY;
    ctrl[3] = L3GD20_CTRL_REG4_RANGE(dev->range);
    // fifo enable, no filter
    ctrl[4] = L3GD20_CTRL_REG5_FIFO_EN;

    l3gd20_reg_write_multi(dev, L3GD20_RA_CTRL_REG1, ctrl, sizeof(ctrl));

    l3gd20_reg_write(dev, L3GD20_RA_FIFO_CTRL_REG, L3GD20_FIFO_CTRL_REG_FM_STREAM_MODE);

    l3gd20_reg_write(dev, L3GD20_RA_INT1_CFG, 0);
}

bool l3gd20_ping(l3gd20_t *dev)
{
    return l3gd20_reg_read(dev, L3GD20_RA_WHO_AM_I) == L3GD20_WHO_AM_I_VALUE;
}

void l3gd20_read(l3gd20_t *dev, float *gyro, float *temp)
{
    int16_t raw_xyz[3] = {0,0,0};

    l3gd20_reg_read_multi(dev, L3GD20_RA_OUT_X_L, (uint8_t *)raw_xyz, sizeof(raw_xyz));

    float gain = l3gd20_gain[dev->range];

    gyro[0] = ((float) raw_xyz[0]) * gain;
    gyro[1] = ((float) raw_xyz[1]) * gain;
    gyro[2] = ((float) raw_xyz[2]) * gain;

    if (temp != NULL) {
        int8_t raw_temp = l3gd20_reg_read(dev, L3GD20_RA_OUT_TEMP);
        *temp = CELSIUS_TO_KELVIN((float) raw_temp);
    }
}
