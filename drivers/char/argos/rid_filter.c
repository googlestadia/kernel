/*
 * Copyright (C) 2021 Google LLC.
 */
# 5 "./drivers/char/argos/rid_filter.c"
#if defined(CONFIG_ACCEL_P2P) || defined(CONFIG_ACCEL_P2P_MODULE)
#define BUILD_WITH_ACCEL_P2P 1
#endif

#ifdef BUILD_WITH_ACCEL_P2P
#include <linux/accel_p2p.h>
#endif
#include <linux/fs.h>
#include <linux/mman.h>
#include <linux/uaccess.h>

#include "../../gasket/gasket_logging.h"
#include "../../gasket/gasket_sysfs.h"
#include "argos_device.h"
#include "pcie_csr_rid_accessors.h"
#include "rid_filter.h"


#define BASE_ADDR_ALIGNMENT 4096

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
# 59 "./drivers/char/argos/rid_filter.c"
static ssize_t sysfs_rid_filter_status_show(
 struct argos_common_device_data *device_data, char *buf)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 const struct argos_device_desc *device_desc = device_data->device_desc;
 const int fw_bar = device_data->device_desc->firmware_register_bar;
 uint16 rid;
 uint64 address;
 ssize_t size;
 u64 value;
 pcie_csr_rid_control_enable_value control_enable;
 pcie_csr_rid_fault_read_value read_status;
 pcie_csr_rid_fault_write_value write_status;

 value = gasket_dev_read_64(gasket_dev, fw_bar,
  device_desc->rid_filter.control_location);
 control_enable = pcie_csr_rid_control_enable(value);
 size = scnprintf(buf, PAGE_SIZE,
  pcie_csr_rid_control_enable_value_name(control_enable));

 value = gasket_dev_read_64(gasket_dev, fw_bar,
  device_desc->rid_filter.fault_location);
 read_status = pcie_csr_rid_fault_read(value);
 value = gasket_dev_read_64(gasket_dev, fw_bar,
  device_desc->rid_filter.read_fault_address_location);
 address = pcie_csr_rid_fault_address_read_address(value);
 value = gasket_dev_read_64(gasket_dev, fw_bar,
  device_desc->rid_filter.read_fault_rid_location);
 rid = pcie_csr_rid_fault_rid_read_rid(value);
 size += scnprintf(buf + size, PAGE_SIZE - size,
  "read  : %s, rid=%02x:%02x.%x, address=%llu\n",
  pcie_csr_rid_fault_read_value_name(read_status),
  PCI_BUS_NUM(rid), PCI_SLOT(rid), PCI_FUNC(rid), address);

 value = gasket_dev_read_64(gasket_dev, fw_bar,
  device_desc->rid_filter.fault_location);
 write_status = pcie_csr_rid_fault_write(value);
 value = gasket_dev_read_64(gasket_dev, fw_bar,
  device_desc->rid_filter.write_fault_address_location);
 address = pcie_csr_rid_fault_address_write_address(value);
 value = gasket_dev_read_64(gasket_dev, fw_bar,
  device_desc->rid_filter.write_fault_rid_location);
 rid = pcie_csr_rid_fault_rid_write_rid(value);
 size += scnprintf(buf + size, PAGE_SIZE - size,
  "write : %s, rid=%02x:%02x.%x, address=%llu\n",
  pcie_csr_rid_fault_write_value_name(write_status),
  PCI_BUS_NUM(rid), PCI_SLOT(rid), PCI_FUNC(rid), address);

 return size;
}
# 117 "./drivers/char/argos/rid_filter.c"
static ssize_t sysfs_rid_filter_table_show(
 struct argos_common_device_data *device_data, char *buf)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 const struct argos_device_desc *device_desc = device_data->device_desc;
 const int fw_bar = device_desc->firmware_register_bar;
 int i;
 uint assignment = 0xff;
 uint16 rid, mask;
 uint64 window_base, window_size;
 ssize_t size = 0;
 int domain_nr = pci_domain_nr(device_data->gasket_dev->pci_dev->bus);
 u64 value;
 pcie_csr_rid_base_addr_read_valid_value read_valid;
 pcie_csr_rid_base_addr_write_valid_value write_valid;

 for (i = 0; size < PAGE_SIZE - 1 && i < device_desc->rid_filter.count;
      ++i) {
  if (device_data->rid_filter_assignments)
   assignment = device_data->rid_filter_assignments[i];
  value = gasket_dev_read_64(gasket_dev, fw_bar,
   device_desc->rid_filter.base_addr_location(i));
  read_valid = pcie_csr_rid_base_addr_read_valid(value);
  write_valid = pcie_csr_rid_base_addr_write_valid(value);






  window_base = value & ~(BASE_ADDR_ALIGNMENT - 1);
  value = gasket_dev_read_64(gasket_dev, fw_bar,
   device_desc->rid_filter.rid_address_location(i));
  rid = pcie_csr_rid_rid_rid(value);
  value = gasket_dev_read_64(gasket_dev, fw_bar,
   device_desc->rid_filter.rid_mask_location(i));
  mask = pcie_csr_rid_rid_mask_rid_mask(value);
  value = gasket_dev_read_64(gasket_dev, fw_bar,
   device_desc->rid_filter.size_location(i));
  window_size = pcie_csr_rid_size_size(value);

  size += scnprintf(buf + size, PAGE_SIZE - size,
   "[%2d] assignment=%#02x, rid=%04x:%02x:%02x.%x, mask_off=0000:%02x:%02x.%x, perm=%c%c-, [%llu-%llu]\n",
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
# 218 "./drivers/char/argos/rid_filter.c"
static bool rid_filter_set_enable(
 struct argos_common_device_data *device_data, bool enable)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 const struct argos_device_desc *device_desc = device_data->device_desc;
 const int fw_bar = device_desc->firmware_register_bar;
 u64 value;
 pcie_csr_rid_control_enable_value control_enable, control_enable_target;


 control_enable_target = enable ? kPcieCsrRidControlEnableValueEnable
  : kPcieCsrRidControlEnableValueDisable;
 value = gasket_dev_read_64(gasket_dev, fw_bar,
  device_desc->rid_filter.control_location);
 set_pcie_csr_rid_control_enable(&value, control_enable_target);
 gasket_dev_write_64(gasket_dev, value, fw_bar,
  device_desc->rid_filter.control_location);


 value = gasket_dev_read_64(gasket_dev, fw_bar,
  device_desc->rid_filter.control_location);
 control_enable = pcie_csr_rid_control_enable(value);

 if (control_enable != control_enable_target) {
  gasket_log_error(gasket_dev,
   "Failed to set the RID filter enable state, device is likely inaccessible (state=%s, target=%s); blindly disabling the RID filter.",
   pcie_csr_rid_control_enable_value_name(control_enable),
   pcie_csr_rid_control_enable_value_name(
    control_enable_target));


  value = gasket_dev_read_64(gasket_dev, fw_bar,
   device_desc->rid_filter.control_location);
  set_pcie_csr_rid_control_enable(
   &value, kPcieCsrRidControlEnableValueDisable);
  gasket_dev_write_64(gasket_dev, value, fw_bar,
   device_desc->rid_filter.control_location);

  return false;
 }

 return true;
}
# 281 "./drivers/char/argos/rid_filter.c"
static void rid_filter_set(
 struct argos_common_device_data *device_data, int idx, u8 queue_idx,
 uint16 rid, uint16 rid_mask, int prot, ulong base_addr, ulong size)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 const struct argos_device_desc *device_desc = device_data->device_desc;
 const int fw_bar = device_desc->firmware_register_bar;
 const ulong base_addr_location =
  device_desc->rid_filter.base_addr_location(idx);
 const ulong rid_address_location =
  device_desc->rid_filter.rid_address_location(idx);
 const ulong rid_mask_location =
  device_desc->rid_filter.rid_mask_location(idx);
 const ulong size_location =
  device_desc->rid_filter.size_location(idx);
 u8 *assignments = device_data->rid_filter_assignments;
 u64 value;

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





 value = gasket_dev_read_64(gasket_dev, fw_bar, base_addr_location);
 set_pcie_csr_rid_base_addr_read_valid(
  &value, kPcieCsrRidBaseAddrReadValidValueReadsNotAllowed);
 set_pcie_csr_rid_base_addr_write_valid(
  &value, kPcieCsrRidBaseAddrWriteValidValueWritesNotAllowed);
 gasket_dev_write_64(gasket_dev, value, fw_bar, base_addr_location);


 value = gasket_dev_read_64(gasket_dev, fw_bar, rid_address_location);
 set_pcie_csr_rid_rid_rid(&value, rid);
 gasket_dev_write_64(gasket_dev, value, fw_bar, rid_address_location);
 value = gasket_dev_read_64(gasket_dev, fw_bar, rid_mask_location);
 set_pcie_csr_rid_rid_mask_rid_mask(&value, rid_mask);
 gasket_dev_write_64(gasket_dev, value, fw_bar, rid_mask_location);







 gasket_dev_write_64(gasket_dev, base_addr, fw_bar, base_addr_location);
 value = gasket_dev_read_64(gasket_dev, fw_bar, size_location);
 set_pcie_csr_rid_size_size(&value, size);
 gasket_dev_write_64(gasket_dev, value, fw_bar, size_location);


 value = gasket_dev_read_64(gasket_dev, fw_bar, base_addr_location);
 set_pcie_csr_rid_base_addr_read_valid(
  &value, (prot & PROT_READ) ?
  kPcieCsrRidBaseAddrReadValidValueReadsAllowed :
  kPcieCsrRidBaseAddrReadValidValueReadsNotAllowed);
 set_pcie_csr_rid_base_addr_write_valid(
  &value, (prot & PROT_WRITE) ?
  kPcieCsrRidBaseAddrWriteValidValueWritesAllowed :
  kPcieCsrRidBaseAddrWriteValidValueWritesNotAllowed);
 gasket_dev_write_64(gasket_dev, value, fw_bar, base_addr_location);

 if (assignments)
  assignments[idx] = queue_idx;
}
# 373 "./drivers/char/argos/rid_filter.c"
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
# 492 "./drivers/char/argos/rid_filter.c"
static int rid_filter_allocate(struct argos_common_device_data *device_data,
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
 if ((base_addr & (BASE_ADDR_ALIGNMENT - 1)) != 0) {
  gasket_log_error(device_data->gasket_dev,
   "Invalid base_addr=%lu (not 4KiB aligned)", base_addr);
  return -EFAULT;
 }

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
# 568 "./drivers/char/argos/rid_filter.c"
static int rid_filter_deallocate(struct argos_common_device_data *device_data,
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

int rid_filter_direct_mapping_setup(struct argos_common_device_data *device_data,
                                    struct direct_mapping *direct_mapping,
                                    u8 queue_idx, ulong bar_offset)
{
 struct argos_direct_mapping_request *req = &direct_mapping->request;

#ifdef BUILD_WITH_ACCEL_P2P
 int ret;
 void *mmio_base;
 struct accel_p2p_request p2p_req = {
  .peer_rid = req->peer_rid_address,
  .peer_rid_mask = req->peer_rid_mask,
  .dram_base = req->base +
  (u64)device_data->gasket_dev->pci_dev->resource[
   req->bar].start,
  .dram_size = req->size,
  .prot = req->prot
 };
#endif


        if(req->peer_rid_address == 0 && req->peer_rid_mask == 0)
          return 0;

 direct_mapping->rid_filter_window = rid_filter_allocate(
  device_data, queue_idx, req->peer_rid_address,
  req->peer_rid_mask, req->prot, req->bar, bar_offset,
  req->size);

 if (direct_mapping->rid_filter_window < 0) {
  gasket_log_error(device_data->gasket_dev,
   "Queue %d direct mapping failed to allocate a RID filter window for peer RID %02x:%02x.%x/%02x:%02x.%x: %d",
   queue_idx,
   PCI_BUS_NUM(req->peer_rid_address),
   PCI_SLOT(req->peer_rid_address),
   PCI_FUNC(req->peer_rid_address),
   PCI_BUS_NUM(req->peer_rid_mask),
   PCI_SLOT(req->peer_rid_mask),
   PCI_FUNC(req->peer_rid_mask),
   direct_mapping->rid_filter_window);
  return direct_mapping->rid_filter_window;
 }

#ifdef BUILD_WITH_ACCEL_P2P

 ret = accel_p2p_setup(&p2p_req, &mmio_base);
 if (ret) {
  gasket_log_error(device_data->gasket_dev,
   "Queue %d accel_p2p_setup failed for peer RID %02x:%02x.%x/%02x:%02x.%x base=%llx size=%llx] %d",
   queue_idx,
   PCI_BUS_NUM(req->peer_rid_address),
   PCI_SLOT(req->peer_rid_address),
   PCI_FUNC(req->peer_rid_address),
   PCI_BUS_NUM(req->peer_rid_mask),
   PCI_SLOT(req->peer_rid_mask),
   PCI_FUNC(req->peer_rid_mask),
   req->base, req->size, ret);
  rid_filter_deallocate(device_data,
    direct_mapping->rid_filter_window,
    queue_idx);
  return ret;
 }
#endif

        return 0;
}
EXPORT_SYMBOL(rid_filter_direct_mapping_setup);

int rid_filter_direct_mapping_teardown(struct argos_common_device_data *device_data,
                                       struct direct_mapping *direct_mapping,
                                       u8 queue_idx)
{
 int ret_p2p = 0, ret_rid;
 struct argos_direct_mapping_request *req = &direct_mapping->request;

#ifdef BUILD_WITH_ACCEL_P2P

 struct accel_p2p_request p2p_req = {
  .peer_rid = req->peer_rid_address,
  .peer_rid_mask = req->peer_rid_mask,
  .dram_base = req->base +
   (u64)device_data->gasket_dev->pci_dev->resource[
    req->bar].start,
  .dram_size = req->size,
  .prot = req->prot
 };
#endif


 if (direct_mapping->rid_filter_window < 0)
  return 0;

#ifdef BUILD_WITH_ACCEL_P2P

        ret_p2p = accel_p2p_teardown(&p2p_req);
        if (ret_p2p) {
         gasket_log_error(device_data->gasket_dev,
          "Queue %d accel_p2p_teardown failed for peer RID %02x:%02x.%x/%02x:%02x.%x: %d",
          queue_idx, PCI_BUS_NUM(req->peer_rid_address),
          PCI_SLOT(req->peer_rid_address),
          PCI_FUNC(req->peer_rid_address),
          PCI_BUS_NUM(req->peer_rid_mask),
          PCI_SLOT(req->peer_rid_mask),
          PCI_FUNC(req->peer_rid_mask),
          ret_p2p);
        }
#endif

 ret_rid = rid_filter_deallocate(
  device_data, direct_mapping->rid_filter_window,
  queue_idx);
 if (ret_rid) {
  gasket_log_error(device_data->gasket_dev,
   "Queue %d direct mapping failed to deallocate RID filter window for peer RID %02x:%02x.%x/%02x:%02x.%x: %d",
   queue_idx, PCI_BUS_NUM(req->peer_rid_address),
   PCI_SLOT(req->peer_rid_address),
   PCI_FUNC(req->peer_rid_address),
   PCI_BUS_NUM(req->peer_rid_mask),
   PCI_SLOT(req->peer_rid_mask),
   PCI_FUNC(req->peer_rid_mask),
   ret_rid);
 }

 return ret_p2p ? ret_p2p : ret_rid;
}
EXPORT_SYMBOL(rid_filter_direct_mapping_teardown);
