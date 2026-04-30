#include "shared.h"

extern uint32_t __shared_start__;

#pragma pack(push)

typedef struct SharedData {
    uint8_t boot_count;
} SharedData;

#pragma pack(pop)

static SharedData *sd = (SharedData *)&__shared_start__;

uint8_t shared_data_get_boot_count(void) {
    return sd->boot_count;
}
void shared_data_increment_boot_count(void) {
    sd->boot_count++;
}
void shared_data_reset_boot_count(void) {
    sd->boot_count = 0;
}