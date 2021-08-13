/*
 * Copyright (C) 2021 Google LLC.
 */
# 3 "./drivers/char/argos/kernel_chip_global_accessors.h"
#ifndef _DRIVERS_CHAR_ARGOS_KERNEL_CHIP_GLOBAL_ACCESSORS_H_
#define _DRIVERS_CHAR_ARGOS_KERNEL_CHIP_GLOBAL_ACCESSORS_H_ 

#ifdef __KERNEL__
#include "drivers/gasket/gasket_types.h"
#else
#include "platforms/asic_sw/kernel/common/gasket/gasket_types.h"
#endif

enum kernel_chip_global_priority_algorithm_priority_0_value {
 kKernelChipGlobalPriorityAlgorithmPriority0ValueRoundRobin = 0,
 kKernelChipGlobalPriorityAlgorithmPriority0ValueWaterfall = 1
};
typedef enum kernel_chip_global_priority_algorithm_priority_0_value
 kernel_chip_global_priority_algorithm_priority_0_value;

enum kernel_chip_global_priority_algorithm_priority_1_value {
 kKernelChipGlobalPriorityAlgorithmPriority1ValueRoundRobin = 0,
 kKernelChipGlobalPriorityAlgorithmPriority1ValueWaterfall = 1
};
typedef enum kernel_chip_global_priority_algorithm_priority_1_value
 kernel_chip_global_priority_algorithm_priority_1_value;

enum kernel_chip_global_priority_algorithm_priority_2_value {
 kKernelChipGlobalPriorityAlgorithmPriority2ValueRoundRobin = 0,
 kKernelChipGlobalPriorityAlgorithmPriority2ValueWaterfall = 1
};
typedef enum kernel_chip_global_priority_algorithm_priority_2_value
 kernel_chip_global_priority_algorithm_priority_2_value;

enum kernel_chip_global_priority_algorithm_priority_3_value {
 kKernelChipGlobalPriorityAlgorithmPriority3ValueRoundRobin = 0,
 kKernelChipGlobalPriorityAlgorithmPriority3ValueWaterfall = 1
};
typedef enum kernel_chip_global_priority_algorithm_priority_3_value
 kernel_chip_global_priority_algorithm_priority_3_value;

enum kernel_chip_global_priority_algorithm_priority_4_value {
 kKernelChipGlobalPriorityAlgorithmPriority4ValueRoundRobin = 0,
 kKernelChipGlobalPriorityAlgorithmPriority4ValueWaterfall = 1
};
typedef enum kernel_chip_global_priority_algorithm_priority_4_value
 kernel_chip_global_priority_algorithm_priority_4_value;

enum kernel_chip_global_priority_algorithm_priority_5_value {
 kKernelChipGlobalPriorityAlgorithmPriority5ValueRoundRobin = 0,
 kKernelChipGlobalPriorityAlgorithmPriority5ValueWaterfall = 1
};
typedef enum kernel_chip_global_priority_algorithm_priority_5_value
 kernel_chip_global_priority_algorithm_priority_5_value;

enum kernel_chip_global_priority_algorithm_priority_6_value {
 kKernelChipGlobalPriorityAlgorithmPriority6ValueRoundRobin = 0,
 kKernelChipGlobalPriorityAlgorithmPriority6ValueWaterfall = 1
};
typedef enum kernel_chip_global_priority_algorithm_priority_6_value
 kernel_chip_global_priority_algorithm_priority_6_value;

enum kernel_chip_global_priority_algorithm_priority_7_value {
 kKernelChipGlobalPriorityAlgorithmPriority7ValueRoundRobin = 0,
 kKernelChipGlobalPriorityAlgorithmPriority7ValueWaterfall = 1
};
typedef enum kernel_chip_global_priority_algorithm_priority_7_value
 kernel_chip_global_priority_algorithm_priority_7_value;

enum kernel_chip_global_priority_algorithm_priority_value {
 kKernelChipGlobalPriorityAlgorithmPriorityValueRoundRobin = 0,
 kKernelChipGlobalPriorityAlgorithmPriorityValueWaterfall = 1
};
typedef enum kernel_chip_global_priority_algorithm_priority_value
 kernel_chip_global_priority_algorithm_priority_value;

enum kernel_chip_global_chip_reset_value_value {
 kKernelChipGlobalChipResetValueValueResetAccepted = 0,
 kKernelChipGlobalChipResetValueValueFunctionalReset = 1
};
typedef enum kernel_chip_global_chip_reset_value_value
 kernel_chip_global_chip_reset_value_value;

enum kernel_chip_global_max_ddr_chunks_per_ctx_valid_value {
 kKernelChipGlobalMaxDdrChunksPerCtxValidValueInvalid = 0,
 kKernelChipGlobalMaxDdrChunksPerCtxValidValueValid = 1
};
typedef enum kernel_chip_global_max_ddr_chunks_per_ctx_valid_value
 kernel_chip_global_max_ddr_chunks_per_ctx_valid_value;

enum kernel_chip_global_codec_reservation_control_control_value {
 kKernelChipGlobalCodecReservationControlControlValueChangeAccepted = 0,
 kKernelChipGlobalCodecReservationControlControlValueChangeRequested = 1
};
typedef enum kernel_chip_global_codec_reservation_control_control_value
 kernel_chip_global_codec_reservation_control_control_value;

enum kernel_chip_global_codec_reservation_status_status_value {
 kKernelChipGlobalCodecReservationStatusStatusValueRequestSucceeded = 0,
 kKernelChipGlobalCodecReservationStatusStatusValueRequestFailed = 1,
 kKernelChipGlobalCodecReservationStatusStatusValueRequestPending = 2
};
typedef enum kernel_chip_global_codec_reservation_status_status_value
 kernel_chip_global_codec_reservation_status_status_value;

static inline bool
kernel_chip_global_priority_algorithm_priority_0_value_is_valid(int value)
{
 if (value == 0) {
  return true;
 }
 if (value == 1) {
  return true;
 }
 return false;
}

static inline const char *
kernel_chip_global_priority_algorithm_priority_0_value_name(
 kernel_chip_global_priority_algorithm_priority_0_value value)
{
 if (value == 0) {
  return "ROUND_ROBIN";
 }
 if (value == 1) {
  return "WATERFALL";
 }
 return "UNKNOWN VALUE";
}

static inline bool
kernel_chip_global_priority_algorithm_priority_1_value_is_valid(int value)
{
 if (value == 0) {
  return true;
 }
 if (value == 1) {
  return true;
 }
 return false;
}

static inline const char *
kernel_chip_global_priority_algorithm_priority_1_value_name(
 kernel_chip_global_priority_algorithm_priority_1_value value)
{
 if (value == 0) {
  return "ROUND_ROBIN";
 }
 if (value == 1) {
  return "WATERFALL";
 }
 return "UNKNOWN VALUE";
}

static inline bool
kernel_chip_global_priority_algorithm_priority_2_value_is_valid(int value)
{
 if (value == 0) {
  return true;
 }
 if (value == 1) {
  return true;
 }
 return false;
}

static inline const char *
kernel_chip_global_priority_algorithm_priority_2_value_name(
 kernel_chip_global_priority_algorithm_priority_2_value value)
{
 if (value == 0) {
  return "ROUND_ROBIN";
 }
 if (value == 1) {
  return "WATERFALL";
 }
 return "UNKNOWN VALUE";
}

static inline bool
kernel_chip_global_priority_algorithm_priority_3_value_is_valid(int value)
{
 if (value == 0) {
  return true;
 }
 if (value == 1) {
  return true;
 }
 return false;
}

static inline const char *
kernel_chip_global_priority_algorithm_priority_3_value_name(
 kernel_chip_global_priority_algorithm_priority_3_value value)
{
 if (value == 0) {
  return "ROUND_ROBIN";
 }
 if (value == 1) {
  return "WATERFALL";
 }
 return "UNKNOWN VALUE";
}

static inline bool
kernel_chip_global_priority_algorithm_priority_4_value_is_valid(int value)
{
 if (value == 0) {
  return true;
 }
 if (value == 1) {
  return true;
 }
 return false;
}

static inline const char *
kernel_chip_global_priority_algorithm_priority_4_value_name(
 kernel_chip_global_priority_algorithm_priority_4_value value)
{
 if (value == 0) {
  return "ROUND_ROBIN";
 }
 if (value == 1) {
  return "WATERFALL";
 }
 return "UNKNOWN VALUE";
}

static inline bool
kernel_chip_global_priority_algorithm_priority_5_value_is_valid(int value)
{
 if (value == 0) {
  return true;
 }
 if (value == 1) {
  return true;
 }
 return false;
}

static inline const char *
kernel_chip_global_priority_algorithm_priority_5_value_name(
 kernel_chip_global_priority_algorithm_priority_5_value value)
{
 if (value == 0) {
  return "ROUND_ROBIN";
 }
 if (value == 1) {
  return "WATERFALL";
 }
 return "UNKNOWN VALUE";
}

static inline bool
kernel_chip_global_priority_algorithm_priority_6_value_is_valid(int value)
{
 if (value == 0) {
  return true;
 }
 if (value == 1) {
  return true;
 }
 return false;
}

static inline const char *
kernel_chip_global_priority_algorithm_priority_6_value_name(
 kernel_chip_global_priority_algorithm_priority_6_value value)
{
 if (value == 0) {
  return "ROUND_ROBIN";
 }
 if (value == 1) {
  return "WATERFALL";
 }
 return "UNKNOWN VALUE";
}

static inline bool
kernel_chip_global_priority_algorithm_priority_7_value_is_valid(int value)
{
 if (value == 0) {
  return true;
 }
 if (value == 1) {
  return true;
 }
 return false;
}

static inline const char *
kernel_chip_global_priority_algorithm_priority_7_value_name(
 kernel_chip_global_priority_algorithm_priority_7_value value)
{
 if (value == 0) {
  return "ROUND_ROBIN";
 }
 if (value == 1) {
  return "WATERFALL";
 }
 return "UNKNOWN VALUE";
}

static inline bool
kernel_chip_global_priority_algorithm_priority_value_is_valid(int value)
{
 if (value == 0) {
  return true;
 }
 if (value == 1) {
  return true;
 }
 return false;
}

static inline const char *
kernel_chip_global_priority_algorithm_priority_value_name(
 kernel_chip_global_priority_algorithm_priority_value value)
{
 if (value == 0) {
  return "ROUND_ROBIN";
 }
 if (value == 1) {
  return "WATERFALL";
 }
 return "UNKNOWN VALUE";
}

static inline kernel_chip_global_priority_algorithm_priority_0_value
kernel_chip_global_priority_algorithm_priority_0(const uint64 reg_value)
{
 return (kernel_chip_global_priority_algorithm_priority_0_value)(
  (((reg_value >> 0) & 0xffULL) << 0));
}

static inline int set_kernel_chip_global_priority_algorithm_priority_0(
 uint64 *reg_value,
 kernel_chip_global_priority_algorithm_priority_0_value value)
{
 if (value & ~(0xffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffULL) << 0)) |
         (((value >> 0) & (0xffULL)) << 0);

 return 0;
}

static inline kernel_chip_global_priority_algorithm_priority_1_value
kernel_chip_global_priority_algorithm_priority_1(const uint64 reg_value)
{
 return (kernel_chip_global_priority_algorithm_priority_1_value)(
  (((reg_value >> 8) & 0xffULL) << 0));
}

static inline int set_kernel_chip_global_priority_algorithm_priority_1(
 uint64 *reg_value,
 kernel_chip_global_priority_algorithm_priority_1_value value)
{
 if (value & ~(0xffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffULL) << 8)) |
         (((value >> 0) & (0xffULL)) << 8);

 return 0;
}

static inline kernel_chip_global_priority_algorithm_priority_2_value
kernel_chip_global_priority_algorithm_priority_2(const uint64 reg_value)
{
 return (kernel_chip_global_priority_algorithm_priority_2_value)(
  (((reg_value >> 16) & 0xffULL) << 0));
}

static inline int set_kernel_chip_global_priority_algorithm_priority_2(
 uint64 *reg_value,
 kernel_chip_global_priority_algorithm_priority_2_value value)
{
 if (value & ~(0xffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffULL) << 16)) |
         (((value >> 0) & (0xffULL)) << 16);

 return 0;
}

static inline kernel_chip_global_priority_algorithm_priority_3_value
kernel_chip_global_priority_algorithm_priority_3(const uint64 reg_value)
{
 return (kernel_chip_global_priority_algorithm_priority_3_value)(
  (((reg_value >> 24) & 0xffULL) << 0));
}

static inline int set_kernel_chip_global_priority_algorithm_priority_3(
 uint64 *reg_value,
 kernel_chip_global_priority_algorithm_priority_3_value value)
{
 if (value & ~(0xffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffULL) << 24)) |
         (((value >> 0) & (0xffULL)) << 24);

 return 0;
}

static inline kernel_chip_global_priority_algorithm_priority_4_value
kernel_chip_global_priority_algorithm_priority_4(const uint64 reg_value)
{
 return (kernel_chip_global_priority_algorithm_priority_4_value)(
  (((reg_value >> 32) & 0xffULL) << 0));
}

static inline int set_kernel_chip_global_priority_algorithm_priority_4(
 uint64 *reg_value,
 kernel_chip_global_priority_algorithm_priority_4_value value)
{
 if (value & ~(0xffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffULL) << 32)) |
         (((value >> 0) & (0xffULL)) << 32);

 return 0;
}

static inline kernel_chip_global_priority_algorithm_priority_5_value
kernel_chip_global_priority_algorithm_priority_5(const uint64 reg_value)
{
 return (kernel_chip_global_priority_algorithm_priority_5_value)(
  (((reg_value >> 40) & 0xffULL) << 0));
}

static inline int set_kernel_chip_global_priority_algorithm_priority_5(
 uint64 *reg_value,
 kernel_chip_global_priority_algorithm_priority_5_value value)
{
 if (value & ~(0xffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffULL) << 40)) |
         (((value >> 0) & (0xffULL)) << 40);

 return 0;
}

static inline kernel_chip_global_priority_algorithm_priority_6_value
kernel_chip_global_priority_algorithm_priority_6(const uint64 reg_value)
{
 return (kernel_chip_global_priority_algorithm_priority_6_value)(
  (((reg_value >> 48) & 0xffULL) << 0));
}

static inline int set_kernel_chip_global_priority_algorithm_priority_6(
 uint64 *reg_value,
 kernel_chip_global_priority_algorithm_priority_6_value value)
{
 if (value & ~(0xffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffULL) << 48)) |
         (((value >> 0) & (0xffULL)) << 48);

 return 0;
}

static inline kernel_chip_global_priority_algorithm_priority_7_value
kernel_chip_global_priority_algorithm_priority_7(const uint64 reg_value)
{
 return (kernel_chip_global_priority_algorithm_priority_7_value)(
  (((reg_value >> 56) & 0xffULL) << 0));
}

static inline int set_kernel_chip_global_priority_algorithm_priority_7(
 uint64 *reg_value,
 kernel_chip_global_priority_algorithm_priority_7_value value)
{
 if (value & ~(0xffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffULL) << 56)) |
         (((value >> 0) & (0xffULL)) << 56);

 return 0;
}

static inline int kernel_chip_global_priority_algorithm_priority_size(void)
{
 return 8;
}

static inline kernel_chip_global_priority_algorithm_priority_value
kernel_chip_global_priority_algorithm_priority(const uint64 reg_value,
            int index)
{
 switch (index) {
 case (0): {
  return (kernel_chip_global_priority_algorithm_priority_value)(
   kernel_chip_global_priority_algorithm_priority_0(
    reg_value));
 }
 case (1): {
  return (kernel_chip_global_priority_algorithm_priority_value)(
   kernel_chip_global_priority_algorithm_priority_1(
    reg_value));
 }
 case (2): {
  return (kernel_chip_global_priority_algorithm_priority_value)(
   kernel_chip_global_priority_algorithm_priority_2(
    reg_value));
 }
 case (3): {
  return (kernel_chip_global_priority_algorithm_priority_value)(
   kernel_chip_global_priority_algorithm_priority_3(
    reg_value));
 }
 case (4): {
  return (kernel_chip_global_priority_algorithm_priority_value)(
   kernel_chip_global_priority_algorithm_priority_4(
    reg_value));
 }
 case (5): {
  return (kernel_chip_global_priority_algorithm_priority_value)(
   kernel_chip_global_priority_algorithm_priority_5(
    reg_value));
 }
 case (6): {
  return (kernel_chip_global_priority_algorithm_priority_value)(
   kernel_chip_global_priority_algorithm_priority_6(
    reg_value));
 }
 case (7): {
  return (kernel_chip_global_priority_algorithm_priority_value)(
   kernel_chip_global_priority_algorithm_priority_7(
    reg_value));
 }
 default:
  return (kernel_chip_global_priority_algorithm_priority_value)(
   -1);
 }
}

static inline int set_kernel_chip_global_priority_algorithm_priority(
 uint64 *reg_value, int index,
 kernel_chip_global_priority_algorithm_priority_value value)
{
 switch (index) {
 case (0): {
  return set_kernel_chip_global_priority_algorithm_priority_0(
   reg_value,
   (kernel_chip_global_priority_algorithm_priority_0_value)
    value);
 }
 case (1): {
  return set_kernel_chip_global_priority_algorithm_priority_1(
   reg_value,
   (kernel_chip_global_priority_algorithm_priority_1_value)
    value);
 }
 case (2): {
  return set_kernel_chip_global_priority_algorithm_priority_2(
   reg_value,
   (kernel_chip_global_priority_algorithm_priority_2_value)
    value);
 }
 case (3): {
  return set_kernel_chip_global_priority_algorithm_priority_3(
   reg_value,
   (kernel_chip_global_priority_algorithm_priority_3_value)
    value);
 }
 case (4): {
  return set_kernel_chip_global_priority_algorithm_priority_4(
   reg_value,
   (kernel_chip_global_priority_algorithm_priority_4_value)
    value);
 }
 case (5): {
  return set_kernel_chip_global_priority_algorithm_priority_5(
   reg_value,
   (kernel_chip_global_priority_algorithm_priority_5_value)
    value);
 }
 case (6): {
  return set_kernel_chip_global_priority_algorithm_priority_6(
   reg_value,
   (kernel_chip_global_priority_algorithm_priority_6_value)
    value);
 }
 case (7): {
  return set_kernel_chip_global_priority_algorithm_priority_7(
   reg_value,
   (kernel_chip_global_priority_algorithm_priority_7_value)
    value);
 }
 default:
  return 1;
 }
}

static inline bool kernel_chip_global_chip_reset_value_value_is_valid(int value)
{
 if (value == 0) {
  return true;
 }
 if (value == 1) {
  return true;
 }
 return false;
}

static inline const char *kernel_chip_global_chip_reset_value_value_name(
 kernel_chip_global_chip_reset_value_value value)
{
 if (value == 0) {
  return "RESET_ACCEPTED";
 }
 if (value == 1) {
  return "FUNCTIONAL_RESET";
 }
 return "UNKNOWN VALUE";
}

static inline kernel_chip_global_chip_reset_value_value
kernel_chip_global_chip_reset_value(const uint64 reg_value)
{
 return (kernel_chip_global_chip_reset_value_value)(
  (((reg_value >> 0) & 0x1ULL) << 0));
}

static inline int set_kernel_chip_global_chip_reset_value(
 uint64 *reg_value, kernel_chip_global_chip_reset_value_value value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 0)) |
         (((value >> 0) & (0x1ULL)) << 0);

 return 0;
}

static inline bool
kernel_chip_global_max_ddr_chunks_per_ctx_valid_value_is_valid(int value)
{
 if (value == 0) {
  return true;
 }
 if (value == 1) {
  return true;
 }
 return false;
}

static inline const char *
kernel_chip_global_max_ddr_chunks_per_ctx_valid_value_name(
 kernel_chip_global_max_ddr_chunks_per_ctx_valid_value value)
{
 if (value == 0) {
  return "INVALID";
 }
 if (value == 1) {
  return "VALID";
 }
 return "UNKNOWN VALUE";
}

static inline kernel_chip_global_max_ddr_chunks_per_ctx_valid_value
kernel_chip_global_max_ddr_chunks_per_ctx_valid(const uint64 reg_value)
{
 return (kernel_chip_global_max_ddr_chunks_per_ctx_valid_value)(
  (((reg_value >> 0) & 0x1ULL) << 0));
}

static inline int set_kernel_chip_global_max_ddr_chunks_per_ctx_valid(
 uint64 *reg_value,
 kernel_chip_global_max_ddr_chunks_per_ctx_valid_value value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 0)) |
         (((value >> 0) & (0x1ULL)) << 0);

 return 0;
}

static inline uint64
kernel_chip_global_max_ddr_chunks_per_ctx_chunks(const uint64 reg_value)
{
 return (uint64)((((reg_value >> 8) & 0xffffffffffffffULL) << 0));
}

static inline int
set_kernel_chip_global_max_ddr_chunks_per_ctx_chunks(uint64 *reg_value,
           uint64 value)
{
 if (value & ~(0xffffffffffffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffffffffffffULL) << 8)) |
         (((value >> 0) & (0xffffffffffffffULL)) << 8);

 return 0;
}

static inline bool
kernel_chip_global_codec_reservation_control_control_value_is_valid(int value)
{
 if (value == 0) {
  return true;
 }
 if (value == 1) {
  return true;
 }
 return false;
}

static inline const char *
kernel_chip_global_codec_reservation_control_control_value_name(
 kernel_chip_global_codec_reservation_control_control_value value)
{
 if (value == 0) {
  return "CHANGE_ACCEPTED";
 }
 if (value == 1) {
  return "CHANGE_REQUESTED";
 }
 return "UNKNOWN VALUE";
}

static inline kernel_chip_global_codec_reservation_control_control_value
kernel_chip_global_codec_reservation_control_control(const uint64 reg_value)
{
 return (kernel_chip_global_codec_reservation_control_control_value)(
  (((reg_value >> 0) & 0x1ULL) << 0));
}

static inline int set_kernel_chip_global_codec_reservation_control_control(
 uint64 *reg_value,
 kernel_chip_global_codec_reservation_control_control_value value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 0)) |
         (((value >> 0) & (0x1ULL)) << 0);

 return 0;
}

static inline uint16
kernel_chip_global_codec_reservation_control_queue(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 1) & 0x3ffULL) << 0));
}

static inline int
set_kernel_chip_global_codec_reservation_control_queue(uint64 *reg_value,
             uint16 value)
{
 if (value & ~(0x3ffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x3ffULL) << 1)) |
         (((value >> 0) & (0x3ffULL)) << 1);

 return 0;
}

static inline uint8
kernel_chip_global_codec_reservation_control_bigsea_reservation(
 const uint64 reg_value)
{
 return (uint8)((((reg_value >> 11) & 0xffULL) << 0));
}

static inline int
set_kernel_chip_global_codec_reservation_control_bigsea_reservation(
 uint64 *reg_value, uint8 value)
{
 if (value & ~(0xffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffULL) << 11)) |
         (((value >> 0) & (0xffULL)) << 11);

 return 0;
}

static inline uint8 kernel_chip_global_codec_reservation_control_g1_reservation(
 const uint64 reg_value)
{
 return (uint8)((((reg_value >> 19) & 0xffULL) << 0));
}

static inline int
set_kernel_chip_global_codec_reservation_control_g1_reservation(
 uint64 *reg_value, uint8 value)
{
 if (value & ~(0xffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffULL) << 19)) |
         (((value >> 0) & (0xffULL)) << 19);

 return 0;
}

static inline bool
kernel_chip_global_codec_reservation_status_status_value_is_valid(int value)
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

static inline const char *
kernel_chip_global_codec_reservation_status_status_value_name(
 kernel_chip_global_codec_reservation_status_status_value value)
{
 if (value == 0) {
  return "REQUEST_SUCCEEDED";
 }
 if (value == 1) {
  return "REQUEST_FAILED";
 }
 if (value == 2) {
  return "REQUEST_PENDING";
 }
 return "UNKNOWN VALUE";
}

static inline kernel_chip_global_codec_reservation_status_status_value
kernel_chip_global_codec_reservation_status_status(const uint64 reg_value)
{
 return (kernel_chip_global_codec_reservation_status_status_value)(
  (((reg_value >> 0) & 0x3ULL) << 0));
}

static inline int set_kernel_chip_global_codec_reservation_status_status(
 uint64 *reg_value,
 kernel_chip_global_codec_reservation_status_status_value value)
{
 if (value & ~(0x3ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x3ULL) << 0)) |
         (((value >> 0) & (0x3ULL)) << 0);

 return 0;
}

static inline uint8 kernel_chip_global_codec_reservation_status_common_bigseas(
 const uint64 reg_value)
{
 return (uint8)((((reg_value >> 2) & 0xffULL) << 0));
}

static inline int
set_kernel_chip_global_codec_reservation_status_common_bigseas(
 uint64 *reg_value, uint8 value)
{
 if (value & ~(0xffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffULL) << 2)) |
         (((value >> 0) & (0xffULL)) << 2);

 return 0;
}

static inline uint8
kernel_chip_global_codec_reservation_status_common_g1s(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 10) & 0xffULL) << 0));
}

static inline int
set_kernel_chip_global_codec_reservation_status_common_g1s(uint64 *reg_value,
          uint8 value)
{
 if (value & ~(0xffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffULL) << 10)) |
         (((value >> 0) & (0xffULL)) << 10);

 return 0;
}

static inline uint8 kernel_chip_global_periodic_ddr_retraining_control_enable(
 const uint64 reg_value)
{
 return (uint8)((((reg_value >> 0) & 0x1ULL) << 0));
}

static inline int
set_kernel_chip_global_periodic_ddr_retraining_control_enable(uint64 *reg_value,
             uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 0)) |
         (((value >> 0) & (0x1ULL)) << 0);

 return 0;
}

static inline uint8
kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_0(
 const uint64 reg_value)
{
 return (uint8)((((reg_value >> 1) & 0x1ULL) << 0));
}

static inline int
set_kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_0(
 uint64 *reg_value, uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 1)) |
         (((value >> 0) & (0x1ULL)) << 1);

 return 0;
}

static inline uint8
kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_1(
 const uint64 reg_value)
{
 return (uint8)((((reg_value >> 2) & 0x1ULL) << 0));
}

static inline int
set_kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_1(
 uint64 *reg_value, uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 2)) |
         (((value >> 0) & (0x1ULL)) << 2);

 return 0;
}

static inline uint8
kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_2(
 const uint64 reg_value)
{
 return (uint8)((((reg_value >> 3) & 0x1ULL) << 0));
}

static inline int
set_kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_2(
 uint64 *reg_value, uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 3)) |
         (((value >> 0) & (0x1ULL)) << 3);

 return 0;
}

static inline uint8
kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_3(
 const uint64 reg_value)
{
 return (uint8)((((reg_value >> 4) & 0x1ULL) << 0));
}

static inline int
set_kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_3(
 uint64 *reg_value, uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 4)) |
         (((value >> 0) & (0x1ULL)) << 4);

 return 0;
}

static inline uint8
kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_4(
 const uint64 reg_value)
{
 return (uint8)((((reg_value >> 5) & 0x1ULL) << 0));
}

static inline int
set_kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_4(
 uint64 *reg_value, uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 5)) |
         (((value >> 0) & (0x1ULL)) << 5);

 return 0;
}

static inline uint8
kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_5(
 const uint64 reg_value)
{
 return (uint8)((((reg_value >> 6) & 0x1ULL) << 0));
}

static inline int
set_kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_5(
 uint64 *reg_value, uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 6)) |
         (((value >> 0) & (0x1ULL)) << 6);

 return 0;
}

static inline int
kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_size(
 void)
{
 return 6;
}

static inline uint8
kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy(
 const uint64 reg_value, int index)
{
 switch (index) {
 case (0): {
  return (uint8)(
   kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_0(
    reg_value));
 }
 case (1): {
  return (uint8)(
   kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_1(
    reg_value));
 }
 case (2): {
  return (uint8)(
   kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_2(
    reg_value));
 }
 case (3): {
  return (uint8)(
   kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_3(
    reg_value));
 }
 case (4): {
  return (uint8)(
   kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_4(
    reg_value));
 }
 case (5): {
  return (uint8)(
   kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_5(
    reg_value));
 }
 default:
  return (uint8)(-1);
 }
}

static inline int
set_kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy(
 uint64 *reg_value, int index, uint8 value)
{
 switch (index) {
 case (0): {
  return set_kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_0(
   reg_value, (uint8)value);
 }
 case (1): {
  return set_kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_1(
   reg_value, (uint8)value);
 }
 case (2): {
  return set_kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_2(
   reg_value, (uint8)value);
 }
 case (3): {
  return set_kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_3(
   reg_value, (uint8)value);
 }
 case (4): {
  return set_kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_4(
   reg_value, (uint8)value);
 }
 case (5): {
  return set_kernel_chip_global_periodic_ddr_retraining_control_request_retraining_phy_5(
   reg_value, (uint8)value);
 }
 default:
  return 1;
 }
}

static inline uint8 kernel_chip_global_periodic_ddr_retraining_status_enabled(
 const uint64 reg_value)
{
 return (uint8)((((reg_value >> 0) & 0x1ULL) << 0));
}

static inline int
set_kernel_chip_global_periodic_ddr_retraining_status_enabled(uint64 *reg_value,
             uint8 value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 0)) |
         (((value >> 0) & (0x1ULL)) << 0);

 return 0;
}

#endif
