// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "adc.h"

#include "dev_access.h"
#include "sonata_system.h"
#include "assert.h"

/**
 * Sets the bits of a given ADC register. Takes an ADC register, a mask of 
 * bits to write, and the value to write using that mask.
 */
static void set_adc_bits(adc_reg_t reg, uint32_t bit_mask, uint32_t bit_val) {
  uint32_t adc_bits = DEV_READ(reg);
  adc_bits &= ~bit_mask;          // Clear bits in the mask
  adc_bits |= bit_mask & bit_val; // Set values of bits in the mask.
  DEV_WRITE(reg, adc_bits);
}

/**
 * Sets the clock divider in the XADC config registers. This controls
 * the sample rate of the ADC. The clock divider should not be set such
 * that the ADC Clock (equal to SYSCLCK / MAX(divider, 2)) falls outside
 * the range of 1-26 MHz.
 */
void adc_set_clock_divider(adc_t *adc, adc_clock_divider divider) {
  assert(divider >= 2 &&
    "A divider of < 2 is equivalent to a divider of 2");
  assert(SYSCLK_FREQ / divider >= ADC_MIN_CLCK_FREQ &&
    "The divider cannot be set such that the ADCCLCK < 1 MHz");
  assert(SYSCLK_FREQ / divider <= ADC_MAX_CLCK_FREQ &&
    "The divider cannot be set such that ADCCLK > 26 MHz");
  adc->divider = divider;
  set_adc_bits(
    ADC_FROM_ADDR_AND_OFFSET(adc->base_reg, ADC_REG_CONFIG_2),
    ADC_CONFIG_REG2_CD,
    (divider << 8)
  );
}

/**
 * Sets the power down in the XADC config registers. This will permanently 
 * disable the XADC according to the supplied option.
 */
void adc_set_power_down(adc_t *adc, AdcPowerDownMode pd) {
  adc->pd = pd;
  set_adc_bits(
    ADC_FROM_ADDR_AND_OFFSET(adc->base_reg, ADC_REG_CONFIG_2),
    ADC_CONFIG_REG2_PD,
    (pd << 4)
  );
}

/**
 * Initialises the ADC for use by software. Takes, a pointer to an ADC context
 * struct for configuration, a pointer to the ADC base register, and a clock 
 * divider. The divider should not be set such that the ADC Clock (equal
 * to SYSCLCK / MAX(divider, 2)) falls outside the range of 1-26 MHz.
 */
void adc_init(adc_t *adc, adc_reg_t base_reg, adc_clock_divider divider) {
  adc->base_reg = base_reg;
  adc_set_clock_divider(adc, divider);
  adc_set_power_down(adc, ADC_POWER_DOWN_NONE);
  // The XADC starts in independent ADC mode by default, monitoring all channels.
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
  
  // Return the ADC value. This simple logic currently just assumes a 
  // unipolar analogue measurement - future support for bipolar
  // measurement would need to check whether it is enabled, and if so
  // correspondingly sign extend negative 12-bit two's complement values.
  return (int16_t)analogue_value;
}
