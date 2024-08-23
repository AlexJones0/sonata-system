// Copyright lowRISC Contributors.
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>
#include "sonata_system.h"

#define ADC_REG_CONFIG_1 0x41

#define DEV_READ(addr) (*((volatile uint32_t *)(addr)))
#define DEV_WRITE(addr, val) (*((volatile uint32_t *)(addr)) = val)
#define ADC_FROM_ADDR_AND_OFFSET_HW(addr, offset) ((uint32_t *)(addr + 4 * offset))

int main(void) {
    uart_init(DEFAULT_UART);
    putstr("\nBefore: ");
    puthexn(DEV_READ(ADC_FROM_ADDR_AND_OFFSET_HW(ADC_BASE, ADC_REG_CONFIG_1)), 8);
    //DEV_WRITE(ADC_FROM_ADDR_AND_OFFSET_HW(ADC_BASE, ADC_REG_CONFIG_0), 3);
    DEV_WRITE(ADC_FROM_ADDR_AND_OFFSET_HW(ADC_BASE, ADC_REG_CONFIG_1), (uint32_t) 0x00001234);
    putstr("\nAfter: ");
    puthexn(DEV_READ(ADC_FROM_ADDR_AND_OFFSET_HW(ADC_BASE, ADC_REG_CONFIG_1)), 8);
    putstr("\n");
    while (1) asm volatile("wfi");
    return 0;
}
