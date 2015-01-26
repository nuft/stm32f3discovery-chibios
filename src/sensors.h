#ifndef SENSORS_H
#define SENSORS_H

#ifdef __cplusplus
extern "C" {
#endif

extern float l3gd20_gyro_rate[3];

void sensors_start(void);

#ifdef __cplusplus
}
#endif

#endif /* SENSORS_H */