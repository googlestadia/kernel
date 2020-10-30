/*
 * Copyright (C) 2020 Google LLC.
 */
# 3 "./drivers/char/argos/firmware_version_accessors.h"
#ifndef _DRIVERS_CHAR_ARGOS_FIRMWARE_VERSION_ACCESSORS_H_
#define _DRIVERS_CHAR_ARGOS_FIRMWARE_VERSION_ACCESSORS_H_ 

#ifdef __KERNEL__
#include "drivers/gasket/gasket_types.h"
#else
#include "platforms/asic_sw/kernel/common/gasket/gasket_types.h"
#endif

enum firmware_version_image_info_current_type_value {
 kFirmwareVersionImageInfoCurrentTypeValueUnknown = 0,
 kFirmwareVersionImageInfoCurrentTypeValuePrimary = 1,
 kFirmwareVersionImageInfoCurrentTypeValueSecondary = 2
};
typedef enum firmware_version_image_info_current_type_value
 firmware_version_image_info_current_type_value;

static inline uint16
firmware_version_primary_version_major(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 48) & 0xffffULL) << 0));
}

static inline int set_firmware_version_primary_version_major(uint64 *reg_value,
            uint16 value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 48)) |
         (((value >> 0) & (0xffffULL)) << 48);

 return 0;
}

static inline uint16
firmware_version_primary_version_minor(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 32) & 0xffffULL) << 0));
}

static inline int set_firmware_version_primary_version_minor(uint64 *reg_value,
            uint16 value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 32)) |
         (((value >> 0) & (0xffffULL)) << 32);

 return 0;
}

static inline uint16
firmware_version_primary_version_point(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 16) & 0xffffULL) << 0));
}

static inline int set_firmware_version_primary_version_point(uint64 *reg_value,
            uint16 value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 16)) |
         (((value >> 0) & (0xffffULL)) << 16);

 return 0;
}

static inline uint16
firmware_version_primary_version_subpoint(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 0) & 0xffffULL) << 0));
}

static inline int
set_firmware_version_primary_version_subpoint(uint64 *reg_value, uint16 value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 0)) |
         (((value >> 0) & (0xffffULL)) << 0);

 return 0;
}

static inline uint16
firmware_version_secondary_version_major(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 48) & 0xffffULL) << 0));
}

static inline int
set_firmware_version_secondary_version_major(uint64 *reg_value, uint16 value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 48)) |
         (((value >> 0) & (0xffffULL)) << 48);

 return 0;
}

static inline uint16
firmware_version_secondary_version_minor(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 32) & 0xffffULL) << 0));
}

static inline int
set_firmware_version_secondary_version_minor(uint64 *reg_value, uint16 value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 32)) |
         (((value >> 0) & (0xffffULL)) << 32);

 return 0;
}

static inline uint16
firmware_version_secondary_version_point(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 16) & 0xffffULL) << 0));
}

static inline int
set_firmware_version_secondary_version_point(uint64 *reg_value, uint16 value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 16)) |
         (((value >> 0) & (0xffffULL)) << 16);

 return 0;
}

static inline uint16
firmware_version_secondary_version_subpoint(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 0) & 0xffffULL) << 0));
}

static inline int
set_firmware_version_secondary_version_subpoint(uint64 *reg_value, uint16 value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 0)) |
         (((value >> 0) & (0xffffULL)) << 0);

 return 0;
}

static inline bool
firmware_version_image_info_current_type_value_is_valid(int value)
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

static inline const char *firmware_version_image_info_current_type_value_name(
 firmware_version_image_info_current_type_value value)
{
 if (value == 0) {
  return "UNKNOWN";
 }
 if (value == 1) {
  return "PRIMARY";
 }
 if (value == 2) {
  return "SECONDARY";
 }
 return "UNKNOWN VALUE";
}

static inline firmware_version_image_info_current_type_value
firmware_version_image_info_current_type(const uint64 reg_value)
{
 return (firmware_version_image_info_current_type_value)(
  (((reg_value >> 0) & 0x3ULL) << 0));
}

static inline int set_firmware_version_image_info_current_type(
 uint64 *reg_value, firmware_version_image_info_current_type_value value)
{
 if (value & ~(0x3ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x3ULL) << 0)) |
         (((value >> 0) & (0x3ULL)) << 0);

 return 0;
}

static inline uint32 firmware_version_image_info_id(const uint64 reg_value)
{
 return (uint32)((((reg_value >> 32) & 0xffffffffULL) << 0));
}

static inline int set_firmware_version_image_info_id(uint64 *reg_value,
           uint32 value)
{
 if (value & ~(0xffffffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffffffULL) << 32)) |
         (((value >> 0) & (0xffffffffULL)) << 32);

 return 0;
}

#endif
