// Copyright lowRISC Contributors.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>

#include "lib/automotive_common.h"
#include "lib/automotive_menu.h"
#include "lib/no_pedal.h"
#include "lib/joystick_pedal.h"
#include "lib/digital_pedal.h"

#include "sonata_system.h"
#include "gpio.h"
#include "timer.h"
#include "rv_plic.h"
#include "spi.h"
#include "adc.h"
#include "core/m3x6_16pt.h"
#include "st7735/lcd_st7735.h"
#include "ksz8851.h"
#include "lcd.h"

static const bool dual_uart = true;
static uart_t uart0, uart1;

St7735Context lcd;

static inline void write_both_uart(char ch0, char ch1) {
  uart_out(uart0, ch0);
  uart_out(uart1, ch1);
}

void write_to_uart(const char *__restrict__ __format, ...) {
    arch_local_irq_disable();
    char __buffer[1024];
    va_list args;
    va_start(args, __format);
    vsnprintf(__buffer, 1024, __format, args);
    va_end(args);
    putstr(__buffer);
    arch_local_irq_enable();
}

void lcd_draw_str(uint32_t x, uint32_t y, const char *format, uint32_t bg_color, uint32_t fg_color, ...) {
    char __buffer[1024];
    va_list args;
    va_start(args, fg_color);
    vsnprintf(__buffer, 1024, format, args);
    va_end(args);

    LCD_Point pos = {x, y};
	lcd_st7735_set_font(&lcd, &m3x6_16ptFont); 
    lcd_st7735_set_font_colors(&lcd, (uint32_t) bg_color, (uint32_t) fg_color);
    lcd_st7735_puts(&lcd, pos, __buffer);
}

void lcd_clean(uint32_t color) {
    size_t w, h;
    lcd_st7735_get_resolution(&lcd, &h, &w);
    LCD_rectangle rect = {(LCD_Point){0, 0}, w, h};
    lcd_st7735_fill_rectangle(&lcd, rect, color);
}

void lcd_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    LCD_rectangle rect = {(LCD_Point){x, y}, w, h};
    lcd_st7735_fill_rectangle(&lcd, rect, color);
}

void lcd_draw_img(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint8_t *data) {
    LCD_rectangle rect = {(LCD_Point){x, y}, w, h};
    lcd_st7735_draw_rgb565(&lcd, rect, data);
}

uint8_t read_joystick(void) {
    return ((uint8_t) read_gpio(GPIO_IN_DBNC)) & 0x1f;
}

bool read_pedal_digital(void) {
    return (read_gpio(GPIO_IN_DBNC) & (1 << 13)) > 0;
}

uint32_t read_pedal_analogue(void) {
    // TODO after implementing the ADC driver
    return 0;
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

struct netif eth_interface;

void send_ethernet_frame(const uint8_t *buffer, uint16_t length) {
    struct fbuf buf = {
        (void *) buffer,
        (uint8_t) length,
    };
    if (ksz8851_output(&eth_interface, &buf) != 0) {
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
	.braking = 2,
	.speed = 0,
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

    // ADC testing!
    // We set the clock divider to sample 500 Kilo samples per second (KSPS)
    ADCClockDivider adc_divider = (SYSCLK_FREQ / ADC_MAX_SAMPLES) * 2;
    adc_t adc;
    adc_init(
        &adc, 
        ADC_FROM_BASE_ADDR(ADC_BASE),
        ADC_FROM_ADDR_AND_OFFSET(
            ADC_BASE,
            ADC_REG_VAUX_P_N_14),
        ADC_CHANNEL_VAUX_P_N_14,
        ADC_AVERAGING_256_SAMPLES,
        adc_divider,
        false,
        false
    );
    while (true) {
        write_to_uart("ADC value: %d\n", read_adc(&adc));
        wait(get_elapsed_time() + 50);
    }

    // Initialise Ethernet support
    rv_plic_init();
    spi_t spi;
    spi_init(&spi, ETH_SPI, /*speed=*/0);
    eth_interface.spi = &spi;
    uint8_t mac_source[6];
    for (uint8_t i = 0; i < 6; i++) {
        mac_source[i] = FixedDemoHeader.mac_source[i];
    }
    ksz8851_init(&eth_interface, mac_source);

    // Initialise LCD display drivers
    LCD_Interface lcd_interface;
    spi_t lcd_spi;
    spi_init(&lcd_spi, LCD_SPI, LcdSpiSpeedHz);
    lcd_init(&lcd_spi, &lcd, &lcd_interface);

    // Wait for a physical link
    if (!ksz8851_get_phy_status(&eth_interface)) {
        write_to_uart("Waiting for a good physical ethernet link...\n");
        lcd_clean(ColorBlack);
        lcd_draw_str(
            (lcd.parent.width / 2) - 55,
            (lcd.parent.height / 2) - 5,
            "Waiting for a good physical",
            ColorBlack,
            ColorWhite
        );
        lcd_draw_str(
            (lcd.parent.width / 2) - 30,
            (lcd.parent.height / 2) + 5,
            "ethernet link...",
            ColorBlack,
            ColorWhite
        );
    }
    while (!ksz8851_get_phy_status(&eth_interface)) {
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
    init_lcd(lcd.parent.width, lcd.parent.height);
    init_callbacks((Automotive_Callbacks) {
        .uart_send = write_to_uart,
        .wait = wait,
        .wait_time = 90,
        .time = get_elapsed_time,
        .loop = null_callback,
        .start = null_callback,
        .joystick_read = read_joystick,
        .digital_pedal_read = read_pedal_digital,
        .analogue_pedal_read = read_pedal_analogue,
        .ethernet_transmit = send_ethernet_frame,
        .lcd = {
            .draw_str = lcd_draw_str,
            .clean = lcd_clean,
            .fill_rect = lcd_fill_rect,
            .draw_img_rgb565 = lcd_draw_img,
        },
    });

    DemoApplication option;
    while (true) {
        // Run demo selection
        option = select_demo();

        // Run automotive demo
        switch (option) {
            case NoPedal:
                // Run simple timed demo with no pedal & using passthrough
                init_no_pedal_demo_mem(&mem_task_one, &mem_task_two);
                run_no_pedal_demo(get_elapsed_time());
                break;
            case JoystickPedal:
                // Run demo using a joystick as a pedal, with passthrough
                init_joystick_demo_mem(&mem_task_one, &mem_task_two);
                run_joystick_demo(get_elapsed_time());
                break;
            case DigitalPedal:
                // Run demo using an actual physical pedal but taking
                // a digital signal, using simulation instead of passthrough
                init_digital_pedal_demo_mem(&mem_task_one, &mem_task_two);
                run_digital_pedal_demo(get_elapsed_time());
                break;
            case AnaloguePedal:
                // TODO not implemented yet
                write_to_uart("This demo is not yet implemented. It requires an ADC");
                // Run demo using an actual physical pedal, taking an
                // analogue signal via an ADC, with passthrough.
                break;

        }
    }

    // Legacy driver cleanup
    timer_disable();
    asm volatile("wfi");
    return 0;
}
