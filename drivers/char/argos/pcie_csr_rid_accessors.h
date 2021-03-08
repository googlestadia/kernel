/*
 * Copyright (C) 2021 Google LLC.
 */
# 3 "./drivers/char/argos/pcie_csr_rid_accessors.h"
#ifndef _DRIVERS_CHAR_ARGOS_PCIE_CSR_RID_ACCESSORS_H_
#define _DRIVERS_CHAR_ARGOS_PCIE_CSR_RID_ACCESSORS_H_ 

#ifdef __KERNEL__
#include "drivers/gasket/gasket_types.h"
#else
#include "platforms/asic_sw/kernel/common/gasket/gasket_types.h"
#endif

enum pcie_csr_rid_control_enable_value {
 kPcieCsrRidControlEnableValueDisable = 0,
 kPcieCsrRidControlEnableValueEnable = 1
};
typedef enum pcie_csr_rid_control_enable_value pcie_csr_rid_control_enable_value;

enum pcie_csr_rid_control_error_on_fault_value {
 kPcieCsrRidControlErrorOnFaultValueDisable = 0,
 kPcieCsrRidControlErrorOnFaultValueEnable = 1
};
typedef enum pcie_csr_rid_control_error_on_fault_value
 pcie_csr_rid_control_error_on_fault_value;

enum pcie_csr_rid_clear_first_fault_read_value {
 kPcieCsrRidClearFirstFaultReadValueNoClear = 0,
 kPcieCsrRidClearFirstFaultReadValueClear = 1
};
typedef enum pcie_csr_rid_clear_first_fault_read_value
 pcie_csr_rid_clear_first_fault_read_value;

enum pcie_csr_rid_clear_first_fault_write_value {
 kPcieCsrRidClearFirstFaultWriteValueNoClear = 0,
 kPcieCsrRidClearFirstFaultWriteValueClear = 1
};
typedef enum pcie_csr_rid_clear_first_fault_write_value
 pcie_csr_rid_clear_first_fault_write_value;

enum pcie_csr_rid_fault_read_value {
 kPcieCsrRidFaultReadValueNoFault = 0,
 kPcieCsrRidFaultReadValueReadFault = 1
};
typedef enum pcie_csr_rid_fault_read_value pcie_csr_rid_fault_read_value;

enum pcie_csr_rid_fault_write_value {
 kPcieCsrRidFaultWriteValueNoFault = 0,
 kPcieCsrRidFaultWriteValueWriteFault = 1
};
typedef enum pcie_csr_rid_fault_write_value pcie_csr_rid_fault_write_value;

enum pcie_csr_rid_base_addr_read_valid_value {
 kPcieCsrRidBaseAddrReadValidValueReadsNotAllowed = 0,
 kPcieCsrRidBaseAddrReadValidValueReadsAllowed = 1
};
typedef enum pcie_csr_rid_base_addr_read_valid_value
 pcie_csr_rid_base_addr_read_valid_value;

enum pcie_csr_rid_base_addr_write_valid_value {
 kPcieCsrRidBaseAddrWriteValidValueWritesNotAllowed = 0,
 kPcieCsrRidBaseAddrWriteValidValueWritesAllowed = 1
};
typedef enum pcie_csr_rid_base_addr_write_valid_value
 pcie_csr_rid_base_addr_write_valid_value;

static inline bool pcie_csr_rid_control_enable_value_is_valid(int value)
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
pcie_csr_rid_control_enable_value_name(pcie_csr_rid_control_enable_value value)
{
 if (value == 0) {
  return "DISABLE";
 }
 if (value == 1) {
  return "ENABLE";
 }
 return "UNKNOWN VALUE";
}

static inline bool pcie_csr_rid_control_error_on_fault_value_is_valid(int value)
{
 if (value == 0) {
  return true;
 }
 if (value == 1) {
  return true;
 }
 return false;
}

static inline const char *pcie_csr_rid_control_error_on_fault_value_name(
 pcie_csr_rid_control_error_on_fault_value value)
{
 if (value == 0) {
  return "DISABLE";
 }
 if (value == 1) {
  return "ENABLE";
 }
 return "UNKNOWN VALUE";
}

static inline pcie_csr_rid_control_enable_value
pcie_csr_rid_control_enable(const uint64 reg_value)
{
 return (pcie_csr_rid_control_enable_value)(
  (((reg_value >> 0) & 0x1ULL) << 0));
}

static inline int
set_pcie_csr_rid_control_enable(uint64 *reg_value,
    pcie_csr_rid_control_enable_value value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 0)) |
         (((value >> 0) & (0x1ULL)) << 0);

 return 0;
}

static inline pcie_csr_rid_control_error_on_fault_value
pcie_csr_rid_control_error_on_fault(const uint64 reg_value)
{
 return (pcie_csr_rid_control_error_on_fault_value)(
  (((reg_value >> 1) & 0x1ULL) << 0));
}

static inline int set_pcie_csr_rid_control_error_on_fault(
 uint64 *reg_value, pcie_csr_rid_control_error_on_fault_value value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 1)) |
         (((value >> 0) & (0x1ULL)) << 1);

 return 0;
}

static inline bool pcie_csr_rid_clear_first_fault_read_value_is_valid(int value)
{
 if (value == 0) {
  return true;
 }
 if (value == 1) {
  return true;
 }
 return false;
}

static inline const char *pcie_csr_rid_clear_first_fault_read_value_name(
 pcie_csr_rid_clear_first_fault_read_value value)
{
 if (value == 0) {
  return "NO_CLEAR";
 }
 if (value == 1) {
  return "CLEAR";
 }
 return "UNKNOWN VALUE";
}

static inline bool
pcie_csr_rid_clear_first_fault_write_value_is_valid(int value)
{
 if (value == 0) {
  return true;
 }
 if (value == 1) {
  return true;
 }
 return false;
}

static inline const char *pcie_csr_rid_clear_first_fault_write_value_name(
 pcie_csr_rid_clear_first_fault_write_value value)
{
 if (value == 0) {
  return "NO_CLEAR";
 }
 if (value == 1) {
  return "CLEAR";
 }
 return "UNKNOWN VALUE";
}

static inline pcie_csr_rid_clear_first_fault_read_value
pcie_csr_rid_clear_first_fault_read(const uint64 reg_value)
{
 return (pcie_csr_rid_clear_first_fault_read_value)(
  (((reg_value >> 0) & 0x1ULL) << 0));
}

static inline int set_pcie_csr_rid_clear_first_fault_read(
 uint64 *reg_value, pcie_csr_rid_clear_first_fault_read_value value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 0)) |
         (((value >> 0) & (0x1ULL)) << 0);

 return 0;
}

static inline pcie_csr_rid_clear_first_fault_write_value
pcie_csr_rid_clear_first_fault_write(const uint64 reg_value)
{
 return (pcie_csr_rid_clear_first_fault_write_value)(
  (((reg_value >> 1) & 0x1ULL) << 0));
}

static inline int set_pcie_csr_rid_clear_first_fault_write(
 uint64 *reg_value, pcie_csr_rid_clear_first_fault_write_value value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 1)) |
         (((value >> 0) & (0x1ULL)) << 1);

 return 0;
}

static inline bool pcie_csr_rid_fault_read_value_is_valid(int value)
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
pcie_csr_rid_fault_read_value_name(pcie_csr_rid_fault_read_value value)
{
 if (value == 0) {
  return "NO_FAULT";
 }
 if (value == 1) {
  return "READ_FAULT";
 }
 return "UNKNOWN VALUE";
}

static inline bool pcie_csr_rid_fault_write_value_is_valid(int value)
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
pcie_csr_rid_fault_write_value_name(pcie_csr_rid_fault_write_value value)
{
 if (value == 0) {
  return "NO_FAULT";
 }
 if (value == 1) {
  return "WRITE_FAULT";
 }
 return "UNKNOWN VALUE";
}

static inline pcie_csr_rid_fault_read_value
pcie_csr_rid_fault_read(const uint64 reg_value)
{
 return (pcie_csr_rid_fault_read_value)(
  (((reg_value >> 0) & 0x1ULL) << 0));
}

static inline int
set_pcie_csr_rid_fault_read(uint64 *reg_value,
       pcie_csr_rid_fault_read_value value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 0)) |
         (((value >> 0) & (0x1ULL)) << 0);

 return 0;
}

static inline pcie_csr_rid_fault_write_value
pcie_csr_rid_fault_write(const uint64 reg_value)
{
 return (pcie_csr_rid_fault_write_value)(
  (((reg_value >> 1) & 0x1ULL) << 0));
}

static inline int
set_pcie_csr_rid_fault_write(uint64 *reg_value,
        pcie_csr_rid_fault_write_value value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 1)) |
         (((value >> 0) & (0x1ULL)) << 1);

 return 0;
}

static inline uint64
pcie_csr_rid_fault_address_read_address(const uint64 reg_value)
{
 return (uint64)((((reg_value >> 0) & 0xffffffffffffffffULL) << 0));
}

static inline int set_pcie_csr_rid_fault_address_read_address(uint64 *reg_value,
             uint64 value)
{
 if (value & ~(0xffffffffffffffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffffffffffffffULL) << 0)) |
         (((value >> 0) & (0xffffffffffffffffULL)) << 0);

 return 0;
}

static inline uint64
pcie_csr_rid_fault_address_write_address(const uint64 reg_value)
{
 return (uint64)((((reg_value >> 0) & 0xffffffffffffffffULL) << 0));
}

static inline int
set_pcie_csr_rid_fault_address_write_address(uint64 *reg_value, uint64 value)
{
 if (value & ~(0xffffffffffffffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffffffffffffffULL) << 0)) |
         (((value >> 0) & (0xffffffffffffffffULL)) << 0);

 return 0;
}

static inline uint16 pcie_csr_rid_fault_rid_read_rid(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 0) & 0xffffULL) << 0));
}

static inline int set_pcie_csr_rid_fault_rid_read_rid(uint64 *reg_value,
            uint16 value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 0)) |
         (((value >> 0) & (0xffffULL)) << 0);

 return 0;
}

static inline uint16 pcie_csr_rid_fault_rid_write_rid(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 0) & 0xffffULL) << 0));
}

static inline int set_pcie_csr_rid_fault_rid_write_rid(uint64 *reg_value,
             uint16 value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 0)) |
         (((value >> 0) & (0xffffULL)) << 0);

 return 0;
}

static inline uint16 pcie_csr_rid_rid_rid(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 0) & 0xffffULL) << 0));
}

static inline int set_pcie_csr_rid_rid_rid(uint64 *reg_value, uint16 value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 0)) |
         (((value >> 0) & (0xffffULL)) << 0);

 return 0;
}

static inline uint16 pcie_csr_rid_rid_mask_rid_mask(const uint64 reg_value)
{
 return (uint16)((((reg_value >> 0) & 0xffffULL) << 0));
}

static inline int set_pcie_csr_rid_rid_mask_rid_mask(uint64 *reg_value,
           uint16 value)
{
 if (value & ~(0xffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xffffULL) << 0)) |
         (((value >> 0) & (0xffffULL)) << 0);

 return 0;
}

static inline bool pcie_csr_rid_base_addr_read_valid_value_is_valid(int value)
{
 if (value == 0) {
  return true;
 }
 if (value == 1) {
  return true;
 }
 return false;
}

static inline const char *pcie_csr_rid_base_addr_read_valid_value_name(
 pcie_csr_rid_base_addr_read_valid_value value)
{
 if (value == 0) {
  return "READS_NOT_ALLOWED";
 }
 if (value == 1) {
  return "READS_ALLOWED";
 }
 return "UNKNOWN VALUE";
}

static inline bool pcie_csr_rid_base_addr_write_valid_value_is_valid(int value)
{
 if (value == 0) {
  return true;
 }
 if (value == 1) {
  return true;
 }
 return false;
}

static inline const char *pcie_csr_rid_base_addr_write_valid_value_name(
 pcie_csr_rid_base_addr_write_valid_value value)
{
 if (value == 0) {
  return "WRITES_NOT_ALLOWED";
 }
 if (value == 1) {
  return "WRITES_ALLOWED";
 }
 return "UNKNOWN VALUE";
}

static inline pcie_csr_rid_base_addr_read_valid_value
pcie_csr_rid_base_addr_read_valid(const uint64 reg_value)
{
 return (pcie_csr_rid_base_addr_read_valid_value)(
  (((reg_value >> 0) & 0x1ULL) << 0));
}

static inline int set_pcie_csr_rid_base_addr_read_valid(
 uint64 *reg_value, pcie_csr_rid_base_addr_read_valid_value value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 0)) |
         (((value >> 0) & (0x1ULL)) << 0);

 return 0;
}

static inline pcie_csr_rid_base_addr_write_valid_value
pcie_csr_rid_base_addr_write_valid(const uint64 reg_value)
{
 return (pcie_csr_rid_base_addr_write_valid_value)(
  (((reg_value >> 1) & 0x1ULL) << 0));
}

static inline int set_pcie_csr_rid_base_addr_write_valid(
 uint64 *reg_value, pcie_csr_rid_base_addr_write_valid_value value)
{
 if (value & ~(0x1ULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0x1ULL) << 1)) |
         (((value >> 0) & (0x1ULL)) << 1);

 return 0;
}

static inline uint64 pcie_csr_rid_base_addr_base_addr(const uint64 reg_value)
{
 return (uint64)((((reg_value >> 12) & 0xfffffffffffffULL) << 0));
}

static inline int set_pcie_csr_rid_base_addr_base_addr(uint64 *reg_value,
             uint64 value)
{
 if (value & ~(0xfffffffffffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xfffffffffffffULL) << 12)) |
         (((value >> 0) & (0xfffffffffffffULL)) << 12);

 return 0;
}

static inline uint64 pcie_csr_rid_size_size(const uint64 reg_value)
{
 return (uint64)((((reg_value >> 12) & 0xfffffffffffffULL) << 0));
}

static inline int set_pcie_csr_rid_size_size(uint64 *reg_value, uint64 value)
{
 if (value & ~(0xfffffffffffffULL))
  return 1;

 (*reg_value) = ((*reg_value) & ~((0xfffffffffffffULL) << 12)) |
         (((value >> 0) & (0xfffffffffffffULL)) << 12);

 return 0;
}

#endif
