/*
 * Copyright (C) 2021 Google LLC.
 */
# 3 "./drivers/char/argos/queue_control_accessors.h"
#ifndef _DRIVERS_CHAR_ARGOS_QUEUE_CONTROL_ACCESSORS_H_
#define _DRIVERS_CHAR_ARGOS_QUEUE_CONTROL_ACCESSORS_H_ 

#ifdef __KERNEL__
#include "drivers/gasket/gasket_types.h"
#else
#include "platforms/asic_sw/kernel/common/gasket/gasket_types.h"
#endif

static inline uint8 queue_control_control_enable(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 0) & 0x1ULL) << 0));
}

static inline int set_queue_control_control_enable(uint64 *reg_value,
         uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 0)) |
         (((value >> 0) & (0x1ULL)) << 0);

 return 0;
}

static inline uint8
queue_control_control_interrupt_enable(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 1) & 0x1ULL) << 0));
}

static inline int set_queue_control_control_interrupt_enable(uint64 *reg_value,
            uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 1)) |
         (((value >> 0) & (0x1ULL)) << 1);

 return 0;
}

static inline uint8 queue_control_status_enabled(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 0) & 0x1ULL) << 0));
}

static inline int set_queue_control_status_enabled(uint64 *reg_value,
         uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 0)) |
         (((value >> 0) & (0x1ULL)) << 0);

 return 0;
}

static inline uint8 queue_control_status_failed(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 1) & 0x1ULL) << 0));
}

static inline int set_queue_control_status_failed(uint64 *reg_value,
        uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 1)) |
         (((value >> 0) & (0x1ULL)) << 1);

 return 0;
}

static inline uint8 queue_control_status_enable_pending(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 2) & 0x1ULL) << 0));
}

static inline int set_queue_control_status_enable_pending(uint64 *reg_value,
         uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 2)) |
         (((value >> 0) & (0x1ULL)) << 2);

 return 0;
}

static inline uint8
queue_control_error_status_dma_failed(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 0) & 0x1ULL) << 0));
}

static inline int set_queue_control_error_status_dma_failed(uint64 *reg_value,
           uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 0)) |
         (((value >> 0) & (0x1ULL)) << 0);

 return 0;
}

static inline uint8
queue_control_error_status_page_fault(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 1) & 0x1ULL) << 0));
}

static inline int set_queue_control_error_status_page_fault(uint64 *reg_value,
           uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 1)) |
         (((value >> 0) & (0x1ULL)) << 1);

 return 0;
}

static inline uint8
queue_control_quiesce_ack_quiesce_ack(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 0) & 0x1ULL) << 0));
}

static inline int set_queue_control_quiesce_ack_quiesce_ack(uint64 *reg_value,
           uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 0)) |
         (((value >> 0) & (0x1ULL)) << 0);

 return 0;
}

#endif
