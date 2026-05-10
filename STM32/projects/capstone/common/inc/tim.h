#ifndef TIM_H
#define TIM_H

#include <inttypes.h>
#include <stdbool.h>

void systick_init(void);
void systick_deinit(void);
uint32_t get_tick_ms(void);

#endif /* TIM_H */