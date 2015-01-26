#include <ch.h>
#include <chprintf.h>
#include "shell.h"
#include "sensors.h"
#include "shell_commands.h"

#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2048)

static void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[])
{
    size_t n, size;

    (void)argv;
    if (argc > 0) {
        chprintf(chp, "Usage: mem\r\n");
        return;
    }
    n = chHeapStatus(NULL, &size);
    chprintf(chp, "core free memory : %u bytes\r\n", chCoreGetStatusX());
    chprintf(chp, "heap fragments   : %u\r\n", n);
    chprintf(chp, "heap free total  : %u bytes\r\n", size);
}

static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[])
{
    static const char *states[] = {CH_STATE_NAMES};
    thread_t *tp;

    (void)argv;
    if (argc > 0) {
        chprintf(chp, "Usage: threads\r\n");
        return;
    }
    chprintf(chp, "    addr    stack prio refs     state\r\n");
    tp = chRegFirstThread();
    do {
        chprintf(chp, "%08lx %08lx %4lu %4lu %9s\r\n",
                 (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
                 (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
                 states[tp->p_state]);
        tp = chRegNextThread(tp);
    } while (tp != NULL);
}

static void cmd_sensors(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    int count = 100;
    while (count-- > 0) {
        float g[3];
        chSysLock();
        g[0] = l3gd20_gyro_rate[0];
        g[1] = l3gd20_gyro_rate[1];
        g[2] = l3gd20_gyro_rate[2];
        chSysUnlock();

        chprintf(chp, "gyro: %d %d %d\n",
            (int)(1000*g[0]),
            (int)(1000*g[1]),
            (int)(1000*g[2]));
        chThdSleepMilliseconds(100);
    }
}

static const ShellCommand commands[] = {
    {"mem", cmd_mem},
    {"threads", cmd_threads},
    {"sensors", cmd_sensors},
    {NULL, NULL}
};

void shell_run(BaseSequentialStream *stream)
{
    ShellConfig shell_cfg1 = {
        stream,
        commands
    };
    shellInit();
    thread_t *shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
    while (!chThdTerminatedX(shelltp)) {
        chThdSleepMilliseconds(10);
    }
    chThdRelease(shelltp);
}
