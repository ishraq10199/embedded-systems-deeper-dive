#ifndef UART_H
#define UART_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum ByteMode {
  BYTE_MODE_ASCII,
  BYTE_MODE_HEX,
} ByteMode;

void uart_tx_init(void);
void uart_tx_deinit(void);

void uart_tx_rx_init(void);
void uart_tx_rx_deinit(void);

bool uart_tx_initialized(void);
bool uart_tx_rx_initialized(void);
bool uart_rx_initialized(void);

void print_sequential_bytes(uint8_t *src, int length, ByteMode mode);
void print_sequential_words(uint32_t *src, int length);

void uart_send_byte(uint8_t ch);
void uart_rx_enable_echo(void);
void uart_rx_disable_echo(void);

int uart_read_byte(uint8_t *dest, uint32_t timeoutMs);
int uart_read_word(uint32_t *dest, uint32_t timeoutMs);
int uart_read_word_stream(uint32_t *dest, uint32_t len, uint32_t timeoutMs);

#endif /* UART_H */
