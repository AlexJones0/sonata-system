/**
 * Copyright lowRISC contributors.
 * Licensed under the Apache License, Version 2.0, see LICENSE for details.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "../../common/defs.h"
#include "../common/platform-pinmux.hh"
#include "../common/sonata-devices.hh"
#include <platform-spi.hh>
#include <ds/xoroshiro.h>
#include <cheri.hh>

using namespace CHERI;

// The full range currently used by Sonata's GPIO
#define GPIO_FULL_BOUNDS (0x0000'0040)

// Testing parameters
static constexpr bool PinmuxCheckFailImmediately = false;
static constexpr bool PinmuxEnableRetry          = true;
static constexpr uint32_t UartTimeoutMsec        = 10;
static constexpr uint32_t UartTestBytes          = 100;
static constexpr uint32_t GpioWaitMsec           = 1;
static constexpr uint32_t GpioTestLength         = 10;

static constexpr uint32_t CyclesPerMillisecond = CPU_TIMER_HZ / 1'000;
static constexpr char prefix[]                 = "[\x1b[35mPinmux Check\033[0m] ";

// A single 32-bit wide instance of Sonata's GPIO
struct SonataGpioInstance {
  uint32_t output;
  uint32_t input;
  uint32_t debounced_input;
  uint32_t output_enable;
};

// The full definition of Sonata's GPIO array
struct SonataGpioFull {
  SonataGpioInstance instances[4];
};

// Enum representing possible GPIO instances, and their ordering
enum class GpioInstance : uint8_t {
  General        = 0,
  RaspberryPiHat = 1,
  ArduinoShield  = 2,
  Pmod           = 3,
};

// A struct for specifying a specific Gpio Pin using its instance & bit
struct GpioPin {
  GpioInstance instance;  // 0-3 (general, rpi, arduino, pmod)
  uint8_t bit;            // 0-31
};

inline void set_gpio_output(Capability<volatile SonataGpioFull> gpio, GpioPin pin, bool value);
inline void set_gpio_output_enable(Capability<volatile SonataGpioFull> gpio, GpioPin pin, bool value);
inline bool get_gpio_input(Capability<volatile SonataGpioFull> gpio, GpioPin pin);

// Enum representing the different type of tests currently able to be implemented
// in a pinmux testplan.
enum class TestType {
  UartSendReceive  = 0,
  GpioWriteRead    = 1,
  I2cBH1745ReadId  = 2,
  SpiPmodSF3ReadId = 3,
};

// The test-specific data required to carry a UART send/receive test
struct UartTest {
  enum class UartNum : uint8_t {
    // Uart0 is used as a console and cannot be tested
    Uart1 = 1,
    Uart2 = 2,
    Uart3 = 3,
    Uart4 = 4,
  } uart;
  uint32_t timeout;
  uint32_t test_length;
};

// The test-specific data required to carry out a GPIO write/read test
struct GpioTest {
  GpioPin output_pin;
  GpioPin input_pin;
  uint32_t wait_time;
  uint32_t test_length;
};

// The test-specific data required to carry out an I2C BH1745 Read ID test
struct I2cTest {
  enum class I2cNum : uint8_t {
    I2c0 = 0,
    I2c1 = 1,
  } i2c;
};

// The test-specific data required to carry out a SPI Pmod SF3 Read ID test
struct SpiTest {
  enum class SpiNum : uint8_t {
    // Spi0-2 are the Flash, LCD and Ethernet respectively and cannot be tested.
    Spi3 = 3,
    Spi4 = 4,
  } spi;
  GpioPin cs_pin;
};

// An output pin assignment to be made via Pinmux for a test
struct OutputPinAssignment {
  SonataPinmux::OutputPin pin;
  uint8_t select;
};

// A block input assignment to be made via Pinmux for a test
struct BlockInputAssignment {
  SonataPinmux::BlockInput input;
  uint8_t select;
};

// A single pinmux test to be executed
struct Test {
  // The type of test being specified.
  TestType type;
  // The name of the test to print to the console
  const char *name;
  // Whether testplan execution should pause for manual intervention. If this
  // is set to true, then an `instruction` must be specified, which is the
  // message to print to the console to direct the user.
  bool manual_required;
  const char *instruction;
  // The output pin and block input assignments to pinmux before testing
  OutputPinAssignment *output_pins;
  uint32_t num_output_pins;
  BlockInputAssignment *block_inputs;
  uint32_t num_block_inputs;
  /**
   * TODO: Ideally we would union the below attributes to save space, but the lack of
   * `memcpy` makes that very difficult, i.e.
   *   union TestData {
   *     UartTest uart;
   *     GpioTest gpio;
   *     I2cTest i2c;
   *     SpiTest spi;
   *   } test_data;
   */
  // The data to use in the test. The attribute corresponding to the specified `type`
  // attribute must be filled in with appropriate data.
  UartTest uart_data;
  GpioTest gpio_data;
  I2cTest i2c_data;
  SpiTest spi_data;
  // True if the test is expected to pass, or false if it should fail.
  bool expected_result;
};

bool execute_testplan(Test *testplan, uint8_t NumTests, ds::xoroshiro::P32R8 &prng,
                      Capability<volatile SonataGpioFull> gpio, UartPtr console, UartPtr uarts[4],
                      Capability<volatile SonataSpi> spis[2], I2cPtr i2cs[2], SonataPinmux *pinmux);
