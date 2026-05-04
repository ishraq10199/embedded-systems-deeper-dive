#ifndef UART_H
#define UART_H

#include <inttypes.h>
#include <stdbool.h>

typedef enum ByteMode {
  BYTE_MODE_ASCII,
  BYTE_MODE_HEX,
} ByteMode;

void uart2_tx_init(void);
void uart2_tx_deinit();
bool uart_initialized(void);

void print_sequential_bytes(uint8_t *src, int length, ByteMode mode);
void print_sequential_words(uint32_t *src, int length);

#endif /* UART_H */
