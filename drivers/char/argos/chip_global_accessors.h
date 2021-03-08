/*
 * Copyright (C) 2021 Google LLC.
 */
# 3 "./drivers/char/argos/chip_global_accessors.h"
#ifndef _DRIVERS_CHAR_ARGOS_CHIP_GLOBAL_ACCESSORS_H_
#define _DRIVERS_CHAR_ARGOS_CHIP_GLOBAL_ACCESSORS_H_ 

#ifdef __KERNEL__
#include "drivers/gasket/gasket_types.h"
#else
#include "platforms/asic_sw/kernel/common/gasket/gasket_types.h"
#endif

enum chip_global_state_value_value {
 kChipGlobalStateValueValuePowerOn = 0,
 kChipGlobalStateValueValueInitialized = 1,
 kChipGlobalStateValueValueBootloaderStart = 2,
 kChipGlobalStateValueValueBootloaderSpiInitDone = 3,
 kChipGlobalStateValueValueBootloaderEnabledIcache = 4,
 kChipGlobalStateValueValueBootloaderFoundImages = 5,
 kChipGlobalStateValueValueBootloaderLoadImage = 6,
 kChipGlobalStateValueValueBootloaderBranchToImage = 7,
 kChipGlobalStateValueValueImageStarting = 8,
 kChipGlobalStateValueValueInitializingOffloadEngines = 9,
 kChipGlobalStateValueValueInitializingRegisterGroups = 10,
 kChipGlobalStateValueValueInitializingRegisterBlocks = 11,
 kChipGlobalStateValueValueInitializingQueueContexts = 12,
 kChipGlobalStateValueValueDoingFunctionalReset = 13,
 kChipGlobalStateValueValueHalted = 14,
 kChipGlobalStateValueValueClearingDdr = 15,
 kChipGlobalStateValueValueInitializingDdr = 16,
 kChipGlobalStateValueValueBootloaderGeneric = 17
};
typedef enum chip_global_state_value_value chip_global_state_value_value;

static inline bool chip_global_state_value_value_is_valid(int value)
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
 if (value == 10) {
  return true;
 }
 if (value == 11) {
  return true;
 }
 if (value == 12) {
  return true;
 }
 if (value == 13) {
  return true;
 }
 if (value == 14) {
  return true;
 }
 if (value == 15) {
  return true;
 }
 if (value == 16) {
  return true;
 }
 if (value == 17) {
  return true;
 }
 return false;
}

static inline const char *
chip_global_state_value_value_name(chip_global_state_value_value value)
{
 if (value == 0) {
  return "POWER_ON";
 }
 if (value == 1) {
  return "INITIALIZED";
 }
 if (value == 2) {
  return "BOOTLOADER_START";
 }
 if (value == 3) {
  return "BOOTLOADER_SPI_INIT_DONE";
 }
 if (value == 4) {
  return "BOOTLOADER_ENABLED_ICACHE";
 }
 if (value == 5) {
  return "BOOTLOADER_FOUND_IMAGES";
 }
 if (value == 6) {
  return "BOOTLOADER_LOAD_IMAGE";
 }
 if (value == 7) {
  return "BOOTLOADER_BRANCH_TO_IMAGE";
 }
 if (value == 8) {
  return "IMAGE_STARTING";
 }
 if (value == 9) {
  return "INITIALIZING_OFFLOAD_ENGINES";
 }
 if (value == 10) {
  return "INITIALIZING_REGISTER_GROUPS";
 }
 if (value == 11) {
  return "INITIALIZING_REGISTER_BLOCKS";
 }
 if (value == 12) {
  return "INITIALIZING_QUEUE_CONTEXTS";
 }
 if (value == 13) {
  return "DOING_FUNCTIONAL_RESET";
 }
 if (value == 14) {
  return "HALTED";
 }
 if (value == 15) {
  return "CLEARING_DDR";
 }
 if (value == 16) {
  return "INITIALIZING_DDR";
 }
 if (value == 17) {
  return "BOOTLOADER_GENERIC";
 }
 return "UNKNOWN VALUE";
}

static inline chip_global_state_value_value
chip_global_state_value(const uint64 reg_value)
{
 return (chip_global_state_value_value)(
  (((reg_value >> 0) & 0xffffULL) << 0));
}

static inline int
set_chip_global_state_value(uint64 *reg_value,
       chip_global_state_value_value value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 0)) |
         (((value >> 0) & (0xffffULL)) << 0);

 return 0;
}

static inline uint16 chip_global_state_sub_value(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 16) & 0xffffULL) << 0));
}

static inline int set_chip_global_state_sub_value(uint64 *reg_value,
        uint16 value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 16)) |
         (((value >> 0) & (0xffffULL)) << 16);

 return 0;
}

static inline uint16 chip_global_ddr_state_total_chunks(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 0) & 0xffffULL) << 0));
}

static inline int set_chip_global_ddr_state_total_chunks(uint64 *reg_value,
        uint16 value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 0)) |
         (((value >> 0) & (0xffffULL)) << 0);

 return 0;
}

static inline uint16
chip_global_ddr_state_available_chunks(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 16) & 0xffffULL) << 0));
}

static inline int set_chip_global_ddr_state_available_chunks(uint64 *reg_value,
            uint16 value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 16)) |
         (((value >> 0) & (0xffffULL)) << 16);

 return 0;
}

static inline uint32 chip_global_firmware_version_major(const uint64 reg_value)
{
 return (uint32)((((reg_value >> 0) & 0xffffffULL) << 0));
}

static inline int set_chip_global_firmware_version_major(uint64 *reg_value,
        uint32 value)
{
 if (value & ~(0xffffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffffULL) << 0)) |
         (((value >> 0) & (0xffffffULL)) << 0);

 return 0;
}

static inline uint32 chip_global_firmware_version_minor(const uint64 reg_value)
{
 return (uint32)((((reg_value >> 24) & 0xffffffULL) << 0));
}

static inline int set_chip_global_firmware_version_minor(uint64 *reg_value,
        uint32 value)
{
 if (value & ~(0xffffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffffULL) << 24)) |
         (((value >> 0) & (0xffffffULL)) << 24);

 return 0;
}

static inline uint8 chip_global_firmware_version_point(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 48) & 0xffULL) << 0));
}

static inline int set_chip_global_firmware_version_point(uint64 *reg_value,
        uint8 value)
{
 if (value & ~(0xffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffULL) << 48)) |
         (((value >> 0) & (0xffULL)) << 48);

 return 0;
}

static inline uint8
chip_global_firmware_version_subpoint(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 56) & 0xffULL) << 0));
}

static inline int set_chip_global_firmware_version_subpoint(uint64 *reg_value,
           uint8 value)
{
 if (value & ~(0xffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffULL) << 56)) |
         (((value >> 0) & (0xffULL)) << 56);

 return 0;
}

static inline uint16 chip_global_api_version_version(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 0) & 0xffffULL) << 0));
}

static inline int set_chip_global_api_version_version(uint64 *reg_value,
            uint16 value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 0)) |
         (((value >> 0) & (0xffffULL)) << 0);

 return 0;
}

static inline uint8 chip_global_pcie_init_count_count(const uint64 reg_value)
{
 return (uint8)((((reg_value >> 0) & 0xffULL) << 0));
}

static inline int set_chip_global_pcie_init_count_count(uint64 *reg_value,
       uint8 value)
{
 if (value & ~(0xffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffULL) << 0)) |
         (((value >> 0) & (0xffULL)) << 0);

 return 0;
}

#endif
