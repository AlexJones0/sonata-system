// Copyright lowRISC Contributors.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include <lwip/init.h>
#include <lwip/netif.h>
#include <lwip/dhcp.h>
#include <lwip/timeouts.h>
#include <netif/ethernet.h>

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

uint64_t wait(const uint64_t wait_for) {
    uint64_t cur_time = get_elapsed_time();
    while (cur_time < wait_for) {
        cur_time = get_elapsed_time();
        // Busy wait
    }
    return cur_time;
}

__attribute__((section(".data.__contiguous.__task_two")))
static TaskTwo mem_task_two = {
	.write = {0},
};

__attribute__((section(".data.__contiguous.__task_one"))) 
static TaskOne mem_task_one = {
	.acceleration = 0,
	.braking = 0,
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

    // TODO when implementing ethernet on legacy
    // Make my own `struct netif` and populate it myself
    // This involves making my own `netif->input` callback function also.
    // Finally, I apparently also need to handle manually allocating and 
    // freeing the pbuf also?

    // Initialise the timer when running in legacy mode
    timer_init();
    timer_enable(SYSCLK_FREQ / 5);

    // Initialise Ethernet support
    rv_plic_init();
    spi_t spi;
    spi_init(&spi, ETH_SPI, /*speed=*/0);
    struct netif netif;
    netif_add(&netif, IP_ADDR_ANY, IP4_ADDR_ANY, IP4_ADDR_ANY, &spi, ksz8851_init, ethernet_input);
    netif.name[0] = 'e';
    netif.name[1] = '0';
    netif_set_default(&netif);
    netif_set_up(&netif);
    netif_set_link_up(&netif);
    dhcp_start(&netif);
    
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
    init_wait_callback(1, wait);
    init_loop_callback(sys_check_timeouts);

    // Run automotive demo
	run(get_elapsed_time());
    
    // Legacy driver cleanup
    timer_disable();
    asm volatile("wfi");
    return 0;
}
