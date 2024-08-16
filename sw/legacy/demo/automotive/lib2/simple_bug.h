// Copyright lowRISC Contributors.
// SPDX-License-Identifier: Apache-2.0

#ifndef AUTOMOTIVE_SIMPLE_BUG_H
#define AUTOMOTIVE_SIMPLE_BUG_H

#include <stdint.h>

typedef struct TaskOne {
    uint64_t acceleration;
    uint64_t braking;
    uint64_t speed;
} TaskOne;

typedef struct TaskTwo {
    uint64_t write[100];
} TaskTwo;

void init_simple_demo_mem(TaskOne *task_one, TaskTwo *task_two);
void run_simple_demo(uint64_t init_time);

#endif // AUTOMOTIVE_SIMPLE_BUG_H
