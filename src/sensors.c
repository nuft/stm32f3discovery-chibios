#include <ch.h>
#include "chprintf.h"
#include "l3gd20.h"
#include "sensors.h"

float l3gd20_gyro_rate[3];

#define L3GD20_INT_EVENT_MASK ~(1<<EXTI_EVENT_L3GD20_INT)

static THD_WORKING_AREA(sensors_wa, 512);
static THD_FUNCTION(sensors, arg)
{
    (void)arg;
    chRegSetThreadName("sensors");

    /*
     * SPI1 configuration structure.
     * SPI1 is on APB2 @ 36MHz / 4 = 9MHz
     * CPHA=1, CPOL=1, 8bits frames, MSb transmitted first.
     */
    static SPIConfig spi_cfg = {
        .end_cb = NULL,
        .ssport = GPIOE,
        .sspad = GPIOE_SPI1_CS,
        .cr1 =  SPI_CR1_BR_0 | SPI_CR1_CPOL | SPI_CR1_CPHA
    };

    static l3gd20_t l3gd20;
    l3gd20_init_using_spi(&l3gd20, &SPID1);

    spiStart(&SPID1, &spi_cfg);
    if (!l3gd20_ping(&l3gd20)) {
        return 0;
    }

    l3gd20_setup(&l3gd20, L3GD20_RANGE_500DPS, L3GD20_DATA_RATE_760HZ);

    while (1) {
        float gyro[3];
        if (palReadPad(GPIOE, GPIOE_L3GD20_INT2)) {
            l3gd20_read(&l3gd20, &gyro[0], NULL);
            chSysLock();
            l3gd20_gyro_rate[0] = gyro[0];
            l3gd20_gyro_rate[1] = gyro[1];
            l3gd20_gyro_rate[2] = gyro[2];
            chSysUnlock();
        }
        chThdSleepMilliseconds(10);
        // todo: use interrupt pin
    }
}

void sensors_start(void)
{
    chThdCreateStatic(sensors_wa, sizeof(sensors_wa), LOWPRIO, sensors, NULL);
}
