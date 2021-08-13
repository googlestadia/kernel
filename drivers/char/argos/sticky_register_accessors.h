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

static inline uint8
sticky_registers_firmware_control_bc_cores(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 0) & 0xffULL) << 0));
}

static inline int
set_sticky_registers_firmware_control_bc_cores(uint64 *reg_value, uint8 value)
{
 if (value & ~(0xffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffULL) << 0)) |
         (((value >> 0) & (0xffULL)) << 0);

 return 0;
}

static inline uint8
sticky_registers_firmware_control_gc_cores(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 8) & 0xfULL) << 0));
}

static inline int
set_sticky_registers_firmware_control_gc_cores(uint64 *reg_value, uint8 value)
{
 if (value & ~(0xfULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xfULL) << 8)) |
         (((value >> 0) & (0xfULL)) << 8);

 return 0;
}

static inline uint8
sticky_registers_firmware_control_lpddr4_controllers(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 12) & 0xfULL) << 0));
}

static inline int
set_sticky_registers_firmware_control_lpddr4_controllers(uint64 *reg_value,
        uint8 value)
{
 if (value & ~(0xfULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xfULL) << 12)) |
         (((value >> 0) & (0xfULL)) << 12);

 return 0;
}

static inline uint16
sticky_registers_firmware_control_speed(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 16) & 0xffffULL) << 0));
}

static inline int set_sticky_registers_firmware_control_speed(uint64 *reg_value,
             uint16 value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 16)) |
         (((value >> 0) & (0xffffULL)) << 16);

 return 0;
}

static inline uint8
sticky_registers_firmware_control_phy_init_enable(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 32) & 0x1ULL) << 0));
}

static inline int
set_sticky_registers_firmware_control_phy_init_enable(uint64 *reg_value,
            uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 32)) |
         (((value >> 0) & (0x1ULL)) << 32);

 return 0;
}

static inline uint8
sticky_registers_firmware_control_phy_training_enable(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 33) & 0x1ULL) << 0));
}

static inline int
set_sticky_registers_firmware_control_phy_training_enable(uint64 *reg_value,
         uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 33)) |
         (((value >> 0) & (0x1ULL)) << 33);

 return 0;
}

static inline uint8
sticky_registers_firmware_control_dfi_align_disable_fifo(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 34) & 0x1ULL) << 0));
}

static inline int
set_sticky_registers_firmware_control_dfi_align_disable_fifo(uint64 *reg_value,
            uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 34)) |
         (((value >> 0) & (0x1ULL)) << 34);

 return 0;
}

static inline uint8
sticky_registers_firmware_control_speedup_spi(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 35) & 0x1ULL) << 0));
}

static inline int
set_sticky_registers_firmware_control_speedup_spi(uint64 *reg_value,
        uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 35)) |
         (((value >> 0) & (0x1ULL)) << 35);

 return 0;
}

static inline uint8
sticky_registers_firmware_control_bypass_zero_ddr(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 36) & 0x1ULL) << 0));
}

static inline int
set_sticky_registers_firmware_control_bypass_zero_ddr(uint64 *reg_value,
            uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 36)) |
         (((value >> 0) & (0x1ULL)) << 36);

 return 0;
}

#endif
