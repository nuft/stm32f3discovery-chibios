#include <ch.h>
#include <hal.h>
#include <chprintf.h>
#include "usbcfg.h"
#include "sensors.h"
#include <shell_commands.h>

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

int main(void) {
    halInit();
    chSysInit();

    // LED status thread
    chThdCreateStatic(waLEDThread, sizeof(waLEDThread), NORMALPRIO+1, LEDThread, NULL);

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

    sensors_start();

    while (1) {
        shell_run((BaseSequentialStream *)&SDU1);
        chThdSleepMilliseconds(100);
    }
}
