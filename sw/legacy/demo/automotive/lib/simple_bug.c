// Copyright lowRISC Contributors.
// SPDX-License-Identifier: Apache-2.0

#include "simple_bug.h"
#include "automotive_common.h"


static TaskOne *task_one_mem;
static TaskTwo *task_two_mem;

static bool first_call_to_task = true;


void init_simple_demo_mem(TaskOne *task_one, TaskTwo *task_two) {
    task_one_mem = task_one;
    task_two_mem = task_two;
}

static void send_pedal_frame(const uint64_t *data, uint16_t length) {
    EthernetHeader header = {
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        {0x3a, 0x30, 0x25, 0x24, 0xfe, 0x7a},
        {0x08, 0x06},
    };
    send_frame(data, header, length);
}

static bool task_one() {
    callbacks.uart_send("Sending pedal data: acceleration=%u, braking=%u.\n", 
        (unsigned int) task_one_mem->acceleration, 
        (unsigned int) task_one_mem->braking);
    const uint64_t frame_data[2] = {
        task_one_mem->acceleration, 
        task_one_mem->braking
    };
    send_pedal_frame(frame_data, 2);
    return true;
}

// Even though we this function is not exported, do not declare this function 
// as static, as otherwise the PCC modifying trick used to catch the CHERI
// exceptions without compartmentalising will not work correctly.
bool task_two() {
    static uint64_t counter;
    if (first_call_to_task) {
        counter = 0;
        first_call_to_task = false;
    }
    counter += 1;
    callbacks.uart_send("Calling task two update, count = %u\n", (unsigned int) counter);
    callbacks.lcd.draw_str(10, 20, "counter = %u;", 0x000000, 0xCCCCCC, (unsigned int) counter);
    callbacks.lcd.draw_str(10, 30, "if (counter <= 100) {", 0x000000, 0xCCCCCC);
    uint32_t text_color = (counter >= 100) ? 0x0000FF : 0xCCCCCC;
    callbacks.lcd.draw_str(10, 40, "    write[counter] = 1000;", 0x000000, text_color);
    callbacks.lcd.draw_str(10, 50, "}", 0x000000, 0xCCCCCC);
    if (counter == 100) {
        callbacks.uart_send("counter == 100: triggering a bug\n");
    }
    if (counter <= 100) {
        task_two_mem->write[counter] = 1000;
    }
    return true;
}

void run_simple_demo(uint64_t init_time)
{
    callbacks.start();
    task_one_mem->acceleration = 15;
    task_one_mem->braking = 2;
    task_one_mem->speed = 0;
    first_call_to_task = true;

	callbacks.uart_send("Automotive demo started!\n");
    callbacks.lcd.draw_str(10, 10, "uint64_t write[100];", 0x000000, 0xCCCCCC);
    uint64_t last_elapsed_time = init_time;
    for (uint32_t i = 0; i < 175; i++) {
        task_one();
        task_two();
        last_elapsed_time = callbacks.wait(last_elapsed_time + callbacks.wait_time);
        callbacks.loop();
    }
    callbacks.uart_send("Automotive demo ended!\n");
}
