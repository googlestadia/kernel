/*
 * Copyright (C) 2019 Google LLC.
 */
# 1 "./drivers/char/argos/rid_filter.c"
#include <linux/fs.h>
#include <linux/mman.h>
#include <linux/uaccess.h>

#include "../../gasket/gasket_logging.h"
#include "../../gasket/gasket_sysfs.h"
#include "argos_device.h"
#include "rid_filter.h"

enum sysfs_attribute_type {
 ATTR_RID_FILTER_STATUS,
 ATTR_RID_FILTER_TABLE,
};

static ssize_t rid_filter_sysfs_show(
 struct device *device, struct device_attribute *attr, char *buf);

static const struct gasket_sysfs_attribute sysfs_attrs[] = {
 GASKET_SYSFS_RO(rid_filter_status, rid_filter_sysfs_show,
   ATTR_RID_FILTER_STATUS),
 GASKET_SYSFS_RO(rid_filter_table, rid_filter_sysfs_show,
   ATTR_RID_FILTER_TABLE),
 GASKET_END_OF_ATTR_ARRAY
};


int rid_filter_sysfs_setup(struct argos_common_device_data *device_data)
{
 if (device_data->gasket_dev->parent != NULL ||
     !device_data->device_desc->rid_filter.count)
  return 0;

 return gasket_sysfs_create_entries(
  &device_data->gasket_dev->accel_dev.dev, sysfs_attrs);
}
# 44 "./drivers/char/argos/rid_filter.c"
static ssize_t sysfs_rid_filter_status_show(
 struct argos_common_device_data *device_data, char *buf)
{
 const struct argos_device_desc *device_desc = device_data->device_desc;
 uint status, rid;
 ulong address;
 ssize_t size;

 status = argos_asic_read_64(device_data,
  &device_desc->rid_filter.control);
 size = scnprintf(buf, PAGE_SIZE, status ? "enabled\n" : "disabled\n");

 status = argos_asic_read_64(device_data,
  &device_desc->rid_filter.read_fault);
 address = argos_asic_read_64(device_data,
  &device_desc->rid_filter.read_fault_address);
 rid = argos_asic_read_64(device_data,
  &device_desc->rid_filter.read_fault_rid);
 size += scnprintf(buf + size, PAGE_SIZE - size,
  "read  : faulted=%u, rid=%02x:%02x.%x, address=%#lx\n",
  status, PCI_BUS_NUM(rid), PCI_SLOT(rid), PCI_FUNC(rid),
  address);

 status = argos_asic_read_64(device_data,
  &device_desc->rid_filter.write_fault);
 address = argos_asic_read_64(device_data,
  &device_desc->rid_filter.write_fault_address);
 rid = argos_asic_read_64(device_data,
  &device_desc->rid_filter.write_fault_rid);
 size += scnprintf(buf + size, PAGE_SIZE - size,
  "write : faulted=%u, rid=%02x:%02x.%x, address=%#lx\n",
  status, PCI_BUS_NUM(rid), PCI_SLOT(rid), PCI_FUNC(rid),
  address);

 return size;
}
# 88 "./drivers/char/argos/rid_filter.c"
static ssize_t sysfs_rid_filter_table_show(
 struct argos_common_device_data *device_data, char *buf)
{
 const struct argos_device_desc *device_desc = device_data->device_desc;
 int i;
 uint assignment = 0xff, read_valid, write_valid, rid, mask;
 ulong window_base, window_size;
 ssize_t size = 0;
 int domain_nr = pci_domain_nr(device_data->gasket_dev->pci_dev->bus);

 for (i = 0; size < PAGE_SIZE - 1 && i < device_desc->rid_filter.count;
      ++i) {
  if (device_data->rid_filter_assignments)
   assignment = device_data->rid_filter_assignments[i];
  read_valid = argos_asic_read_64_indexed(device_data,
   &device_desc->rid_filter.read_valid, i);
  write_valid = argos_asic_read_64_indexed(device_data,
   &device_desc->rid_filter.write_valid, i);
  rid = argos_asic_read_64_indexed(device_data,
   &device_desc->rid_filter.rid_address, i);
  mask = argos_asic_read_64_indexed(device_data,
   &device_desc->rid_filter.rid_mask, i);
  window_base = argos_asic_read_64_indexed(device_data,
   &device_desc->rid_filter.base_addr, i);
  window_size = argos_asic_read_64_indexed(device_data,
   &device_desc->rid_filter.size, i);

  size += scnprintf(buf + size, PAGE_SIZE - size,
   "[%2d] assignment=%#02x, rid=%04x:%02x:%02x.%x, mask_off=0000:%02x:%02x.%x, perm=%c%c-, [%#lx-%#lx]\n",
   i, assignment, domain_nr,
   PCI_BUS_NUM(rid), PCI_SLOT(rid), PCI_FUNC(rid),
   PCI_BUS_NUM(mask), PCI_SLOT(mask), PCI_FUNC(mask),
   read_valid ? 'r' : '-', write_valid ? 'w' : '-',
   window_base, window_base + window_size - 1);
 }

 return size;
}


static ssize_t rid_filter_sysfs_show(
 struct device *device, struct device_attribute *attr, char *buf)
{
 struct gasket_dev *gasket_dev;
 struct gasket_sysfs_attribute *gasket_attr;
 enum sysfs_attribute_type attr_type;
 struct argos_common_device_data *device_data;
 ssize_t ret = 0;

 gasket_dev = gasket_sysfs_get_device_data(device);
 if (gasket_dev == NULL)
  return 0;

 gasket_attr = gasket_sysfs_get_attr(device, attr);
 if (gasket_attr == NULL)
  return 0;

 if (gasket_dev->cb_data == NULL) {
  gasket_log_error(gasket_dev,
   "device descriptor is missing queue context pointer!");
  return 0;
 }
 device_data = gasket_dev->cb_data;

 attr_type = gasket_attr->data.attr_type;
 switch (attr_type) {
 case ATTR_RID_FILTER_STATUS:
  return sysfs_rid_filter_status_show(device_data, buf);
 case ATTR_RID_FILTER_TABLE:
  return sysfs_rid_filter_table_show(device_data, buf);
 default:
  gasket_log_error(
   gasket_dev, "Unknown attribute: %s", attr->attr.name);
 }

 return ret;
}
# 175 "./drivers/char/argos/rid_filter.c"
static bool rid_filter_set_enable(
 struct argos_common_device_data *device_data, bool enable)
{
 const struct argos_device_desc *device_desc = device_data->device_desc;
 int target = enable ? 1 : 0;
 int value;

 argos_asic_write_64(
  device_data, &device_desc->rid_filter.control, target);


 value = argos_asic_read_64(device_data,
       &device_desc->rid_filter.control);

 if (value != target) {
  gasket_log_error(device_data->gasket_dev,
   "Failed to set the RID filter enable state, device is likely inaccessible (state=%d, target=%d); blindly disabling the RID filter.",
   value, target);


  argos_asic_write_64(
   device_data, &device_desc->rid_filter.control, 0);

  return false;
 }

 return true;
}
# 223 "./drivers/char/argos/rid_filter.c"
static void rid_filter_set(
 struct argos_common_device_data *device_data, int idx, u8 queue_idx,
 uint rid, uint rid_mask, int prot, ulong base_addr, ulong size)
{
 const struct argos_device_desc *device_desc = device_data->device_desc;
 u8 *assignments = device_data->rid_filter_assignments;

 if (device_data->gasket_dev->parent) {
  struct argos_common_device_data *parent_device_data;

  parent_device_data = device_data->gasket_dev->parent->cb_data;
  if (parent_device_data)
   assignments =
    parent_device_data->rid_filter_assignments;
  else
   gasket_log_warn(device_data->gasket_dev,
    "Parent callback data is NULL!");
 }

 gasket_log_debug(device_data->gasket_dev,
  "[%2d] assignment=%#02x, rid=%04x:%02x:%02x.%x, mask_off=0000:%02x:%02x.%x, perm=%c%c-, [%#lx-%#lx]",
  idx, queue_idx,
  pci_domain_nr(device_data->gasket_dev->pci_dev->bus),
  PCI_BUS_NUM(rid), PCI_SLOT(rid), PCI_FUNC(rid),
  PCI_BUS_NUM(rid_mask), PCI_SLOT(rid_mask), PCI_FUNC(rid_mask),
  prot & PROT_READ ? 'r' : '-', prot & PROT_WRITE ? 'w' : '-',
  base_addr, base_addr + size - 1);





 argos_asic_write_64_indexed(device_data,
  &device_desc->rid_filter.read_valid, idx, 0);
 argos_asic_write_64_indexed(device_data,
  &device_desc->rid_filter.write_valid, idx, 0);

 argos_asic_write_64_indexed(device_data,
   &device_desc->rid_filter.rid_address, idx, rid);
 argos_asic_write_64_indexed(device_data,
   &device_desc->rid_filter.rid_mask, idx, rid_mask);
 argos_asic_write_64_indexed(device_data,
   &device_desc->rid_filter.base_addr, idx, base_addr);
 argos_asic_write_64_indexed(device_data,
   &device_desc->rid_filter.size, idx, size);

 argos_asic_write_64_indexed(device_data,
  &device_desc->rid_filter.read_valid, idx,
  (prot & PROT_READ) ? 1 : 0);
 argos_asic_write_64_indexed(device_data,
  &device_desc->rid_filter.write_valid, idx,
  (prot & PROT_WRITE) ? 1 : 0);

 if (assignments)
  assignments[idx] = queue_idx;
}
# 288 "./drivers/char/argos/rid_filter.c"
static int rid_filter_disable_and_clear_locked(
 struct argos_common_device_data *device_data)
{
 int ret = 0;
 int i;

 if (!rid_filter_set_enable(device_data, false))
  ret = -EIO;

 kfree(device_data->rid_filter_assignments);
 device_data->rid_filter_assignments = NULL;

 for (i = 0; i < device_data->device_desc->rid_filter.count; ++i)
  rid_filter_set(device_data, i, ARGOS_RID_FILTER_FREE,
   PCI_DEVID(0, 0), PCI_DEVID(0, 0), PROT_NONE, 0, 0);

 return ret;
}

int rid_filter_disable_and_clear(struct argos_common_device_data *device_data)
{
 int ret = 0;

 mutex_lock(&device_data->rid_filter_lock);

 if (!device_data->device_desc->rid_filter.count)
  goto exit;

 ret = rid_filter_disable_and_clear_locked(device_data);
 if (ret)
  goto exit;

 gasket_log_info(device_data->gasket_dev,
  "RID filter is disabled and cleared");

exit:
 mutex_unlock(&device_data->rid_filter_lock);
 return ret;
}

int rid_filter_setup(struct argos_common_device_data *device_data)
{
 int count = device_data->device_desc->rid_filter.count;
 int ret;
 struct pci_bus *bus;

 mutex_lock(&device_data->rid_filter_lock);

 ret = rid_filter_disable_and_clear_locked(device_data);
 if (ret)
  goto exit_from_disable;

 if (device_data->device_desc->queue_ctx_count >=
     ARGOS_RID_FILTER_RESERVED) {
  gasket_log_error(device_data->gasket_dev,
   "Device has too many queues (%d) for this RID filter code (>=%d)!",
   device_data->device_desc->queue_ctx_count,
   ARGOS_RID_FILTER_RESERVED);
  ret = -EIO;
  goto exit_from_disable;
 }

 device_data->rid_filter_assignments = kmalloc(count, GFP_KERNEL);
 if (!device_data->rid_filter_assignments) {
  gasket_log_error(device_data->gasket_dev,
   "Unable to allocate RID filter assignment map - out of memory");
  ret = -ENOMEM;
  goto exit;
 }
 memset(device_data->rid_filter_assignments, 0xff, count);

 rid_filter_set(device_data, 0, ARGOS_RID_FILTER_RESERVED,
  PCI_DEVID(0, 0), PCI_DEVID(0, 0xff), PROT_READ | PROT_WRITE, 0,
  0);

 bus = device_data->gasket_dev->pci_dev->bus;
 while (bus->parent)
  bus = bus->parent;
 WARN_ON(!pci_is_root_bus(bus));
 rid_filter_set(device_data, 1, 0xfe, PCI_DEVID(bus->number, 0),
         PCI_DEVID(0, 0xff), PROT_READ | PROT_WRITE, 0, 0);

 if (!rid_filter_set_enable(device_data, true)) {
  ret = -EIO;
  goto exit;
 }

 gasket_log_info(device_data->gasket_dev, "RID filter is enabled");

exit:
 if (ret)
  rid_filter_disable_and_clear_locked(device_data);
exit_from_disable:
 mutex_unlock(&device_data->rid_filter_lock);
 return ret;
}

int rid_filter_allocate(struct argos_common_device_data *device_data,
 u8 queue_idx, uint rid, uint rid_mask, int prot, int bar,
 ulong offset, ulong size)
{
 u8 *assignments;
 struct mutex *lock;
 ulong base_addr;
 int ret;
 int i;

 if (device_data->gasket_dev->parent) {
  struct argos_common_device_data *parent_device_data;

  parent_device_data = device_data->gasket_dev->parent->cb_data;
  if (!parent_device_data) {
   gasket_log_error(device_data->gasket_dev,
    "Parent callback data is NULL!");
   return -EINVAL;
  }
  assignments = parent_device_data->rid_filter_assignments;
  lock = &parent_device_data->rid_filter_lock;
 } else {
  assignments = device_data->rid_filter_assignments;
  lock = &device_data->rid_filter_lock;
 }

 if (!assignments)
  return -EOPNOTSUPP;
 if (queue_idx >= ARGOS_RID_FILTER_RESERVED) {
  gasket_log_error(device_data->gasket_dev,
   "Invoked with invalid queue_idx=%d (>=%d), but should never happen",
   queue_idx, ARGOS_RID_FILTER_RESERVED);
  return -EINVAL;
 }

 base_addr = device_data->device_desc->rid_filter.bar_base_addr[bar] +
  offset;

 mutex_lock(lock);

 for (i = 0; i < device_data->device_desc->rid_filter.count; ++i) {
  if (assignments[i] == ARGOS_RID_FILTER_FREE)
   break;
 }
 if (i == device_data->device_desc->rid_filter.count) {
  gasket_log_warn(device_data->gasket_dev,
   "No free RID filters");
  ret = -EIO;
  goto exit;
 }

 rid_filter_set(device_data, i, queue_idx, rid, rid_mask, prot,
         base_addr, size);
 ret = i;
exit:
 mutex_unlock(lock);
 return ret;
}
EXPORT_SYMBOL(rid_filter_allocate);

int rid_filter_deallocate(struct argos_common_device_data *device_data,
 int idx, u8 queue_idx)
{
 u8 *assignments;
 struct mutex *lock;

 if (device_data->gasket_dev->parent) {
  struct argos_common_device_data *parent_device_data;

  parent_device_data = device_data->gasket_dev->parent->cb_data;
  if (!parent_device_data) {
   gasket_log_error(device_data->gasket_dev,
    "Parent callback data is NULL!");
   return -EINVAL;
  }
  assignments = parent_device_data->rid_filter_assignments;
  lock = &parent_device_data->rid_filter_lock;
 } else {
  assignments = device_data->rid_filter_assignments;
  lock = &device_data->rid_filter_lock;
 }

 if (!assignments)
  return -EOPNOTSUPP;
 if (queue_idx >= ARGOS_RID_FILTER_RESERVED) {
  gasket_log_error(device_data->gasket_dev,
   "Invoked with invalid queue_idx=%d (>=%d), but should never happen",
   queue_idx, ARGOS_RID_FILTER_RESERVED);
  return -EINVAL;
 }
 if (idx < 0 || idx >= device_data->device_desc->rid_filter.count)
  return -EINVAL;

 mutex_lock(lock);

 if (assignments[idx] == queue_idx)
  rid_filter_set(device_data, idx, ARGOS_RID_FILTER_FREE,
   PCI_DEVID(0, 0), PCI_DEVID(0, 0), PROT_NONE, 0, 0);
 else
  gasket_log_warn(device_data->gasket_dev,
   "RID filter window %d not allocated to queue %d (%d)",
   idx, queue_idx, assignments[idx]);

 mutex_unlock(lock);
 return 0;
}
EXPORT_SYMBOL(rid_filter_deallocate);
