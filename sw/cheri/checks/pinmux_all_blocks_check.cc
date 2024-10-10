/**
 * Copyright lowRISC contributors.
 * Licensed under the Apache License, Version 2.0, see LICENSE for details.
 * SPDX-License-Identifier: Apache-2.0
 */

#define CHERIOT_NO_AMBIENT_MALLOC
#define CHERIOT_NO_NEW_DELETE
#define CHERIOT_PLATFORM_CUSTOM_UART

#include "pinmux_checker.hh"
#include "../common/uart-utils.hh"

using namespace CHERI;

#define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)))

/**
 * Run a pinmux check testplan which tests the use of the same PMOD pins for
 * GPIO, UART, I2C and SPI transmissions via manual intervention. This
 * provides confidence that the Pinmux is working as expected in the more
 * extreme scenario that all muxed blocks are actually being muxed over the
 * same pins.
 */
[[noreturn]] extern "C" void entry_point(void *rwRoot) {
  Capability<void> root{rwRoot};

  // Initialise capabilities for UART0 (the console), and all other UARTS (1-4)
  UartPtr uart0 = uart_ptr(rwRoot, 0);
  uart0->init(BAUD_RATE);
  UartPtr uarts[4] = {
      uart_ptr(rwRoot, 1),
      uart_ptr(rwRoot, 2),
      uart_ptr(rwRoot, 3),
      uart_ptr(rwRoot, 4),
  };
  for (UartPtr uart : uarts) {
    uart->init(BAUD_RATE);
  }

  // Create capabilities for SPI3&4, I2C0&1, GPIO and Pinmux for use in Pinmux testing
  Capability<volatile SonataSpi> spi3    = root.cast<volatile SonataSpi>();
  spi3.address()                         = SPI3_ADDRESS;
  spi3.bounds()                          = SPI_BOUNDS;
  Capability<volatile SonataSpi> spi4    = root.cast<volatile SonataSpi>();
  spi4.address()                         = SPI4_ADDRESS;
  spi4.bounds()                          = SPI_BOUNDS;
  Capability<volatile SonataSpi> spis[2] = {spi3, spi4};

  I2cPtr i2cs[2] = {i2c_ptr(root, 0), i2c_ptr(root, 1)};

  Capability<volatile SonataGpioFull> gpio = root.cast<volatile SonataGpioFull>();
  gpio.address()                           = GPIO_ADDRESS;
  gpio.bounds()                            = GPIO_FULL_BOUNDS;

  Capability<volatile uint8_t> pinmux = root.cast<volatile uint8_t>();
  pinmux.address()                    = PINMUX_ADDRESS;
  pinmux.bounds()                     = PINMUX_BOUNDS;
  SonataPinmux Pinmux                 = SonataPinmux(pinmux);

  // Initialise Pseudo-Random Number Generation for use in Pinmux UART testing
  ds::xoroshiro::P32R8 prng;
  prng.set_state(0xDEAD, 0xBEEF);

  // Define the Pin Output and Block Input Mux settings to be used in the Pinmux
  // testplan. We have to define these separately instead of using nested
  // initializers, as that requires `memcpy` to exist which we do not have.
  // Likewise, arrays with multiple pins/inputs must be individually set, or
  // we will get errors with `memcpy`.
  OutputPinAssignment pmod_test_gpio_on_pins[]     = {{SonataPinmux::OutputPin::Pmod0Io2, 4}};
  BlockInputAssignment pmod_test_gpio_on_inputs[]  = {{SonataPinmux::BlockInput::PmodGpio2, 1}};
  OutputPinAssignment pmod_test_gpio_off_pins[]    = {{SonataPinmux::OutputPin::Pmod0Io2, 0}};
  BlockInputAssignment pmod_test_gpio_off_inputs[] = {{SonataPinmux::BlockInput::PmodGpio2, 0}};

  OutputPinAssignment pmod_test_uart_on_pins[]     = {{SonataPinmux::OutputPin::Pmod0Io2, 1}};
  BlockInputAssignment pmod_test_uart_on_inputs[]  = {{SonataPinmux::BlockInput::UartReceive2, 2}};
  OutputPinAssignment pmod_test_uart_off_pins[]    = {{SonataPinmux::OutputPin::Pmod0Io2, 0}};
  BlockInputAssignment pmod_test_uart_off_inputs[] = {{SonataPinmux::BlockInput::UartReceive2, 0}};

  OutputPinAssignment pmod_test_i2c_on_pins[2];
  pmod_test_i2c_on_pins[0] = {SonataPinmux::OutputPin::Pmod0Io1, 1};  // Mux to I2C SDA
  pmod_test_i2c_on_pins[1] = {SonataPinmux::OutputPin::Pmod0Io2, 2};  // Mux to I2C SCL
  OutputPinAssignment pmod_test_i2c_off_pins[2];
  pmod_test_i2c_off_pins[0] = {SonataPinmux::OutputPin::Pmod0Io1, 0};
  pmod_test_i2c_off_pins[1] = {SonataPinmux::OutputPin::Pmod0Io2, 0};

  OutputPinAssignment pmod_test_spi_on_pins[3];
  pmod_test_spi_on_pins[0]                       = {SonataPinmux::OutputPin::Pmod0Io1, 2};  // Mux to GPIO for CS
  pmod_test_spi_on_pins[1]                       = {SonataPinmux::OutputPin::Pmod0Io2, 3};  // Mux to SPI COPI
  pmod_test_spi_on_pins[2]                       = {SonataPinmux::OutputPin::Pmod0Io4, 1};  // Mux to SPI Sck
  BlockInputAssignment pmod_test_spi_on_inputs[] = {{SonataPinmux::BlockInput::SpiReceive3, 3}};
  OutputPinAssignment pmod_test_spi_off_pins[3];
  pmod_test_spi_off_pins[0]                       = {SonataPinmux::OutputPin::Pmod0Io1, 0};
  pmod_test_spi_off_pins[1]                       = {SonataPinmux::OutputPin::Pmod0Io2, 0};
  pmod_test_spi_off_pins[2]                       = {SonataPinmux::OutputPin::Pmod0Io4, 0};
  BlockInputAssignment pmod_test_spi_off_inputs[] = {{SonataPinmux::BlockInput::SpiReceive3, 0}};

  // The pinmux testplan to execute. This testplan runs through testing GPIO, UART, I2C and SPI
  // all on the same PMOD pins, with users manually changing out the connected devices between
  // tests when necessary.
  Test pinmux_testplan[] = {
      {
          .type             = TestType::GpioWriteRead,
          .name             = "PMOD0_2 -> PMOD0_3 GPIO Muxed    ",
          .manual_required  = true,
          .instruction      = "Manually connect PMOD0 Pins 2 & 3 with a wire in a loop.",
          .output_pins      = pmod_test_gpio_on_pins,
          .num_output_pins  = ARRAYSIZE(pmod_test_gpio_on_pins),
          .block_inputs     = pmod_test_gpio_on_inputs,
          .num_block_inputs = ARRAYSIZE(pmod_test_gpio_on_inputs),
          .gpio_data =
              {
                  {GpioInstance::Pmod, 1},  // PMOD0_2
                  {GpioInstance::Pmod, 2},  // PMOD0_3
                  GpioWaitMsec,
                  GpioTestLength,
              },
          .expected_result = true,
      },
      {
          .type             = TestType::GpioWriteRead,
          .name             = "PMOD0_2 -> PMOD0_3 GPIO Not Muxed",
          .manual_required  = false,
          .output_pins      = pmod_test_gpio_off_pins,
          .num_output_pins  = ARRAYSIZE(pmod_test_gpio_off_pins),
          .block_inputs     = pmod_test_gpio_off_inputs,
          .num_block_inputs = ARRAYSIZE(pmod_test_gpio_off_inputs),
          .gpio_data =
              {
                  {GpioInstance::Pmod, 1},  // PMOD0_2
                  {GpioInstance::Pmod, 2},  // PMOD0_3
                  GpioWaitMsec,
                  GpioTestLength,
              },
          .expected_result = false,
      },
      {
          .type             = TestType::UartSendReceive,
          .name             = "PMOD0_2 -> PMOD0_3 UART Muxed    ",
          .manual_required  = false,
          .output_pins      = pmod_test_uart_on_pins,
          .num_output_pins  = ARRAYSIZE(pmod_test_uart_on_pins),
          .block_inputs     = pmod_test_uart_on_inputs,
          .num_block_inputs = ARRAYSIZE(pmod_test_uart_on_inputs),
          .uart_data =
              {
                  UartTest::UartNum::Uart2,
                  UartTimeoutMsec,
                  UartTestBytes,
              },
          .expected_result = true,
      },
      {
          .type             = TestType::UartSendReceive,
          .name             = "PMOD0_2 -> PMOD0_3 UART Not Muxed",
          .manual_required  = false,
          .output_pins      = pmod_test_uart_off_pins,
          .num_output_pins  = ARRAYSIZE(pmod_test_uart_off_pins),
          .block_inputs     = pmod_test_uart_off_inputs,
          .num_block_inputs = ARRAYSIZE(pmod_test_uart_off_inputs),
          .uart_data =
              {
                  UartTest::UartNum::Uart2,
                  UartTimeoutMsec,
                  UartTestBytes,
              },
          .expected_result = false,
      },
      {
          .type             = TestType::I2cBH1745ReadId,
          .name             = "PMOD0_1 & PMOD0_2 I2C Muxed      ",
          .manual_required  = true,
          .instruction      = "Remove the wire connecting PMOD0 Pins 2 & 3. Connect the BH17745 to PMOD0.",
          .output_pins      = pmod_test_i2c_on_pins,
          .num_output_pins  = ARRAYSIZE(pmod_test_i2c_on_pins),
          .block_inputs     = nullptr,
          .num_block_inputs = 0,
          .i2c_data         = {I2cTest::I2cNum::I2c0},
          .expected_result  = true,
      },
      {
          .type             = TestType::I2cBH1745ReadId,
          .name             = "PMOD0_1 & PMOD0_2 I2C Not Muxed  ",
          .manual_required  = false,
          .output_pins      = pmod_test_i2c_off_pins,
          .num_output_pins  = ARRAYSIZE(pmod_test_i2c_off_pins),
          .block_inputs     = nullptr,
          .num_block_inputs = 0,
          .i2c_data         = {I2cTest::I2cNum::I2c0},
          .expected_result  = false,
      },
      {
          .type             = TestType::SpiPmodSF3ReadId,
          .name             = "PMOD0_{1,2,3,4} SPI Muxed        ",
          .manual_required  = true,
          .instruction      = "Remove the BH17745 from PMOD0. Connect the Spi PmodSF3 Flash to PMOD0.",
          .output_pins      = pmod_test_spi_on_pins,
          .num_output_pins  = ARRAYSIZE(pmod_test_spi_on_pins),
          .block_inputs     = pmod_test_spi_on_inputs,
          .num_block_inputs = ARRAYSIZE(pmod_test_spi_on_inputs),
          .spi_data =
              {
                  SpiTest::SpiNum::Spi3, {GpioInstance::Pmod, 0},  // PMOD0_1 is used as the CS signal
              },
          .expected_result = true,
      },
      {
          .type             = TestType::SpiPmodSF3ReadId,
          .name             = "PMOD0_{1,2,3,4} SPI Not Muxed    ",
          .manual_required  = false,
          .output_pins      = pmod_test_spi_off_pins,
          .num_output_pins  = ARRAYSIZE(pmod_test_spi_off_pins),
          .block_inputs     = pmod_test_spi_off_inputs,
          .num_block_inputs = ARRAYSIZE(pmod_test_spi_off_inputs),
          .spi_data =
              {
                  SpiTest::SpiNum::Spi3, {GpioInstance::Pmod, 0},  // PMOD0_1 is used as the CS signal
              },
          .expected_result = false,
      },
  };

  // Execute the pinmux testplan
  const uint8_t NumTests = ARRAYSIZE(pinmux_testplan);
  execute_testplan(pinmux_testplan, NumTests, prng, gpio, uart0, uarts, spis, i2cs, &Pinmux);

  // Infinite loop to stop execution returning
  while (true) {
    asm volatile("");
  }
}
