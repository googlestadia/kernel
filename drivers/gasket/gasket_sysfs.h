/*
 * Copyright (C) 2021 Google LLC.
 */
# 13 "./drivers/gasket/gasket_sysfs.h"
#ifndef __GASKET_SYSFS_H__
#define __GASKET_SYSFS_H__ 

#include "gasket_constants.h"
#include "gasket_core.h"
#include <linux/device.h>
#include <linux/stringify.h>
#include <linux/sysfs.h>


#define GASKET_SYSFS_NUM_MAPPINGS \
 (GASKET_FRAMEWORK_DESC_MAX * GASKET_DEV_MAX)



#define GASKET_SYSFS_MAX_NODES 196


#define GASKET_ARRAY_END_TOKEN GASKET_RESERVED_ARRAY_END
#define GASKET_ARRAY_END_MARKER __stringify(GASKET_ARRAY_END_TOKEN)





#define GASKET_END_OF_ATTR_ARRAY \
 { \
  .attr = __ATTR(GASKET_ARRAY_END_TOKEN, S_IRUGO, NULL, NULL), \
  .data.attr_type = 0, \
 }





struct gasket_sysfs_attribute {

 struct device_attribute attr;


 union {
  struct bar_address_ {
   ulong bar;
   ulong offset;
  } bar_address;
  uint attr_type;
 } data;
# 68 "./drivers/gasket/gasket_sysfs.h"
 void (*write_callback)(
  struct gasket_dev *dev, struct gasket_sysfs_attribute *attr,
  ulong value);
};

#define GASKET_SYSFS_RO(_name,_show_function,_attr_type) \
 { \
  .attr = __ATTR(_name, S_IRUGO, _show_function, NULL), \
  .data.attr_type = _attr_type \
 }
#define GASKET_SYSFS_RW(_name,_show_function,_store_function,_attr_type) \
 { \
  .attr = __ATTR(_name, S_IWUSR | S_IRUGO, _show_function, \
   _store_function), \
  .data.attr_type = _attr_type \
 }


#define GASKET_SYSFS_REG(_name,_offset,_bar) \
 { \
  .attr = __ATTR(_name, S_IRUGO, gasket_sysfs_register_show, \
   NULL), \
  .data.bar_address = { \
   .bar = _bar, \
   .offset = _offset \
  } \
 }


#define GASKET_SYSFS_RW_REG(_name,_offset,_bar,_write_cb) \
 { \
  .attr = __ATTR(_name, S_IWUSR | S_IRUGO, \
   gasket_sysfs_register_show, \
   gasket_sysfs_register_store), \
  .data.bar_address = { \
   .bar = _bar, \
   .offset = _offset \
  }, \
  .write_callback = _write_cb, \
 }






void gasket_sysfs_init(void);
# 128 "./drivers/gasket/gasket_sysfs.h"
int gasket_sysfs_create_mapping(
 struct device *device, struct gasket_dev *mapping_data);
# 141 "./drivers/gasket/gasket_sysfs.h"
int gasket_sysfs_create_entries(
 struct device *device, const struct gasket_sysfs_attribute *attrs);
# 151 "./drivers/gasket/gasket_sysfs.h"
void gasket_sysfs_remove_mapping(struct device *device);
# 161 "./drivers/gasket/gasket_sysfs.h"
int gasket_sysfs_remove_entries(
 struct device *device, const struct gasket_sysfs_attribute *attrs);
# 171 "./drivers/gasket/gasket_sysfs.h"
struct gasket_dev *gasket_sysfs_get_device_data(struct device *device);
# 181 "./drivers/gasket/gasket_sysfs.h"
struct gasket_sysfs_attribute *gasket_sysfs_get_attr(
 struct device *device, struct device_attribute *attr);




ssize_t gasket_sysfs_register_show(
 struct device *device, struct device_attribute *attr, char *buf);






ssize_t gasket_sysfs_register_store(
 struct device *device, struct device_attribute *attr, const char *buf,
 size_t count);

#endif
