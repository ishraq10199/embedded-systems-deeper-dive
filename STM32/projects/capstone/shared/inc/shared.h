#ifndef SHARED_H
#define SHARED_H

#include <inttypes.h>

typedef struct SharedData_t SharedData_t;

uint8_t shared_data_get_boot_count(void);
void shared_data_increment_boot_count(void);
void shared_data_reset_boot_count(void);

#endif /* SHARED_H */
