/*
 * Copyright (C) 2021 Google LLC.
 */
# 2 "./drivers/char/argos/argos_device.h"
#ifndef __ARGOS_DEVICE_H__
#define __ARGOS_DEVICE_H__ 

#include "../../gasket/gasket_core.h"
#include "argos_types.h"


#define ARGOS_DRIVER_VERSION "0.13"


#define ARGOS_DEVICE_DRIVER_FIRMWARE_API_VERSION 1lu


#define ARGOS_NAME_OFFSET_WRAPPER(chip,register_name,field_name) \
 get_##chip##_##register_name##_##field_name##_offset
# 33 "./drivers/char/argos/argos_device.h"
#define ARGOS_OFFSET_WRAPPER(chip,register_name,field_name) \
static ulong \
ARGOS_NAME_OFFSET_WRAPPER(chip, register_name, field_name)(int queue_idx) \
{ \
 const struct chip##_##register_name##_offsets *offset = \
  chip##_##register_name##_offsets_get(queue_idx); \
 if (offset) \
  return offset->field_name; \
 return 0; \
}


#define ARGOS_NAME_FIELD_DECODER_WRAPPER(func_name) get_ ##func_name





#define ARGOS_FIELD_DECODER_WRAPPER(func_name) \
static ulong \
ARGOS_NAME_FIELD_DECODER_WRAPPER(func_name)(const uint64 reg_value) \
{ \
 return func_name(reg_value); \
}
# 70 "./drivers/char/argos/argos_device.h"
int argos_wait_for_expected_value(
 struct argos_common_device_data *device_data, int bar, ulong offset,
 ulong timeout, ulong (*get_decoded_value)(uint64),
 ulong expected_value);







static inline bool argos_check_ownership(struct gasket_dev *gasket_dev)
{
 if (gasket_dev->ownership.is_owned &&
  gasket_dev->ownership.owner == current->tgid) {
  return true;
 }

 return false;
}
# 98 "./drivers/char/argos/argos_device.h"
int argos_sysfs_setup_cb(struct gasket_dev *gasket_dev);





int argos_device_enable_dev(struct gasket_dev *gasket_dev);







void argos_initialize_gasket_mappable_region(
 struct gasket_mappable_region *region, uint64 start, uint64 end);





int argos_device_disable_dev(struct gasket_dev *gasket_dev);






int argos_device_open(struct gasket_dev *gasket_dev, struct file *filp);
# 137 "./drivers/char/argos/argos_device.h"
int argos_device_release(struct gasket_dev *gasket_dev, struct file *filp);





int argos_device_close(struct gasket_dev *gasket_dev);
# 154 "./drivers/char/argos/argos_device.h"
long argos_device_ioctl(struct file *filp, uint cmd, ulong arg);
# 166 "./drivers/char/argos/argos_device.h"
int argos_device_reset(struct gasket_dev *gasket_dev, uint reset_type);
# 179 "./drivers/char/argos/argos_device.h"
int argos_configure_queue_ctx_dram(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx, uint *bitmap, int bitmap_elems);







void argos_populate_queue_mappable_region(
 struct argos_common_device_data *device_data,
 int queue_idx, struct gasket_mappable_region *mappable_region);
# 200 "./drivers/char/argos/argos_device.h"
int argos_get_mappable_regions_cb(
 struct gasket_dev *gasket_dev, int bar_index,
 struct gasket_mappable_region **mappable_regions,
 int *num_mappable_regions);






bool argos_owns_page_table(struct gasket_dev *gasket_dev, int page_table_id);
# 221 "./drivers/char/argos/argos_device.h"
int argos_device_firmware_version_cb(
 struct gasket_dev *gasket_dev, unsigned int *major, unsigned int *minor,
 unsigned int *point, unsigned int *subpoint);

#endif
