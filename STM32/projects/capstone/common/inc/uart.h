#ifndef UART_H
#define UART_H

#include <inttypes.h>

typedef enum ByteMode {
  BYTE_MODE_ASCII,
  BYTE_MODE_HEX,
} ByteMode;

void uart2_tx_init(void);
void uart2_tx_deinit();

void print_sequential_bytes(uint8_t *src, int length, ByteMode mode);

#endif /* UART_H */
