/**
 * @file uart.c
 *
 * \copyright Copyright 2013 /Dev. All rights reserved.
 * \license This project is released under MIT license.
 *
 * @author Ferdi van der Werf <efcm@slashdev.nl>
 * @since 0.2.0
 */

#include "uart.h"

// Do we want UART?
#ifdef COM_UART

// Check if F_CPU is set
#ifndef F_CPU
#error F_CPU is not set but required for UART
#endif // F_CPU

// Define buffer ring size if not defined
#ifndef COM_UART_BUFFER_RING_SIZE
#define COM_UART_BUFFER_RING_SIZE 64
#endif // COM_UART_BUFFER_RING_SIZE

// Buffer ring struct
typedef struct ringbuffer {
    uint8_t buffer[COM_UART_BUFFER_RING_SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
} ringbuffer_t;

// Create buffer ring
ringbuffer_t com_uart_tx_buffer_ring = { { 0 }, 0, 0 };

void uart_init(uart_config_t *config) {
    // Disable transmit and receive
    UCSR0B &= ~((1 << RXEN0) | (1 << TXEN0));
    
    // Set UCSR0A register
    // Set double speed and calculate brr divider
    if (config->doublespeed) {
        UCSR0A |= (1 << U2X0);
		// Calculate baudrate divider
		UBRR0 = ((F_CPU / (8UL * config->baudrate)) - 1);
    } else {
        UCSR0A &= ~(1 << U2X0);
		// Calculate baudrate divider
		UBRR0 = ((F_CPU / (16UL * config->baudrate)) - 1);
    }
    
    // Set multi-processor mode
    if (config->multiprocessormode) {
        UCSR0A |= (1 << MPCM0);
    } else {
        UCSR0A &= ~(1 << MPCM0);
    }
    
    // Set UCSR0B register
    // Receive enable/disable
    if (config->enable_rx) {
        UCSR0B |= (1 << RXEN0);
    } else {
        UCSR0B &= ~(1 << RXEN0);
    }
    
    // Transmit enable/disable
    // Set interrupt accordingly
    if (config->enable_tx) {
        UCSR0B |= (1 << TXEN0) | (1 << UDRIE0);
    } else {
        UCSR0B &= ~((1 << TXEN0) | (1 << UDRIE0));
    }
    
}

void uart_get_default_config(uart_config_t *config) {
    // Make sure we have a valid pointer
    if (!config)
        return;
    
    config->baudrate = 9600;
    config->doublespeed = 0;
    config->multiprocessormode = 0;
    config->enable_rx = 1;
    config->enable_tx = 1;
}

// Send if tx buffer has contents
ISR(USART0_UDRE_vect) {
    if (com_uart_tx_buffer_ring.head == com_uart_tx_buffer_ring.tail) {
        // Buffer empty, disable interrupt
        UCSR0B &= ~(1 << UDRIE0);
    } else {
        // Get data to send
        uint8_t value = com_uart_tx_buffer_ring.buffer[com_uart_tx_buffer_ring.tail];
        com_uart_tx_buffer_ring.tail = (com_uart_tx_buffer_ring.tail + 1) % COM_UART_BUFFER_RING_SIZE;
        // Send next byte
        UDR0 = value;
    }
}

void uart_send(uint8_t data) {
    // Get index + 1 of buffer head
    uint8_t index = (com_uart_tx_buffer_ring.head + 1) % COM_UART_BUFFER_RING_SIZE;
    
    // If the tx buffer is full, wait until interrupt handler empties
    // a slot in the buffer to write to.
    while (index == com_uart_tx_buffer_ring.tail) {
        continue;
    }
    
    // Write the byte
    com_uart_tx_buffer_ring.buffer[com_uart_tx_buffer_ring.head] = data;
    com_uart_tx_buffer_ring.head = index;
    
    // Set data registry empty bit
    UCSR0B |= (1 << UDRIE0);
    // Clear the transmit complete bit
    UCSR0A &= ~(1 << TXC0);
}

void uart_send_string(char *data) {
    while (*data) {
        uart_send(*data++);
    }
}

void uart_send_string_p(const char *pdata) {
    char c;
    while ((c = pgm_read_byte(pdata++))) {
        uart_send(c);
    }
}

#endif // COM_UART
