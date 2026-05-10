#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

/* Defined in `memory_map.ld` */
#include <inttypes.h>
#include <stdint.h>
extern uint32_t __bootrom_start__;
extern uint32_t __bootrom_size__;
extern uint32_t __appromA_start__;
extern uint32_t __appromA_size__;
extern uint32_t __appromB_start__;
extern uint32_t __appromB_size__;
extern uint32_t __metarom_start__;
extern uint32_t __shared_start__;

#endif /* MEMORY_MAP_H */
