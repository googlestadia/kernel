/*
 * Copyright (C) 2019 Google LLC.
 */
# 1 "./drivers/char/argos/argos_common.c"
#include <linux/fs.h>
#include <linux/uaccess.h>

#include "../../gasket/gasket_logging.h"
#include "../../gasket/gasket_sysfs.h"
#include "argos_types.h"




enum sysfs_attribute_type {
 ATTR_IS_REAL_HARDWARE,
 ATTR_PCIE_MRRS,
 ATTR_TIMEOUT_SCALING,
};


static ssize_t sysfs_show(struct device *device, struct device_attribute *attr,
 char *buf);
static ssize_t sysfs_store(
 struct device *device, struct device_attribute *attr, const char *buf,
 size_t count);
static int valid_pcie_mrrs(int pcie_mrrs);


static const struct gasket_sysfs_attribute argos_common_sysfs_attrs[] = {
 GASKET_SYSFS_RO(is_real_hardware, sysfs_show,
  ATTR_IS_REAL_HARDWARE),
 GASKET_SYSFS_RW(pcie_mrrs, sysfs_show, sysfs_store,
  ATTR_PCIE_MRRS),
 GASKET_SYSFS_RW(timeout_scaling, sysfs_show, sysfs_store,
  ATTR_TIMEOUT_SCALING),
 GASKET_END_OF_ATTR_ARRAY
};
EXPORT_SYMBOL(argos_common_sysfs_attrs);


static ssize_t sysfs_show(
 struct device *device, struct device_attribute *attr, char *buf)
{
 struct gasket_dev *gasket_dev;
 struct gasket_sysfs_attribute *gasket_attr;
 enum sysfs_attribute_type attr_type;
 struct argos_common_device_data *device_data;

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

 attr_type = (enum sysfs_attribute_type) gasket_attr->data.attr_type;
 switch (attr_type) {
 case ATTR_IS_REAL_HARDWARE:
  return scnprintf(
   buf, PAGE_SIZE, "%u\n", device_data->is_real_hardware);
 case ATTR_PCIE_MRRS:
  return scnprintf(
   buf, PAGE_SIZE, "%u\n",
   pcie_get_readrq(gasket_dev->pci_dev));
 case ATTR_TIMEOUT_SCALING:
  return scnprintf(
   buf, PAGE_SIZE, "%u\n", device_data->timeout_scaling);
 default:
  gasket_log_error(
   gasket_dev, "Unknown attribute: %s", attr->attr.name);
  return 0;
 }
}


static int valid_pcie_mrrs(int pcie_mrrs)
{
 if (pcie_mrrs == 128 || pcie_mrrs == 256 || pcie_mrrs == 512 ||
  pcie_mrrs == 1024 || pcie_mrrs == 2048 || pcie_mrrs == 4096)
  return 1;
 return 0;
}

static ssize_t sysfs_store(
 struct device *device, struct device_attribute *attr, const char *buf,
 size_t count)
{
 int ret;
 int parse_buffer;
 struct gasket_dev *gasket_dev;
 struct argos_common_device_data *device_data;
 struct gasket_sysfs_attribute *gasket_attr;
 enum sysfs_attribute_type type;

 gasket_dev = gasket_sysfs_get_device_data(device);
 if (gasket_dev == NULL)
  return 0;

 if (gasket_dev->cb_data == NULL) {
  gasket_log_error(gasket_dev,
   "device descriptor is missing queue context pointer!");
  return 0;
 }
 device_data = gasket_dev->cb_data;

 gasket_attr = gasket_sysfs_get_attr(device, attr);
 if (gasket_attr == NULL)
  return 0;

 type = (enum sysfs_attribute_type) gasket_attr->data.attr_type;
 switch (type) {
 case ATTR_PCIE_MRRS:
  ret = kstrtoint(buf, 10, &parse_buffer);
  if (ret || !valid_pcie_mrrs(parse_buffer)) {
   gasket_log_error(gasket_dev,
    "Invalid pcie_mrrs arg: %s", buf);
   ret = 0;
  } else {
   ret = pcie_set_readrq(
    gasket_dev->pci_dev, parse_buffer);
   if (ret) {
    gasket_log_error(gasket_dev,
     "Error setting PCI MRRS");
    ret = 0;
   } else
    ret = count;
  }
  break;
 case ATTR_TIMEOUT_SCALING:
  ret = kstrtoint(buf, 10, &parse_buffer);
  if (ret) {
   gasket_log_error(gasket_dev,
    "Invalid timeout_scaling arg: %s", buf);
   ret = 0;
  } else {
   device_data->timeout_scaling = parse_buffer;
   ret = count;
  }
  break;
 default:
  gasket_log_error(
   gasket_dev, "Unknown attribute: %s", attr->attr.name);
  ret = 0;
  break;
 }

 return ret;
}
