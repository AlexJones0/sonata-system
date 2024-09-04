// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef ADC_H__
#define ADC_H__

#include <stdint.h>
#include <stdbool.h>

/** ADC Max samples, DRP register size and measurement bit
 * width are defined in the documentation:
 * https://docs.amd.com/r/en-US/ug480_7Series_XADC/Introduction-and-Quick-Start?section=XREF_92049_Introduction
 */
#define ADC_MAX_SAMPLES   (1 * 1000 * 1000)
#define ADC_MIN_CLCK_FREQ (1 * 1000 * 1000)
#define ADC_MAX_CLCK_FREQ (26 * 1000 * 1000)
#define ADC_DRP_REG_SIZE  16
#define ADC_BIT_WIDTH     12

/**
 * ADC DRP register offsets (each mapped sequentially to 4 bytes in memory).
 * Source / Documentation:
 * https://docs.amd.com/r/en-US/ug480_7Series_XADC/XADC-Register-Interface
 */
typedef enum AdcDrpRegisterOffset {
  ADC_REG_TEMPERATURE         = 0x00,
  ADC_REG_V_CCINT             = 0x01,
  ADC_REG_V_CCAUX             = 0x02,
  ADC_REG_V_P_N               = 0x03,
  ADC_REG_V_REF_P             = 0x04,
  ADC_REG_V_REF_N             = 0x05,
  ADC_REG_V_CCBRAM            = 0x06,
  ADC_REG_VAUX_P_N_0          = 0x10,
  ADC_REG_VAUX_P_N_1          = 0x11,
  ADC_REG_VAUX_P_N_2          = 0x12,
  ADC_REG_VAUX_P_N_3          = 0x13,
  ADC_REG_VAUX_P_N_4          = 0x14,
  ADC_REG_VAUX_P_N_5          = 0x15,
  ADC_REG_VAUX_P_N_6          = 0x16,
  ADC_REG_VAUX_P_N_7          = 0x17,
  ADC_REG_VAUX_P_N_8          = 0x18,
  ADC_REG_VAUX_P_N_9          = 0x19,
  ADC_REG_VAUX_P_N_10         = 0x1A,
  ADC_REG_VAUX_P_N_11         = 0x1B,
  ADC_REG_VAUX_P_N_12         = 0x1C,
  ADC_REG_VAUX_P_N_13         = 0x1D,
  ADC_REG_VAUX_P_N_14         = 0x1E,
  ADC_REG_VAUX_P_N_15         = 0x1F,
  ADC_REG_CONFIG_0            = 0x40,
  ADC_REG_CONFIG_1            = 0x41,
  ADC_REG_CONFIG_2            = 0x42,
} AdcDrpRegisterOffset;

/**
 * ADC Config Register 2 bit field definitions. Source / Documentation:
 * https://docs.amd.com/r/en-US/ug480_7Series_XADC/Control-Registers?section=XREF_53021_Configuration
 */
typedef enum AdcConfig2Field {
  /* Power-down bits for the XADC. Setting both to 1 powers down the entire
  XADC block, setting PD0=0 and PD1=1 powers down XADC block B permanently.*/
  ADC_CONFIG_REG2_PD = (0x3 << 4),

  /* Bits used to select the division ratio between the DRP clock (DCLK) and
  the lower frequency ADC clock (ADCCLK). Note that values of 0x0 and 0x1
  are mapped to a division of 2 by the DCLK division selection specification.
  All other 8-bit values are mapped identically. That is, the minimum division
  ratio is 2 such that ADCCLK = DCLK/2. */
  ADC_CONFIG_REG2_CD = (0xFF << 8),
} AdcConfig2Field;

/**
 * Power down settings that can be selected using XADC Config Register 2.
 * Source / Documentation:
 * https://docs.amd.com/r/en-US/ug480_7Series_XADC/Control-Registers?section=XREF_93518_Power_Down
 */
typedef enum AdcPowerDownMode {
  ADC_POWER_DOWN_NONE  = 0x0,  // Default
  ADC_POWER_DOWN_ADC_B = 0x2,
  ADC_POWER_DOWN_XADC  = 0x3,
} AdcPowerDownMode;

/**
 * An enum representing the offsets of the ADC DRP status registers that are
 * used to store values that are measured/sampled by the XADC.
 */
typedef enum AdcSampleStatusRegister {
  /**
   * The mapping of Sonata Arduino Analogue pins to the status registers that
   * store the results of sampling the auxiliary analogue channels that those
   * pins are connected to in the XADC. Source / Documentation: Sheet 15 of
   * https://github.com/newaetech/sonata-pcb/blob/649b11c2fb758f798966605a07a8b6b68dd434e9/sonata-schematics-r09.pdf
   */
  ADC_STATUS_ARDUINO_A0  = ADC_REG_VAUX_P_N_4,
  ADC_STATUS_ARDUINO_A1  = ADC_REG_VAUX_P_N_12,
  ADC_STATUS_ARDUINO_A2  = ADC_REG_VAUX_P_N_5,
  ADC_STATUS_ARDUINO_A3  = ADC_REG_VAUX_P_N_13,
  ADC_STATUS_ARDUINO_A4  = ADC_REG_VAUX_P_N_6,
  ADC_STATUS_ARDUINO_A5  = ADC_REG_VAUX_P_N_14,

  // Status registers for internal XADC measurements.
  ADC_STATUS_TEMPERATURE = ADC_REG_TEMPERATURE,
  ADC_STATUS_V_CCINT     = ADC_REG_V_CCINT,
  ADC_STATUS_V_CCAUX     = ADC_REG_V_CCAUX,
  ADC_STATUS_V_REF_P     = ADC_REG_V_REF_P,
  ADC_STATUS_V_REF_N     = ADC_REG_V_REF_N,
  ADC_STATUS_V_CCBRAM    = ADC_REG_V_CCBRAM,
} AdcSampleStatusRegister;

typedef uint8_t adc_clock_divider;

/**
 * The result of measurements / analog conversions stored in DRP status
 * registers are 12 bit values that are stored MSB justified.
 */
#define ADC_MEASUREMENT_MASK 0xFFF0

/**
 * ADC DRP registers are 16 bits, but each 16 bit register is mapped to
 * the lower bits of 1 word (4 bytes) in memory.
 */
typedef uint32_t *adc_reg_t;

#define ADC_FROM_BASE_ADDR(addr) ((adc_reg_t)addr)
#define ADC_FROM_ADDR_AND_OFFSET(addr, offset) ((adc_reg_t)(addr + offset))

typedef struct adc {
  adc_reg_t base_reg;
  adc_clock_divider divider;
  AdcPowerDownMode pd;
} adc_t;

void adc_init(adc_t *adc, adc_reg_t base_reg, adc_clock_divider divider);
void adc_set_clock_divider(adc_t *adc, adc_clock_divider divider);
void adc_set_power_down(adc_t *adc, AdcPowerDownMode pd);
int16_t read_adc(adc_t *adc, AdcSampleStatusRegister reg);

#endif //ADC_H__
