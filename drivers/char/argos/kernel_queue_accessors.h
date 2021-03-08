/*
 * Copyright (C) 2021 Google LLC.
 */
# 3 "./drivers/char/argos/kernel_queue_accessors.h"
#ifndef _DRIVERS_CHAR_ARGOS_KERNEL_QUEUE_ACCESSORS_H_
#define _DRIVERS_CHAR_ARGOS_KERNEL_QUEUE_ACCESSORS_H_ 

#ifdef __KERNEL__
#include "drivers/gasket/gasket_types.h"
#else
#include "platforms/asic_sw/kernel/common/gasket/gasket_types.h"
#endif

enum kernel_queue_ddr_control_change_requested_value {
 kKernelQueueDdrControlChangeRequestedValueRequestAccepted = 0,
 kKernelQueueDdrControlChangeRequestedValueCountBasedRequest = 1,
 kKernelQueueDdrControlChangeRequestedValueBitmapBasedRequest = 2
};
typedef enum kernel_queue_ddr_control_change_requested_value
 kernel_queue_ddr_control_change_requested_value;

enum kernel_queue_ddr_status_value_value {
 kKernelQueueDdrStatusValueValueSuccess = 0,
 kKernelQueueDdrStatusValueValueNotEnoughAvailable = 1,
 kKernelQueueDdrStatusValueValueTooLarge = 2,
 kKernelQueueDdrStatusValueValuePendingConfigInProgress = 3,
 kKernelQueueDdrStatusValueValueQueueNotDisabled = 4,
 kKernelQueueDdrStatusValueValueInvalidRequestType = 5,
 kKernelQueueDdrStatusValueValueChunkAlreadyReserved = 6,
 kKernelQueueDdrStatusValueValueNoChunkBitmap = 7,
 kKernelQueueDdrStatusValueValueUnknownError = 8,
 kKernelQueueDdrStatusValueValueBitmapNotSupported = 9
};
typedef enum kernel_queue_ddr_status_value_value
 kernel_queue_ddr_status_value_value;

static inline uint16
kernel_queue_control_interrupt_number(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 0) & 0xffffULL) << 0));
}

static inline int set_kernel_queue_control_interrupt_number(uint64 *reg_value,
           uint16 value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 0)) |
         (((value >> 0) & (0xffffULL)) << 0);

 return 0;
}

static inline uint8
kernel_queue_control_page_table_index(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 16) & 0xffULL) << 0));
}

static inline int set_kernel_queue_control_page_table_index(uint64 *reg_value,
           uint8 value)
{
 if (value & ~(0xffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffULL) << 16)) |
         (((value >> 0) & (0xffULL)) << 16);

 return 0;
}

static inline uint8 kernel_queue_control_priority_value(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 24) & 0xffULL) << 0));
}

static inline int set_kernel_queue_control_priority_value(uint64 *reg_value,
         uint8 value)
{
 if (value & ~(0xffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffULL) << 24)) |
         (((value >> 0) & (0xffULL)) << 24);

 return 0;
}

static inline bool
kernel_queue_ddr_control_change_requested_value_is_valid(int value)
{
 if (value == 0) {
  return true;
 }
 if (value == 1) {
  return true;
 }
 if (value == 2) {
  return true;
 }
 return false;
}

static inline const char *kernel_queue_ddr_control_change_requested_value_name(
 kernel_queue_ddr_control_change_requested_value value)
{
 if (value == 0) {
  return "REQUEST_ACCEPTED";
 }
 if (value == 1) {
  return "COUNT_BASED_REQUEST";
 }
 if (value == 2) {
  return "BITMAP_BASED_REQUEST";
 }
 return "UNKNOWN VALUE";
}

static inline uint16
kernel_queue_ddr_control_requested_chunks(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 0) & 0xffffULL) << 0));
}

static inline int
set_kernel_queue_ddr_control_requested_chunks(uint64 *reg_value, uint16 value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 0)) |
         (((value >> 0) & (0xffffULL)) << 0);

 return 0;
}

static inline kernel_queue_ddr_control_change_requested_value
kernel_queue_ddr_control_change_requested(const uint64 reg_value)
{
 return (kernel_queue_ddr_control_change_requested_value)(
  (((reg_value >> 16) & 0x3ULL) << 0));
}

static inline int set_kernel_queue_ddr_control_change_requested(
 uint64 *reg_value,
 kernel_queue_ddr_control_change_requested_value value)
{
 if (value & ~(0x3ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x3ULL) << 16)) |
         (((value >> 0) & (0x3ULL)) << 16);

 return 0;
}

static inline bool kernel_queue_ddr_status_value_value_is_valid(int value)
{
 if (value == 0) {
  return true;
 }
 if (value == 1) {
  return true;
 }
 if (value == 2) {
  return true;
 }
 if (value == 3) {
  return true;
 }
 if (value == 4) {
  return true;
 }
 if (value == 5) {
  return true;
 }
 if (value == 6) {
  return true;
 }
 if (value == 7) {
  return true;
 }
 if (value == 8) {
  return true;
 }
 if (value == 9) {
  return true;
 }
 return false;
}

static inline const char *kernel_queue_ddr_status_value_value_name(
 kernel_queue_ddr_status_value_value value)
{
 if (value == 0) {
  return "SUCCESS";
 }
 if (value == 1) {
  return "NOT_ENOUGH_AVAILABLE";
 }
 if (value == 2) {
  return "TOO_LARGE";
 }
 if (value == 3) {
  return "PENDING_CONFIG_IN_PROGRESS";
 }
 if (value == 4) {
  return "QUEUE_NOT_DISABLED";
 }
 if (value == 5) {
  return "INVALID_REQUEST_TYPE";
 }
 if (value == 6) {
  return "CHUNK_ALREADY_RESERVED";
 }
 if (value == 7) {
  return "NO_CHUNK_BITMAP";
 }
 if (value == 8) {
  return "UNKNOWN_ERROR";
 }
 if (value == 9) {
  return "BITMAP_NOT_SUPPORTED";
 }
 return "UNKNOWN VALUE";
}

static inline kernel_queue_ddr_status_value_value
kernel_queue_ddr_status_value(const uint64 reg_value)
{
 return (kernel_queue_ddr_status_value_value)(
  (((reg_value >> 0) & 0xffffULL) << 0));
}

static inline int
set_kernel_queue_ddr_status_value(uint64 *reg_value,
      kernel_queue_ddr_status_value_value value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 0)) |
         (((value >> 0) & (0xffffULL)) << 0);

 return 0;
}

static inline uint16
kernel_queue_ddr_status_current_chunks(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 16) & 0xffffULL) << 0));
}

static inline int set_kernel_queue_ddr_status_current_chunks(uint64 *reg_value,
            uint16 value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 16)) |
         (((value >> 0) & (0xffffULL)) << 16);

 return 0;
}

static inline uint8
kernel_queue_ddr_status_pending_config(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 32) & 0x1ULL) << 0));
}

static inline int set_kernel_queue_ddr_status_pending_config(uint64 *reg_value,
            uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 32)) |
         (((value >> 0) & (0x1ULL)) << 32);

 return 0;
}

#endif
