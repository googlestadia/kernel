/*
 * Copyright (C) 2021 Google LLC.
 */
# 3 "./drivers/char/argos/sticky_register_accessors.h"
#ifndef _DRIVERS_CHAR_ARGOS_STICKY_REGISTER_ACCESSORS_H_
#define _DRIVERS_CHAR_ARGOS_STICKY_REGISTER_ACCESSORS_H_ 

#ifdef __KERNEL__
#include "drivers/gasket/gasket_types.h"
#else
#include "platforms/asic_sw/kernel/common/gasket/gasket_types.h"
#endif

static inline uint64
sticky_registers_firmware_control_enable_cl(const uint64 reg_value)
{
 return (uint64)((((reg_value >> 0) & 0xfffffffffULL) << 0));
}

static inline int
set_sticky_registers_firmware_control_enable_cl(uint64 *reg_value, uint64 value)
{
 if (value & ~(0xfffffffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xfffffffffULL) << 0)) |
         (((value >> 0) & (0xfffffffffULL)) << 0);

 return 0;
}

#endif
