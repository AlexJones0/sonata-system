// Copyright lowRISC Contributors.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>

#include "./automotive.c"

#include "sonata_system.h"
#include "gpio.h"
#include "timer.h"
#include "rv_plic.h"
#include "spi.h"
#include "ksz8851.h"

static const bool dual_uart = true;
static uart_t uart0, uart1;

static inline void write_both_uart(char ch0, char ch1) {
  uart_out(uart0, ch0);
  uart_out(uart1, ch1);
}

void write_to_uart(const char *__restrict__ __format, ...) {
    arch_local_irq_disable();
    char __buffer[1024];
    char *__restrict__ __buf_ptr = &*__buffer;
    va_list args;
    va_start(args, __format);
    vsnprintf(__buf_ptr, 1024, __format, args);
    va_end(args);
    putstr(__buf_ptr);
    arch_local_irq_enable();
}

void lcd_draw_str(uint32_t x, uint32_t y, const char *format, uint32_t bg_color, uint32_t fg_color, ...) {
    /*// TODO modularise this code, its repeated from the UART stuff, and a huge mess
    char __buffer[1024];
    char *__restrict__ __buf_ptr = &*__buffer;
    va_list args;
    va_start(args, fg_color);
    vsnprintf(__buf_ptr, 1024, format, args);
    va_end(args);

    lcd->draw_str({x, y}, __buffer, static_cast<Color>(bg_color), static_cast<Color>(fg_color));*/
    // TODO implement
}

void lcd_clean(uint32_t color) {
    //lcd->clean(static_cast<Color>(color));
    // TODO implement
}

void lcd_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    /*lcd->fill_rect(Rect::from_point_and_size({x, y}, {w, h}), 
                   static_cast<Color>(color));*/
    // TODO implement
}

void lcd_draw_img(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint8_t *data) {
    //lcd->draw_image_rgb565(Rect::from_point_and_size({x, y}, {w, h}), data);
    // TODO implement
}

uint64_t wait(const uint64_t wait_for) {
    uint64_t cur_time = get_elapsed_time();
    while (cur_time < wait_for) {
        cur_time = get_elapsed_time();
        // Busy wait
    }
    return cur_time;
}

void null_callback(void) {};

struct netif interface;

typedef struct EthernetHeader {
    uint8_t mac_destination[6];
    uint8_t mac_source[6];
    uint8_t type[2];
} __attribute__((__packed__)) EthernetHeader;

void send_ethernet_frame(const uint64_t *buffer, uint16_t length) {
    if (length > (100 / 8)) {
        length = 100 / 8;
    }
    uint8_t transmit_buf[128];
    EthernetHeader header = {
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        {0x3a, 0x30, 0x25, 0x24, 0xfe, 0x7a},
        {0x08, 0x06},
    };
    for (uint32_t i = 0; i < 14; i++) {
        transmit_buf[i] = header.mac_destination[i];
    }
    for (uint32_t i = 0; i < length; ++i) {
        transmit_buf[14+i*8+0] = (buffer[i] >> 56) & 0xFF;
        transmit_buf[14+i*8+1] = (buffer[i] >> 48) & 0xFF;
        transmit_buf[14+i*8+2] = (buffer[i] >> 40) & 0xFF;
        transmit_buf[14+i*8+3] = (buffer[i] >> 32) & 0xFF;
        transmit_buf[14+i*8+4] = (buffer[i] >> 24) & 0xFF;
        transmit_buf[14+i*8+5] = (buffer[i] >> 16) & 0xFF;
        transmit_buf[14+i*8+6] = (buffer[i] >> 8 ) & 0xFF;
        transmit_buf[14+i*8+7] = buffer[i] & 0xFF;
    }

    struct fbuf buf = {
        (void *) transmit_buf,
        (int8_t) 14 + length * 8,
    };
    
    if (ksz8851_output(&interface, &buf) != 0) {
        write_to_uart("Error sending frame...\n");
    }
}

__attribute__((section(".data.__contiguous.__task_two")))
static TaskTwo mem_task_two = {
	.write = {0},
};

__attribute__((section(".data.__contiguous.__task_one"))) 
static TaskOne mem_task_one = {
	.acceleration = 12,
	.braking = 59,
	.speed = 30,
};

// Thread entry point
int main(void) 
{
    // Initialise UART when running in legacy mode
    if (dual_uart) {
        const char *signon = "automotive demo application; UART ";
        uart0 = UART_FROM_BASE_ADDR(UART0_BASE);
        uart1 = UART_FROM_BASE_ADDR(UART1_BASE);
        uart_init(uart0);
        uart_init(uart1);
        // Send a sign-on message to both UARTs.
        char ch;
        while ('\0' != (ch = *signon)) {
            write_both_uart(ch, ch);
            signon++;
        }
        write_both_uart('0', '1');
        write_both_uart('\r', '\r');
        write_both_uart('\n', '\n');
    } else {
        uart0 = DEFAULT_UART;
        uart_init(uart0);
    }

    // Initialise the timer when running in legacy mode
    timer_init();
    timer_enable(SYSCLK_FREQ / 1000);

    // Initialise Ethernet support
    rv_plic_init();
    spi_t spi;
    spi_init(&spi, ETH_SPI, /*speed=*/0);
    interface.spi = &spi;
    uint8_t mac_source[6] = {0x3a, 0x30, 0x25, 0x24, 0xfe, 0x7a};
    ksz8851_init(&interface, mac_source);

    // Wait for a physical link
    if (!ksz8851_get_phy_status(&interface)) {
        write_to_uart("Waiting for a good physical ethernet link...\n");
    }
    while (!ksz8851_get_phy_status(&interface)) {
        wait(get_elapsed_time() + 50);
    }
    wait(get_elapsed_time() + 2500);

    // Verify that for the purpose of a reproducible error in our demo, that
    // task one and task two have been assigned contiguous memories as required.
    write_to_uart("task_two_mem_location: %u\n", (unsigned int) &mem_task_two);
    write_to_uart("task_two_mem_size: %u\n", (unsigned int) sizeof(mem_task_two));
    write_to_uart("task_one_mem_location: %u\n", (unsigned int) &mem_task_one);
    write_to_uart("task_one_mem_size: %u\n", (unsigned int) sizeof(mem_task_one));
    assert((uint32_t) &mem_task_two + sizeof(mem_task_two) == (uint32_t) &mem_task_one);

    // Adapt common automotive library for legacy drivers 
    init_mem(&mem_task_one, &mem_task_two);
    init_uart_callback(write_to_uart);
    init_wait_callback(100, wait);
    init_time_callback(get_elapsed_time);
    init_lcd(0, 0); // TODO implement
    init_lcd_draw_str_callback(lcd_draw_str);
    init_lcd_clean_callback(lcd_clean);
    init_lcd_fill_rect_callback(lcd_fill_rect);
    init_lcd_draw_img_callback(lcd_draw_img);
    init_loop_callback(null_callback);
    init_ethernet_transmit_callback(send_ethernet_frame);

    // Run automotive demo
	run(get_elapsed_time());
    
    // Legacy driver cleanup
    timer_disable();
    asm volatile("wfi");
    return 0;
}
