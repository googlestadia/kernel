/*
 * Copyright (C) 2019 Google LLC.
 */
# 7 "./drivers/gasket/gasket_core.h"
#ifndef __GASKET_CORE_H__
#define __GASKET_CORE_H__ 

#include <linux/accel.h>
#include <linux/cdev.h>
#include <linux/compiler.h>
#include <linux/device.h>
#include <linux/google/gasket.h>
#include <linux/hashtable.h>
#include <linux/init.h>
#include <linux/mm_types.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/sched.h>
#include <linux/types.h>

#include "gasket_constants.h"

struct gasket_dev;
# 35 "./drivers/gasket/gasket_core.h"
struct gasket_num_name {
 uint snn_num;
 const char *snn_name;
};
# 50 "./drivers/gasket/gasket_core.h"
enum legacy_gasket_interrupt_packing {
 PACK_0 = 0,
 PACK_1 = 1,
 PACK_2 = 2,
 PACK_3 = 3,
 UNPACKED = 4,
};





struct legacy_gasket_interrupt_desc {

 int index;

 u64 reg;

 int packing;
};


struct gasket_interrupt_desc {

 int bar_index;

 u64 reg;
};


struct gasket_bar_data {

 u8 __iomem *virt_base;


 ulong phys_base;


 ulong length_bytes;
};


struct gasket_ownership {

 int is_owned;


 pid_t owner;


 int write_open_count;
};


enum gasket_page_table_mode {

 GASKET_PAGE_TABLE_MODE_NORMAL,


 GASKET_PAGE_TABLE_MODE_SIMPLE,


 GASKET_PAGE_TABLE_MODE_EXTENDED,
};






struct gasket_page_table_config {

 int id;


 enum gasket_page_table_mode mode;


 ulong total_entries;


 int bar_index;


 int base_reg;






 int extended_reg;


 int extended_bit;
# 161 "./drivers/gasket/gasket_core.h"
 int (*map_buffer_cb)(struct gasket_dev *dev, int page_table_id,
  ulong dma_addr, ulong dev_addr, uint num_pages);
# 178 "./drivers/gasket/gasket_core.h"
 int (*unmap_buffer_cb)(struct gasket_dev *dev, int page_table_id,
  ulong dev_addr, uint num_pages);
# 189 "./drivers/gasket/gasket_core.h"
 bool (*owns_page_table_cb)(
  struct gasket_dev *gasket_dev, int page_table_id);
};
# 200 "./drivers/gasket/gasket_core.h"
struct gasket_mappable_region {
 u64 start;
 u64 length_bytes;
 vm_flags_t flags;
};


struct gasket_bar_desc {
# 217 "./drivers/gasket/gasket_core.h"
 u64 size;




 ulong permissions;

 u64 base;

 int num_mappable_regions;

 const struct gasket_mappable_region *mappable_regions;
};


enum gasket_mapping_options { GASKET_NOMAP = 0 };


#define GASKET_UNUSED_BAR \
 { \
  0, GASKET_NOMAP, 0, 0, NULL \
 }
# 251 "./drivers/gasket/gasket_core.h"
enum gasket_iommu_mappings {

 GASKET_IOMMU_ANY = 0,

 GASKET_IOMMU_PREFER,
};


struct gasket_internal_desc;







struct gasket_dev {

 struct accel_dev accel_dev;


 bool accel_registered;


 struct gasket_internal_desc *internal_desc;


 struct pci_dev *pci_dev;


 int dev_idx;


 char kobj_name[GASKET_NAME_MAX];


 struct gasket_bar_data bar_data[GASKET_NUM_BARS];


 int num_page_tables;


 struct gasket_page_table *page_table[GASKET_MAX_NUM_PAGE_TABLES];


 struct gasket_interrupt_data *interrupt_data;


 enum gasket_status status;


 uint reset_count;


 char legacy_cdev_name[GASKET_NAME_MAX];


 dev_t legacy_devt;


 struct device *legacy_device;


 struct cdev legacy_cdev;


 int legacy_cdev_added;


 struct gasket_ownership ownership;


 int hardware_revision;





 void *cb_data;


 struct gasket_dev *parent;


 int clone_index;


 struct gasket_dev *clones[GASKET_MAX_CLONES];


 struct mutex mutex;


 struct hlist_node hlist_node;
 struct hlist_node legacy_hlist_node;
};


typedef int (*gasket_ioctl_permissions_cb_t)(
 struct file *filp, uint cmd, ulong arg);
# 361 "./drivers/gasket/gasket_core.h"
struct gasket_driver_desc {

 const char *name;


 const char *chip_model;


 const char *chip_version;


 const char *driver_version;





 int legacy_support;


 int legacy_major, legacy_minor;


 struct module *module;


 const struct pci_device_id *pci_id_table;


 int num_page_tables;


 int page_table_bar_index;


 const struct gasket_page_table_config *page_table_configs;






 ulong legacy_mmap_address_offset;


 struct gasket_bar_desc bar_descriptions[GASKET_NUM_BARS];






 int legacy_interrupt_bar_index;





 int num_msix_interrupts;


 int num_interrupts;


 const struct gasket_interrupt_desc *interrupts;




 const struct legacy_gasket_interrupt_desc *legacy_interrupts;
# 441 "./drivers/gasket/gasket_core.h"
 int legacy_interrupt_pack_width;






 enum gasket_iommu_mappings iommu_mappings;
# 461 "./drivers/gasket/gasket_core.h"
 int (*add_dev_cb)(struct gasket_dev *dev);
# 472 "./drivers/gasket/gasket_core.h"
 int (*remove_dev_cb)(struct gasket_dev *dev);
# 482 "./drivers/gasket/gasket_core.h"
 int (*device_open_cb)(struct gasket_dev *dev, struct file *file);
# 493 "./drivers/gasket/gasket_core.h"
 int (*device_release_cb)(
  struct gasket_dev *gasket_dev, struct file *file);
# 507 "./drivers/gasket/gasket_core.h"
 int (*device_close_cb)(struct gasket_dev *dev);
# 521 "./drivers/gasket/gasket_core.h"
 int (*enable_dev_cb)(struct gasket_dev *dev);
# 530 "./drivers/gasket/gasket_core.h"
 int (*disable_dev_cb)(struct gasket_dev *dev);
# 539 "./drivers/gasket/gasket_core.h"
 int (*sysfs_setup_cb)(struct gasket_dev *dev);
# 548 "./drivers/gasket/gasket_core.h"
 int (*sysfs_cleanup_cb)(struct gasket_dev *dev);
# 563 "./drivers/gasket/gasket_core.h"
 int (*get_mappable_regions_cb)(
  struct gasket_dev *gasket_dev, int bar_index,
  struct gasket_mappable_region **mappable_regions,
  int *num_mappable_regions);
# 579 "./drivers/gasket/gasket_core.h"
 gasket_ioctl_permissions_cb_t ioctl_permissions_cb;
# 592 "./drivers/gasket/gasket_core.h"
 long (*ioctl_handler_cb)(struct file *filp, uint cmd, ulong arg);
# 602 "./drivers/gasket/gasket_core.h"
 enum gasket_status (*device_status_cb)(struct gasket_dev *dev);
# 611 "./drivers/gasket/gasket_core.h"
 int (*hardware_revision_cb)(struct gasket_dev *dev);
# 626 "./drivers/gasket/gasket_core.h"
 int (*device_reset_cb)(struct gasket_dev *dev, uint reset_type);

};
# 638 "./drivers/gasket/gasket_core.h"
int gasket_register_device(const struct gasket_driver_desc *desc);
# 647 "./drivers/gasket/gasket_core.h"
void gasket_unregister_device(const struct gasket_driver_desc *desc);
# 667 "./drivers/gasket/gasket_core.h"
int gasket_clone_create(struct gasket_dev *parent, struct gasket_dev *clone);





int gasket_clone_cleanup(struct gasket_dev *clone);
# 685 "./drivers/gasket/gasket_core.h"
int gasket_reset(struct gasket_dev *gasket_dev, uint reset_type);
int gasket_reset_nolock(struct gasket_dev *gasket_dev, uint reset_type);


bool gasket_dev_is_overseer(struct gasket_dev *gasket_dev);







int gasket_mm_unmap_region(
 const struct gasket_dev *gasket_dev, struct vm_area_struct *vma,
 int map_bar_index,
 const struct gasket_mappable_region *map_region);





gasket_ioctl_permissions_cb_t gasket_get_ioctl_permissions_cb(
 struct gasket_dev *gasket_dev);







const char *gasket_num_name_lookup(
 uint num, const struct gasket_num_name *table);






void gasket_mapped_unforkable_page(struct gasket_dev *gasket_dev);


static inline u64 gasket_dev_read_64(
 struct gasket_dev *gasket_dev, int bar, ulong location)
{
 return readq(&gasket_dev->bar_data[bar].virt_base[location]);
}

static inline void gasket_dev_write_64(
 struct gasket_dev *gasket_dev, u64 value, int bar, ulong location)
{
 writeq(value, &gasket_dev->bar_data[bar].virt_base[location]);
}

static inline u32 gasket_dev_read_32(
 struct gasket_dev *gasket_dev, int bar, u32 location)
{
 return readl(&gasket_dev->bar_data[bar].virt_base[location]);
}

static inline void gasket_dev_write_32(
 struct gasket_dev *gasket_dev, u32 value, int bar, ulong location)
{
 writel(value, &gasket_dev->bar_data[bar].virt_base[location]);
}

static inline void gasket_read_modify_write_64(
 struct gasket_dev *dev, int bar, ulong location,
 u64 value, u64 mask_width, u64 mask_shift)
{
 u64 mask, tmp;

 tmp = gasket_dev_read_64(dev, bar, location);
 mask = ((1 << mask_width) - 1) << mask_shift;
 tmp = (tmp & ~mask) | (value << mask_shift);
 gasket_dev_write_64(dev, tmp, bar, location);
}


bool gasket_pci_is_iommu_enabled(struct pci_dev *pdev);

#endif
