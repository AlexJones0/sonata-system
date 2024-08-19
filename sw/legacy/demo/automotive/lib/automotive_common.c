// Copyright lowRISC Contributors.
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "automotive_common.h"

LCD_Size lcdSize, lcdCentre;

Automotive_Callbacks callbacks;

void init_lcd(uint32_t size_x, uint32_t size_y) {
    lcdSize.x = size_x;
    lcdSize.y = size_y;
    lcdCentre.x = size_x / 2;
    lcdCentre.y = size_y / 2;
}

void init_callbacks(Automotive_Callbacks auto_callbacks) {
    callbacks = auto_callbacks;
}

bool joystick_in_direction(uint8_t joystick, enum JoystickDir direction) {
    return (joystick & ((uint8_t) direction)) > 0;
};

void send_frame(const uint64_t *data, EthernetHeader header, uint16_t length) {
    assert(length < (100 / 8));  // TODO could up length to about 1000 bytes?
    uint8_t frame_buf[128];
    uint8_t frame_len = 0;
    for (uint8_t i = 0; i < 14; ++i) {
        frame_buf[frame_len++] = header.mac_destination[i];
    }
    for (uint8_t i = 0; i < length; ++i) {
        for (int8_t j = 7; j >= 0; --j) {
            frame_buf[frame_len++] = (data[i] >> (8 * j)) & 0xFF;
        }
    }
    callbacks.ethernet_transmit(frame_buf, frame_len);
}
