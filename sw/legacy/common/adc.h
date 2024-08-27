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
#define ADC_MAX_SAMPLES  (1 * 1000 * 1000)
#define ADC_DRP_REG_SIZE 16
#define ADC_BIT_WIDTH    12

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
  ADC_REG_SUPPLY_A_OFFSET     = 0x08,
  ADC_REG_ADC_A_OFFSET        = 0x09,
  ADC_REG_ADC_A_GAIN          = 0x0A,
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
  ADC_REG_MAX_TEMP            = 0x20,
  ADC_REG_MAX_V_CCINT         = 0x21,
  ADC_REG_MAX_V_CCAUX         = 0x22,
  ADC_REG_MAX_V_CCBRAM        = 0x23,
  ADC_REG_MIN_TEMP            = 0x24,
  ADC_REG_MIN_V_CCINT         = 0x25,
  ADC_REG_MIN_V_CCAUX         = 0x26,
  ADC_REG_MIN_V_CCBRAM        = 0x27,
  ADC_REG_SUPPLY_B_OFFSET     = 0x30,
  ADC_REG_ADC_B_OFFSET        = 0x31,
  ADC_REG_ADC_B_GAIN          = 0x32,
  ADC_REG_FLAG                = 0x3F,
  ADC_REG_CONFIG_0            = 0x40,
  ADC_REG_CONFIG_1            = 0x41,
  ADC_REG_CONFIG_2            = 0x42,
  ADC_REG_SEQ_CHANNEL_0       = 0x48,
  ADC_REG_SEQ_CHANNEL_1       = 0x49,
  ADC_REG_SEQ_AVERAGE_0       = 0x4A,
  ADC_REG_SEQ_AVERAGE_1       = 0x4B,
  ADC_REG_SEQ_INPUT_MODE_0    = 0x4C,
  ADC_REG_SEQ_INPUT_MODE_1    = 0x4D,
  ADC_REG_SEQ_SETTLING_TIME_0 = 0x4E,
  ADC_REG_SEQ_SETTLING_TIME_1 = 0x4F,
  ADC_REG_ALARM_0             = 0x50,
  ADC_REG_ALARM_1             = 0x51,
  ADC_REG_ALARM_2             = 0x52,
  ADC_REG_ALARM_3             = 0x53,
  ADC_REG_ALARM_4             = 0x54,
  ADC_REG_ALARM_5             = 0x55,
  ADC_REG_ALARM_6             = 0x56,
  ADC_REG_ALARM_7             = 0x57,
  ADC_REG_ALARM_8             = 0x58,
  ADC_REG_ALARM_9             = 0x59,
  ADC_REG_ALARM_10            = 0x5A,
  ADC_REG_ALARM_11            = 0x5B,
  ADC_REG_ALARM_12            = 0x5C,
  ADC_REG_ALARM_13            = 0x5D,
  ADC_REG_ALARM_14            = 0x5E,
  ADC_REG_ALARM_15            = 0x5F,
} AdcDrpRegisterOffset;

/** 
 * ADC Config Register 0 bit field definitions. Source / Documentation: 
 * https://docs.amd.com/r/en-US/ug480_7Series_XADC/Control-Registers?section=XREF_53021_Configuration
 */
typedef enum AdcConfig0Field {
  /* 5 bits for channel selection when operating in single channel mode (or external
  multiplexer mode). */
  ADC_CONFIG_REG0_CH   = (0x1F << 0),

  /* In single channel mode, ACQ is used to increase the acquisition time of 
  external analogue inputs in continuous sampling by 6 ADCCLK cycles. */
  ADC_CONFIG_REG0_ACQ  = (0x1 << 8),

  /* Select either continuous or event-driven sampling mode for the ADC.
  Logical 1 is event-driven, logical 0 is continuous sampling. */
  ADC_CONFIG_REG0_EC   = (0x1 << 9),

  /* Used when in single channel mode to select either the unipolar or bipolar
  operating mode for the ADC analogue inputs. Logic 1 is bipolar (both
  positive & negative voltage, both directions), and logic 0 is unipolar. */
  ADC_CONFIG_REG0_BU   = (0x1 << 10),

  /* Enables external multiplexer mode if set to 1. */
  ADC_CONFIG_REG0_MUX  = (0x1 << 11),

  /* Used to set the amount of sampling averaging on selected channels, in both
  single channel and sequence modes.*/
  ADC_CONFIG_REG0_AVG  = (0x3 << 12),

  /*Used to disable averaging for the calculation of calibration coefficients. 
  Enabled by default (logic 0), set to 1 to disable averaging. Averaging is
  fixed at 16 samples. */
  ADC_CONFIG_REG0_CAVG = (0x1 << 15),
} AdcConfig0Field;

/** 
 * ADC Config Register 1 bit field definitions. Source / Documentation: 
 * https://docs.amd.com/r/en-US/ug480_7Series_XADC/Control-Registers?section=XREF_53021_Configuration
 */
typedef enum AdcConfig1Field {
  /* Set to logic 1 to disable the over-temperature signal. */
  ADC_CONFIG_REG1_OT  = (0x1 << 0),

  /* Register bits to disable individual alarm outputs.*/
  ADC_CONFIG_REG1_ALM = (0x787 << 1),
  
  /* Register bits to enable calibration coefficients.*/
  ADC_CONFIG_REG1_CAL = (0xF << 4),

  /* Register bits to enable the channel-sequencer function*/
  ADC_CONFIG_REG1_SEQ = (0xF << 12),
} AdcConfig1Field;

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
 * Channels that can be selected using XADC Config Register 0. 
 * Source / Documentation: 
 * https://docs.amd.com/r/en-US/ug480_7Series_XADC/Control-Registers?section=XREF_59086_ADC_Channel_Select
 */
typedef enum AdcChannel {
  ADC_CHANNEL_ON_CHIP_TEMPERATURE = 0x00,
  ADC_CHANNEL_V_CCINT             = 0x01,
  ADC_CHANNEL_V_CCAUX             = 0x02,
  ADC_CHANNEL_V_P_N               = 0x03, // Dedicated Analog inputs
  ADC_CHANNEL_V_REFP              = 0x04, // 1.25V
  ADC_CHANNEL_V_REFN              = 0x05, // 0V
  ADC_CHANNEL_V_CCBRAM            = 0x06,
  ADC_CHANNEL_XADC_CALIBRATION    = 0x08,
  ADC_CHANNEL_V_CCPINT            = 0x0D,
  ADC_CHANNEL_V_CCPAUX            = 0x0E,
  ADC_CHANNEL_V_CCO_DDR           = 0x0F,
  ADC_CHANNEL_VAUX_P_N_0          = 0x10,
  ADC_CHANNEL_VAUX_P_N_1          = 0x11,
  ADC_CHANNEL_VAUX_P_N_2          = 0x12,
  ADC_CHANNEL_VAUX_P_N_3          = 0x13,
  ADC_CHANNEL_VAUX_P_N_4          = 0x14,
  ADC_CHANNEL_VAUX_P_N_5          = 0x15,
  ADC_CHANNEL_VAUX_P_N_6          = 0x16,
  ADC_CHANNEL_VAUX_P_N_7          = 0x17,
  ADC_CHANNEL_VAUX_P_N_8          = 0x18,
  ADC_CHANNEL_VAUX_P_N_9          = 0x19,
  ADC_CHANNEL_VAUX_P_N_10         = 0x1A,
  ADC_CHANNEL_VAUX_P_N_11         = 0x1B,
  ADC_CHANNEL_VAUX_P_N_12         = 0x1C,
  ADC_CHANNEL_VAUX_P_N_13         = 0x1D,
  ADC_CHANNEL_VAUX_P_N_14         = 0x1E,
  ADC_CHANNEL_VAUX_P_N_15         = 0x1F,
} AdcChannel;

/** 
 * Sampling filters that can be selected using XADC Config Register 0. 
 * Source / Documentation: 
 * https://docs.amd.com/r/en-US/ug480_7Series_XADC/Control-Registers?section=XREF_21317_Averaging_Filter
 */
typedef enum AdcAveragingFilter {
  ADC_AVERAGING_NONE        = 0x0,
  ADC_AVERAGING_16_SAMPLES  = 0x1,
  ADC_AVERAGING_64_SAMPLES  = 0x2,
  ADC_AVERAGING_256_SAMPLES = 0x3,
} AdcAveragingFilter;

/**
 * Sequencer operation modes that can be selected using XADC Config Register 1.
 * Source / Documentation:
 * https://docs.amd.com/r/en-US/ug480_7Series_XADC/Control-Registers?section=XREF_70073_Sequencer
 */
typedef enum AdcSequencerMode {
  ADC_SEQUENCER_DEFAULT_MODE               = 0x0,
  ADC_SEQUENCER_SINGLE_PASS_SEQUENCE       = 0x1,
  ADC_SEQUENCER_CONTINUOUS_SEQUENCE_MODE   = 0x2,
  ADC_SEQUENCER_SINGLE_CHANNEL_MODE        = 0x3,
  ADC_SEQUENCER_SIMULTANEOUS_SAMPLING_MODE = 0x4,
  ADC_SEQUENCER_INDEPENDENT_ADC_MODE       = 0x8,
} AdcSequencerMode;

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
 * Bits used in the channel sequencer control registers. Each of the four
 * channel sequencer properties are configured by two sequential DRP control
 * registers (0 and 1). Setting the reelvant bit (described here) to 1 in the
 * correct register will enable that property for that channel in the XADC's
 * automatic sequencer. This enum covers bits for register 0 (the lower offset
 * of the pair). Source / Documentation:
 * https://docs.amd.com/r/en-US/ug480_7Series_XADC/Automatic-Channel-Sequencer?section=XREF_99700_Sequencer_On_Chip
 */
typedef enum AdcSequencerChannelBit0 {
  ADC_SEQUENCER_XADC_CALIBRATION_BIT    = (1 << 0),
  ADC_SEQUENCER_V_CCPINT_BIT            = (1 << 5),
  ADC_SEQUENCER_V_CCPAUX_BIT            = (1 << 6),
  ADC_SEQUENCER_V_CCO_DDR_BIT           = (1 << 7),
  ADC_SEQUENCER_ON_CHIP_TEMPERATURE_BIT = (1 << 8),
  ADC_SEQUENCER_V_CCINT_BIT             = (1 << 9),
  ADC_SEQUENCER_V_CCAUX_BIT             = (1 << 10),
  ADC_SEQUENCER_V_P_N_BIT               = (1 << 11), 
  ADC_SEQUENCER_V_REFP_BIT              = (1 << 12),
  ADC_SEQUENCER_V_REFN_BIT              = (1 << 13),
  ADC_SEQUENCER_V_CCBRAM_BIT            = (1 << 14),
} AdcSequencerChannelBit0;

/**
 * Bits used in the channel sequencer control registers. Each of the four
 * channel sequencer properties are configured by two sequential DRP control
 * registers (0 and 1). Setting the reelvant bit (described here) to 1 in the
 * correct register will enable that property for that channel in the XADC's
 * automatic sequencer. This enum covers bits for register 1 (the higher offset
 * of the pair). Source / Documentation:
 * https://docs.amd.com/r/en-US/ug480_7Series_XADC/Automatic-Channel-Sequencer?section=XREF_54328_Sequencer
 */
typedef enum AdcSequencerChannelBit1 {
  ADC_SEQUENCER_VAUX_P_N_0_BIT  = (1 << 0),
  ADC_SEQUENCER_VAUX_P_N_1_BIT  = (1 << 1),
  ADC_SEQUENCER_VAUX_P_N_2_BIT  = (1 << 2),
  ADC_SEQUENCER_VAUX_P_N_3_BIT  = (1 << 3),
  ADC_SEQUENCER_VAUX_P_N_4_BIT  = (1 << 4),
  ADC_SEQUENCER_VAUX_P_N_5_BIT  = (1 << 5),
  ADC_SEQUENCER_VAUX_P_N_6_BIT  = (1 << 6),
  ADC_SEQUENCER_VAUX_P_N_7_BIT  = (1 << 7),
  ADC_SEQUENCER_VAUX_P_N_8_BIT  = (1 << 8),
  ADC_SEQUENCER_VAUX_P_N_9_BIT  = (1 << 9),
  ADC_SEQUENCER_VAUX_P_N_10_BIT = (1 << 10),
  ADC_SEQUENCER_VAUX_P_N_11_BIT = (1 << 11),
  ADC_SEQUENCER_VAUX_P_N_12_BIT = (1 << 12),
  ADC_SEQUENCER_VAUX_P_N_13_BIT = (1 << 13),
  ADC_SEQUENCER_VAUX_P_N_14_BIT = (1 << 14),
  ADC_SEQUENCER_VAUX_P_N_15_BIT = (1 << 15),
} AdcSequencerChannelBit1;

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

typedef uint8_t AdcClockDivider;

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

/**
 * ADC DRP Registers are mapped to memory at 4-byte intervals; all accesses
 * must be 4-byte aligned. 
 */
#define ADC_FROM_ADDR_AND_OFFSET(addr, offset) ((adc_reg_t)(addr + offset))

typedef struct adc {
  adc_reg_t base_reg;
  AdcClockDivider divider;
  AdcPowerDownMode pd;
  bool bipolar;
} adc_t;

void adc_init(adc_t *adc, adc_reg_t base_reg, AdcClockDivider divider);
void set_clock_divider(adc_t *adc, AdcClockDivider divider);
void power_down(adc_t *adc, AdcPowerDownMode pd);
int16_t read_adc(adc_t *adc, AdcSampleStatusRegister reg);

#endif //ADC_H__
