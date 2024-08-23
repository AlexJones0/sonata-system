// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "adc.h"

#include "sonata_system.h"
#include "dev_access.h"

static void set_adc_bits(adc_reg_t reg, uint32_t bit_mask, uint32_t bit_val) {
  uint32_t adc_bits = DEV_READ(reg);
  adc_bits &= ~bit_mask;  // Clear bits in the mask
  adc_bits |= bit_mask & bit_val; // Set values of bits in the mask.
  DEV_WRITE(reg, adc_bits);
}

static void set_adc_bit(adc_reg_t reg, uint32_t bit_mask, bool bit_val) {
  set_adc_bits(reg, bit_mask, bit_val ? 0xFFFF : 0x0000);
}

/* For now, this is just running default mode ADC, with continuous
sampling, with no alarms. You must supply the correct `output_reg` register for
the channel that you have selected. `reg` just refers to the ADC base address.*/
void adc_init(
  adc_t *adc, 
  adc_reg_t reg, 
  adc_reg_t output_reg,
  ADCChannelSelection channel, 
  ADCSampleAveragingMode averaging_mode, 
  ADCClockDivider divider, 
  bool enable_acq,
  bool enable_bipolar
) {
  adc_reg_t reg_config0 = ADC_FROM_ADDR_AND_OFFSET(ADC_BASE, ADC_REG_CONFIG_0);

  adc->reg = reg;
  adc->output_reg = output_reg;

  adc->channel = channel;
  //set_adc_bits(reg_config0, ADC_CONFIG_REG0_CHANNEL_MASK, channel);
  
  adc->averaging_mode = averaging_mode;
  //set_adc_bits(reg_config0, ADC_CONFIG_REG0_AVG_MASK, (averaging_mode << 12));
  
  adc->divider = divider;
  /*set_adc_bits(
    ADC_FROM_ADDR_AND_OFFSET(ADC_BASE, ADC_REG_CONFIG_2),
    ADC_CONFIG_REG2_DIVIDER_MASK,
    (divider << 8)
  );*/

  adc->enable_acq = enable_acq;
  //set_adc_bit(reg_config0, ADC_CONFIG_REG0_ACQ, enable_acq);
  
  adc->enable_bipolar = enable_bipolar;
  //set_adc_bit(reg_config0, ADC_CONFIG_REG0_BU, enable_bipolar);

  /*set_adc_bits(
    ADC_FROM_ADDR_AND_OFFSET(ADC_BASE, ADC_REG_CONFIG_1),
    ADC_CONFIG_REG1_SEQUENCER_MASK,
    (ADC_SEQUENCER_DEFAULT_MODE << 12)
  );*/
}

int16_t read_adc(adc_t *adc) {
  /*putstr("ADC_REG_V_P_N: ");
  puthexn(DEV_READ(ADC_FROM_ADDR_AND_OFFSET(ADC_BASE, ADC_REG_V_P_N)), 4);
  putstr("\nADC_REG_VAUX_P_N_0: ");
  puthexn(DEV_READ(ADC_FROM_ADDR_AND_OFFSET(ADC_BASE, ADC_REG_VAUX_P_N_0)), 4);
  putstr("\nADC_REG_VAUX_P_N_1: ");
  puthexn(DEV_READ(ADC_FROM_ADDR_AND_OFFSET(ADC_BASE, ADC_REG_VAUX_P_N_1)), 4);
  putstr("\nADC_REG_VAUX_P_N_2: ");
  puthexn(DEV_READ(ADC_FROM_ADDR_AND_OFFSET(ADC_BASE, ADC_REG_VAUX_P_N_2)), 4);
  putstr("\nADC_REG_VAUX_P_N_3: ");
  puthexn(DEV_READ(ADC_FROM_ADDR_AND_OFFSET(ADC_BASE, ADC_REG_VAUX_P_N_3)), 4);
  putstr("\nADC_REG_VAUX_P_N_4: ");
  puthexn(DEV_READ(ADC_FROM_ADDR_AND_OFFSET(ADC_BASE, ADC_REG_VAUX_P_N_4)), 4);
  putstr("\nADC_REG_VAUX_P_N_5: ");
  puthexn(DEV_READ(ADC_FROM_ADDR_AND_OFFSET(ADC_BASE, ADC_REG_VAUX_P_N_5)), 4);
  putstr("\nADC_REG_VAUX_P_N_6: ");
  puthexn(DEV_READ(ADC_FROM_ADDR_AND_OFFSET(ADC_BASE, ADC_REG_VAUX_P_N_6)), 4);
  putstr("\nADC_REG_VAUX_P_N_7: ");
  puthexn(DEV_READ(ADC_FROM_ADDR_AND_OFFSET(ADC_BASE, ADC_REG_VAUX_P_N_7)), 4);
  putstr("\nADC_REG_VAUX_P_N_8: ");
  puthexn(DEV_READ(ADC_FROM_ADDR_AND_OFFSET(ADC_BASE, ADC_REG_VAUX_P_N_8)), 4);
  putstr("\nADC_REG_VAUX_P_N_9: ");
  puthexn(DEV_READ(ADC_FROM_ADDR_AND_OFFSET(ADC_BASE, ADC_REG_VAUX_P_N_9)), 4);
  putstr("\nADC_REG_VAUX_P_N_10: ");
  puthexn(DEV_READ(ADC_FROM_ADDR_AND_OFFSET(ADC_BASE, ADC_REG_VAUX_P_N_10)), 4);
  putstr("\nADC_REG_VAUX_P_N_11: ");
  puthexn(DEV_READ(ADC_FROM_ADDR_AND_OFFSET(ADC_BASE, ADC_REG_VAUX_P_N_11)), 4);
  putstr("\nADC_REG_VAUX_P_N_12: ");
  puthexn(DEV_READ(ADC_FROM_ADDR_AND_OFFSET(ADC_BASE, ADC_REG_VAUX_P_N_12)), 4);
  putstr("\nADC_REG_VAUX_P_N_13: ");
  puthexn(DEV_READ(ADC_FROM_ADDR_AND_OFFSET(ADC_BASE, ADC_REG_VAUX_P_N_13)), 4);
  putstr("\nADC_REG_VAUX_P_N_14: ");
  puthexn(DEV_READ(ADC_FROM_ADDR_AND_OFFSET(ADC_BASE, ADC_REG_VAUX_P_N_14)), 4);
  putstr("\nADC_REG_VAUX_P_N_15: ");
  puthexn(DEV_READ(ADC_FROM_ADDR_AND_OFFSET(ADC_BASE, ADC_REG_VAUX_P_N_15)), 4);
  putstr("\n");*/
  uint16_t analogue_value = DEV_READ(adc->output_reg);
  analogue_value &= ADC_MEASUREMENT_MASK;
  analogue_value >>= (ADC_DRP_REG_SIZE - ADC_BIT_WIDTH);
  if (!adc->enable_bipolar) {
    return (int16_t) analogue_value;
  }

  // If using bipolar, sign extend negative 12-bit values to 16 bit integers.
  int16_t extended = 0;
  extended |= analogue_value;
  int16_t sign_bit_mask = 1U << (ADC_BIT_WIDTH - 1);
  return (extended ^ sign_bit_mask) - sign_bit_mask;
}
