#ifndef EXTI_H
#define EXTI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch.h>

#define EXTI_EVENT_NRF_IRQ (1<<0)

extern event_source_t exti_events;

void exti_setup(void);

#ifdef __cplusplus
}
#endif

#endif /* EXTI_H */