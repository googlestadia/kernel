/*
 * Copyright (C) 2021 Google LLC.
 */
# 3 "./drivers/char/argos/interrupt_control_accessors.h"
#ifndef _DRIVERS_CHAR_ARGOS_INTERRUPT_CONTROL_ACCESSORS_H_
#define _DRIVERS_CHAR_ARGOS_INTERRUPT_CONTROL_ACCESSORS_H_ 

#ifdef __KERNEL__
#include "drivers/gasket/gasket_types.h"
#else
#include "platforms/asic_sw/kernel/common/gasket/gasket_types.h"
#endif

static inline uint8 interrupt_control_control_enabled(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 0) & 0x1ULL) << 0));
}

static inline int set_interrupt_control_control_enabled(uint64 *reg_value,
       uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 0)) |
         (((value >> 0) & (0x1ULL)) << 0);

 return 0;
}

static inline uint8 interrupt_control_status_hot(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 0) & 0x1ULL) << 0));
}

static inline int set_interrupt_control_status_hot(uint64 *reg_value,
         uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 0)) |
         (((value >> 0) & (0x1ULL)) << 0);

 return 0;
}

#endif
