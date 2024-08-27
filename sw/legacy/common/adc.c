// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "adc.h"

#include "dev_access.h"
#include "sonata_system.h"

/**
 * Sets the bits of a given ADC register. Takes an ADC register, a mask of 
 * bits to write, and the value to write using that mask.
 */
static void set_adc_bits(adc_reg_t reg, uint32_t bit_mask, uint32_t bit_val) {
  uint32_t adc_bits = DEV_READ(reg);
  adc_bits &= ~bit_mask;  // Clear bits in the mask
  adc_bits |= bit_mask & bit_val; // Set values of bits in the mask.
  DEV_WRITE(reg, adc_bits);
}

/**
 * Sets the bits of a given ADC register to either 1 or 0 according to a
 * given mask. Takes an ADC register, a mask of bits to write, and the bit
 * value to write using that mask.
 */
static void set_adc_bit(adc_reg_t reg, uint32_t bit_mask, bool bit_val) {
  set_adc_bits(reg, bit_mask, bit_val ? 0xFFFF : 0x0000);
}

/**
 * Sets the clock divider in the XADC config registers. This controls
 * the sample rate of the ADC.
 */
void set_clock_divider(adc_t *adc, AdcClockDivider divider) {
  adc->divider = divider;
  set_adc_bits(
    ADC_FROM_ADDR_AND_OFFSET(adc->base_reg, ADC_REG_CONFIG_0),
    ADC_CONFIG_REG2_CD,
    (divider << 8)
  );
}

/**
 * Sets the power down in the XADC config registers. This will permanently 
 * disable the XADC according to the supplied option.
 */
void set_power_down(adc_t *adc, AdcPowerDownMode pd) {
  adc->pd = pd;
  set_adc_bits(
    ADC_FROM_ADDR_AND_OFFSET(adc->base_reg, ADC_REG_CONFIG_0),
    ADC_CONFIG_REG2_PD,
    (pd << 4)
  );
}

/**
 * Initialises the ADC for use by software. Takes, a pointer to an ADC context
 * struct for configuration, and a pointer to the ADC base register.
 */
void adc_init(adc_t *adc, adc_reg_t base_reg, AdcClockDivider divider) {
  adc->base_reg = base_reg;
  set_clock_divider(adc, divider);
  adc->pd = ADC_POWER_DOWN_NONE;
  adc->bipolar = false;
}

/**
 * Reads from the given ADC status register to find the latest result of a
 * given XADC sample. To observe changes in this value, the XADC must be
 * correctly configured to sample this channel, which it is by default.
 */
int16_t read_adc(adc_t *adc, AdcSampleStatusRegister reg) {
  adc_reg_t sample_reg = ADC_FROM_ADDR_AND_OFFSET(adc->base_reg, reg);
  uint16_t analogue_value = DEV_READ(sample_reg);
  analogue_value &= ADC_MEASUREMENT_MASK;
  analogue_value >>= (ADC_DRP_REG_SIZE - ADC_BIT_WIDTH);
  if (!adc->bipolar) {
    return (int16_t) analogue_value;
  }

  // If using bipolar, sign extend negative 12-bit values to 16 bit integers.
  int16_t extended = 0;
  extended |= analogue_value;
  int16_t sign_bit_mask = 1U << (ADC_BIT_WIDTH - 1);
  return (extended ^ sign_bit_mask) - sign_bit_mask;
}
