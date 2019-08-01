/*
 * Copyright (C) 2019 Google LLC.
 */
# 2 "./drivers/char/argos/argos_device.h"
#ifndef __ARGOS_DEVICE_H__
#define __ARGOS_DEVICE_H__ 

#include "../../gasket/gasket_core.h"
#include "argos_types.h"


#define ARGOS_DRIVER_VERSION "0.13"


#define ARGOS_DEVICE_DRIVER_FIRMWARE_API_VERSION 1lu





#define ARGOS_REGISTER_FIELD_DESC(register_name,field_name) \
 { \
  .location = (register_name), \
  .shift = (register_name##_##field_name##_SHIFT), \
  .mask = (register_name##_##field_name##_MASK), \
 }
# 38 "./drivers/char/argos/argos_device.h"
int argos_wait_for_value(
 struct argos_common_device_data *device_data, int bar, ulong offset,
 ulong timeout, ulong mask, ulong expected_value);







static inline bool argos_check_ownership(struct gasket_dev *gasket_dev)
{
 if (gasket_dev->ownership.is_owned &&
     gasket_dev->ownership.owner == current->tgid) {
  return true;
 }

 return false;
}
# 65 "./drivers/char/argos/argos_device.h"
int argos_sysfs_setup_cb(struct gasket_dev *gasket_dev);





int argos_device_enable_dev(struct gasket_dev *gasket_dev);





int argos_device_disable_dev(struct gasket_dev *gasket_dev);






int argos_device_open(struct gasket_dev *gasket_dev, struct file *filp);
# 95 "./drivers/char/argos/argos_device.h"
int argos_device_release(struct gasket_dev *gasket_dev, struct file *filp);
# 106 "./drivers/char/argos/argos_device.h"
long argos_device_ioctl(struct file *filp, uint cmd, ulong arg);
# 119 "./drivers/char/argos/argos_device.h"
int argos_configure_queue_ctx_dram(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx, uint *bitmap, int bitmap_elems);
# 130 "./drivers/char/argos/argos_device.h"
void argos_populate_queue_mappable_region(
 struct argos_common_device_data *device_data,
 int queue_idx, struct gasket_mappable_region *mappable_region);
# 141 "./drivers/char/argos/argos_device.h"
int argos_get_mappable_regions_cb(
 struct gasket_dev *gasket_dev, int bar_index,
 struct gasket_mappable_region **mappable_regions,
 int *num_mappable_regions);






bool argos_owns_page_table(struct gasket_dev *gasket_dev, int page_table_id);





static inline u64 argos_asic_read_64(
 const struct argos_common_device_data *device_data,
 const struct argos_register_field_desc *register_desc)
{
 u64 value;

 value = gasket_dev_read_64(
  device_data->gasket_dev,
  device_data->device_desc->firmware_register_bar,
  register_desc->location);
 if (register_desc->mask)
  return (value & register_desc->mask) >> register_desc->shift;
 else
  return value;
}
# 180 "./drivers/char/argos/argos_device.h"
static inline u64 argos_asic_read_64_indexed(
 const struct argos_common_device_data *device_data,
 const struct argos_register_field_desc *register_desc,
 int index)
{
 struct argos_register_field_desc indexed_desc = *register_desc;

 if (register_desc->get_location)
  indexed_desc.location = register_desc->get_location(index);
 else
  WARN_ON(index != 0);

 return argos_asic_read_64(device_data, &indexed_desc);
}





static inline void argos_asic_write_64(
 const struct argos_common_device_data *device_data,
 const struct argos_register_field_desc *register_desc, u64 value)
{




 if (register_desc->mask) {
  u64 tmp;

  tmp = gasket_dev_read_64(device_data->gasket_dev,
   device_data->device_desc->firmware_register_bar,
   register_desc->location);
  value = (tmp & ~register_desc->mask) |
   ((value << register_desc->shift) & register_desc->mask);
 }

 gasket_dev_write_64(
  device_data->gasket_dev, value,
  device_data->device_desc->firmware_register_bar,
  register_desc->location);
}
# 230 "./drivers/char/argos/argos_device.h"
static inline void argos_asic_write_64_indexed(
 const struct argos_common_device_data *device_data,
 const struct argos_register_field_desc *register_desc,
 int index, u64 value)
{
 struct argos_register_field_desc indexed_desc = *register_desc;

 if (register_desc->get_location)
  indexed_desc.location = register_desc->get_location(index);
 else
  WARN_ON(index != 0);

 argos_asic_write_64(device_data, &indexed_desc, value);
}

#endif
