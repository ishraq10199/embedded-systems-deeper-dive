#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

/* Defined in `memory_map.ld` */
#include <inttypes.h>
extern uint32_t __bootrom_start__;
extern uint32_t __bootrom_size__;
extern uint32_t __approm_start__;
extern uint32_t __approm_size__;
extern uint32_t __shared_start__;

#endif /* MEMORY_MAP_H */
