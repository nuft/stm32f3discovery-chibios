#include <ch.h>
#include <hal.h>
#include <chprintf.h>
#include "usbcfg.h"
#include "sensors.h"
#include "shell_cmds.h"
#include "uart/blocking_uart_driver.h"

SerialUSBDriver SDU1;

#define GPIOE_LED_STATUS GPIOE_LED7_GREEN

static THD_WORKING_AREA(waLEDThread, 128);
static THD_FUNCTION(LEDThread, arg) {
    (void)arg;
    chRegSetThreadName("LED status");
    while (true) {
        palSetPad(GPIOE, GPIOE_LED_STATUS);
        chThdSleepMilliseconds(80);
        palClearPad(GPIOE, GPIOE_LED_STATUS);
        chThdSleepMilliseconds(80);
        palSetPad(GPIOE, GPIOE_LED_STATUS);
        chThdSleepMilliseconds(80);
        palClearPad(GPIOE, GPIOE_LED_STATUS);
        chThdSleepMilliseconds(760);
    }
    return 0;
}

static BaseSequentialStream *usb_cdc_init(void)
{
    // USB CDC
    sduObjectInit(&SDU1);
    sduStart(&SDU1, &serusbcfg);
    // usbDisconnectBus(serusbcfg.usbp);
    chThdSleepMilliseconds(1000);
    usbStart(serusbcfg.usbp, &usbcfg);
    // usbConnectBus(serusbcfg.usbp);
    while (SDU1.config->usbp->state != USB_ACTIVE) {
        chThdSleepMilliseconds(10);
    }
    return (BaseSequentialStream *) &SDU1;
}

int main(void) {
    halInit();
    chSysInit();

    // LED status thread
    chThdCreateStatic(waLEDThread, sizeof(waLEDThread), NORMALPRIO+1, LEDThread, NULL);

    // BaseSequentialStream *bs = usb_cdc_init();

    // sensors_start();

    // sdStart(&SD2, NULL);
    // BaseSequentialStream *bs = (BaseSequentialStream *)&SD2;

    // BlockingUARTDriver bu;
    // blocking_uart_init(&bu, USART2, 115200);
    // BaseSequentialStream *bs = (BaseSequentialStream *)&bu;

    // chprintf(bs, "hello world\n");

    while (1) {
        // shell_run(bs);
        chThdSleepMilliseconds(100);
    }
}
