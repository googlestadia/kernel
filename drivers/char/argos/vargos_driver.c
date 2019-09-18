/*
 * Copyright (C) 2019 Google LLC.
 */
# 2 "./drivers/char/argos/vargos_driver.c"
#include <linux/google/argos.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/pci_regs.h>
#include <linux/uaccess.h>

#define CREATE_TRACE_POINTS 
#include <trace/events/vargos_driver.h>

#include "../../pci/pci.h"
#include "../../gasket/gasket_core.h"
#include "../../gasket/gasket_interrupt.h"
#include "../../gasket/gasket_logging.h"
#include "../../gasket/gasket_sysfs.h"
#include "argos_device.h"
#include "argos_ioctl.h"
#include "argos_queue.h"
#include "argos_types.h"
#include "tgid_hash.h"
#include "vargos_mailbox_types.h"


#define VARGOS_PCI_VENDOR_ID 0x1ae0
#define VARGOS_PCI_DEVICE_ID 0xffd1
#define VARGOS_PCI_SUBSYSTEM_VENDOR_ID 0x1ae0
#define VARGOS_PCI_SUBSYSTEM_DEVICE_ID 0xffd1







#define VARGOS_QUEUE_CTX_COUNT 128
#define VARGOS_DRAM_BYTES (8ull << 30)
#define VARGOS_DRAM_CHUNK_COUNT \
 (VARGOS_DRAM_BYTES / ARGOS_DRAM_CHUNK_BYTES)
#define VARGOS_MAX_DRAM_CHUNKS_PER_CTX 1024
#define VARGOS_PAGE_TABLE_MAX_ENTRIES 256
#define VARGOS_INTERRUPT_COUNT (VARGOS_QUEUE_CTX_COUNT + 1)
#define VARGOS_INTERRUPT_FAILED_CODEC VARGOS_QUEUE_CTX_COUNT

#define VARGOS_FIRMWARE_BAR 0
#define VARGOS_FIRMWARE_BAR_OFFSET 0x0
#define VARGOS_FIRMWARE_BAR_SIZE (64 << 20)

#define VARGOS_DRAM_BAR 2
#define VARGOS_DRAM_BAR_OFFSET 0x100000000ull
#define VARGOS_DRAM_BAR_SIZE (16ull << 30)

#define FAILED_CODEC_INTERRUPT_REGISTER 0x02010018
#define FAILED_CODEC_INTERRUPT_CONTROL_CONTROL 0x02020008
#define FAILED_CODEC_INTERRUPT_CONTROL_STATUS 0x02020010

#define DEFAULT_TIMEOUT_SCALING 1ul





#define VARGOS_BAR 0

#define VARGOS_MAGIC_OFFSET 0x100
#define VARGOS_MAGIC_VALUE 0x9e8e9ff0a599e249ull

#define VARGOS_INTERFACE_OFFSET 0x108
#define VARGOS_INTERFACE_MAILBOX 0x00786f626c69616dull

#define VARGOS_WORMHOLE_BASE 0x140
#define VARGOS_WORMHOLE_SIZE 0x148
#define VARGOS_WORMHOLE_BAR_BASE_0 0x150
#define VARGOS_WORMHOLE_BAR_BASE_1 0x158
#define VARGOS_WORMHOLE_BAR_BASE_2 0x160
#define VARGOS_WORMHOLE_BAR_BASE_3 0x168
#define VARGOS_WORMHOLE_BAR_BASE_4 0x170
#define VARGOS_WORMHOLE_BAR_BASE_5 0x178

#define VARGOS_EXTENDED_PAGE_TABLE_BLACKHOLE 0x1f8

#define VARGOS_MAILBOX_VERSION_OFFSET 0x200
#define VARGOS_MAILBOX_CONTROL_OFFSET 0x204
#define VARGOS_MAILBOX_COMMAND_COUNT_OFFSET 0x208
#define VARGOS_MAILBOX_STATE_OFFSET 0x20c
#define VARGOS_MAILBOX_STATUS_CODE_OFFSET 0x210
#define VARGOS_MAILBOX_COMMAND_ADDRESS_OFFSET 0x218
#define VARGOS_MAILBOX_RESPONSE_ADDRESS_OFFSET 0x220

#define VARGOS_PAGE_TABLE_BASE 0x00800000
#define VARGOS_PAGE_TABLE_SIZE 0x1000

#define VARGOS_CHIP_GLOBAL_START 0x02000000
#define VARGOS_CHIP_GLOBAL_SIZE 0x20
#define VARGOS_FAKE_STICKY_START 0x02106070
#define VARGOS_FAKE_STICKY_SIZE sizeof(u64)
# 105 "./drivers/char/argos/vargos_driver.c"
#define VARGOS_MAILBOX_TIMEOUT (msecs_to_jiffies(500))

#if VARGOS_QUEUE_CTX_COUNT > TGID_HASH_MAX_QUEUE_COUNT
#error "VArgos requires more queues than tgid_hash supports"
#endif


enum sysfs_attribute_type {
 ATTR_MAILBOX,
 ATTR_TIMEOUT_SCALING,
};


struct vargos_mailbox {
 struct gasket_dev *gasket_dev;

 struct mutex mutex;
 struct vargos_mailbox_command *commands;
 struct vargos_mailbox_response *responses;

 dma_addr_t commands_dma_addr;
 dma_addr_t responses_dma_addr;





 int last_count;
};


struct vargos_device_data {
 struct argos_common_device_data common;


 struct queue_ctx queue_ctxs[VARGOS_QUEUE_CTX_COUNT];


 struct vargos_mailbox mailbox;
};

static struct vargos_mailbox *get_mailbox(
 struct argos_common_device_data *device_data)
{
 struct vargos_device_data *vargos_device_data;

 vargos_device_data = container_of(
   device_data, struct vargos_device_data, common);

 return &vargos_device_data->mailbox;
}


static int __init vargos_init(void);
static int vargos_wormhole_setup(void);
static void vargos_exit(void);
static int vargos_alloc_device_data(struct gasket_dev *gasket_dev);
static int vargos_free_device_data(struct gasket_dev *gasket_dev);
static int vargos_add_dev(struct gasket_dev *gasket_dev);
static int vargos_remove_dev(struct gasket_dev *gasket_dev);
static int vargos_enable_dev(struct gasket_dev *gasket_dev);
static int vargos_disable_dev(struct gasket_dev *gasket_dev);
static int vargos_close(struct gasket_dev *gasket_dev);

static int vargos_get_mappable_regions_cb(
 struct gasket_dev *gasket_dev, int bar_index,
 struct gasket_mappable_region **mappable_regions,
 int *num_mappable_regions);
static enum gasket_status vargos_status(struct gasket_dev *gasket_dev);
static int vargos_map_buffer(
 struct gasket_dev *gasket_dev, int queue_idx, ulong dma_addr,
 ulong dev_addr, uint num_pages);
static int vargos_unmap_buffer(
 struct gasket_dev *gasket_dev, int queue_idx, ulong dev_addr,
 uint num_pages);


static bool vargos_is_queue_ctx_failed(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx);
static int vargos_allocate_queue_ctx(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx,
 const struct argos_subcontainer_queue_ctx_config *config);
static int vargos_enable_queue_ctx(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx);
static int vargos_disable_queue_ctx(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx,
 struct gasket_mappable_region *mappable_region);
static int vargos_deallocate_queue_ctx(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx);
static int vargos_allocate_direct_mapping(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx,
 struct direct_mapping *direct_mapping);
static int vargos_deallocate_direct_mapping(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx,
 struct direct_mapping *direct_mapping);


static unsigned long get_command_queue_start(int queue_idx);
static unsigned long get_command_queue_size(int queue_idx);
static unsigned long get_queue_control_offset(int queue_idx);
static unsigned long get_page_table_entry_offset(
 int reg_index, int entry_index);
static ssize_t sysfs_show(struct device *device, struct device_attribute *attr,
 char *buf);
static ssize_t sysfs_store(
 struct device *device, struct device_attribute *attr, const char *buf,
 size_t count);

static int mailbox_submit_and_wait_locked(
 struct vargos_mailbox *mailbox, int count);
static int mailbox_submit_and_wait_one(
 struct vargos_mailbox *mailbox,
 const struct vargos_mailbox_command *command);


static const struct gasket_sysfs_attribute vargos_sysfs_attrs[] = {
 GASKET_SYSFS_RW(mailbox, sysfs_show, sysfs_store, ATTR_MAILBOX),
 GASKET_SYSFS_RW(timeout_scaling, sysfs_show, sysfs_store,
  ATTR_TIMEOUT_SCALING),
 GASKET_END_OF_ATTR_ARRAY
};

static const struct pci_device_id pci_ids[] = {
 { PCI_DEVICE(VARGOS_PCI_VENDOR_ID, VARGOS_PCI_DEVICE_ID) }, { 0 }
};

static const struct gasket_mappable_region fw_bar_regions[] = {
 { 0x0, VARGOS_FIRMWARE_BAR_SIZE, VM_READ | VM_WRITE }
};

static const struct gasket_mappable_region dram_bar_regions[] = {
 { 0x0, VARGOS_DRAM_BAR_SIZE, VM_READ | VM_WRITE }
};

static struct gasket_page_table_config
 page_table_configs[VARGOS_QUEUE_CTX_COUNT];

static struct legacy_gasket_interrupt_desc
 interrupt_descs[VARGOS_INTERRUPT_COUNT];

static struct gasket_driver_desc driver_desc = {
 .name = "argos",
 .chip_model = "vargos",
 .chip_version = "0.0.0",
 .driver_version = ARGOS_DRIVER_VERSION,
 .legacy_support = 0,
 .module = THIS_MODULE,
 .pci_id_table = pci_ids,

 .num_page_tables = VARGOS_QUEUE_CTX_COUNT,
 .page_table_bar_index = 0,
 .page_table_configs = page_table_configs,

 .bar_descriptions = {
  { VARGOS_FIRMWARE_BAR_SIZE, (VM_READ | VM_WRITE),
   VARGOS_FIRMWARE_BAR_OFFSET, ARRAY_SIZE(fw_bar_regions),
   fw_bar_regions },
  GASKET_UNUSED_BAR,
  { VARGOS_DRAM_BAR_SIZE, (VM_READ | VM_WRITE),
   VARGOS_DRAM_BAR_OFFSET, ARRAY_SIZE(dram_bar_regions),
   dram_bar_regions },
  GASKET_UNUSED_BAR,
  GASKET_UNUSED_BAR,
  GASKET_UNUSED_BAR,
 },

 .legacy_interrupt_bar_index = VARGOS_FIRMWARE_BAR,
 .num_interrupts = VARGOS_INTERRUPT_COUNT,
 .legacy_interrupts = interrupt_descs,

 .add_dev_cb = vargos_add_dev,
 .remove_dev_cb = vargos_remove_dev,

 .enable_dev_cb = vargos_enable_dev,
 .disable_dev_cb = vargos_disable_dev,

 .sysfs_setup_cb = argos_sysfs_setup_cb,
 .sysfs_cleanup_cb = NULL,

 .device_open_cb = argos_device_open,
 .device_release_cb = argos_device_release,
 .device_close_cb = vargos_close,

 .get_mappable_regions_cb = vargos_get_mappable_regions_cb,
 .ioctl_permissions_cb = argos_check_gasket_ioctl_permissions,
 .ioctl_handler_cb = argos_device_ioctl,

 .device_status_cb = vargos_status,
 .hardware_revision_cb = NULL,
 .device_reset_cb = NULL,
};

static struct argos_device_desc vargos_device_desc = {
 .queue_ctx_count = VARGOS_QUEUE_CTX_COUNT,
 .failed_codec_interrupt = VARGOS_INTERRUPT_FAILED_CODEC,
 .overseer_supported = false,

 .sysfs_attrs = vargos_sysfs_attrs,

 .firmware_register_bar = VARGOS_FIRMWARE_BAR,
};

static struct argos_device_callbacks vargos_argos_callbacks = {
 .is_queue_ctx_failed_cb = vargos_is_queue_ctx_failed,
 .allocate_queue_ctx_cb = vargos_allocate_queue_ctx,
 .enable_queue_ctx_cb = vargos_enable_queue_ctx,
 .disable_queue_ctx_cb = vargos_disable_queue_ctx,
 .deallocate_queue_ctx_cb = vargos_deallocate_queue_ctx,
 .allocate_direct_mapping_cb = vargos_allocate_direct_mapping,
 .deallocate_direct_mapping_cb = vargos_deallocate_direct_mapping,
};


MODULE_DESCRIPTION("Google VArgos driver");
MODULE_VERSION(ARGOS_DRIVER_VERSION);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Clint Smullen <clintsmullen@google.com>");
MODULE_DEVICE_TABLE(pci, pci_ids);
module_init(vargos_init);
module_exit(vargos_exit);






static int __init vargos_init(void)
{
 int i;

 gasket_nodev_info("Loading VArgos driver: a935519476cc.");

 i = vargos_wormhole_setup();
 if (i)
  return i;

 for (i = 0; i < VARGOS_QUEUE_CTX_COUNT; i++) {
  page_table_configs[i].id = i;
  page_table_configs[i].mode = GASKET_PAGE_TABLE_MODE_EXTENDED;
  page_table_configs[i].total_entries =
   VARGOS_PAGE_TABLE_MAX_ENTRIES;
  page_table_configs[i].base_reg =
   get_page_table_entry_offset(i, 0);
  page_table_configs[i].extended_reg =
   VARGOS_EXTENDED_PAGE_TABLE_BLACKHOLE;
  page_table_configs[i].map_buffer_cb = vargos_map_buffer;
  page_table_configs[i].unmap_buffer_cb = vargos_unmap_buffer;

  interrupt_descs[i].index = i;
  interrupt_descs[i].reg = get_queue_control_offset(i);
  interrupt_descs[i].packing = UNPACKED;
 }


 interrupt_descs[VARGOS_INTERRUPT_FAILED_CODEC].index =
  VARGOS_INTERRUPT_FAILED_CODEC;
 interrupt_descs[VARGOS_INTERRUPT_FAILED_CODEC].reg =
  FAILED_CODEC_INTERRUPT_REGISTER;
 interrupt_descs[VARGOS_INTERRUPT_FAILED_CODEC].packing = UNPACKED;

 return gasket_register_device(&driver_desc);
}
# 383 "./drivers/char/argos/vargos_driver.c"
static int hacky_pci_update_resource(struct pci_dev *pdev, int bar)
{
 const int reg = PCI_BASE_ADDRESS_0 + 4 * bar;
 struct resource *res = pdev->resource + bar;
 u32 value, check;
 int ret = 0;

 value = (u32)res->start;
 pci_write_config_dword(pdev, reg, value);
 pci_read_config_dword(pdev, reg, &check);
 check &= PCI_BASE_ADDRESS_MEM_MASK;
 if (check != value) {
  dev_err(&pdev->dev, "BAR%d: error updating (%#08x != %#08x)\n",
   bar, value, check);
  ret = -EIO;
 }

 if (res->flags & IORESOURCE_MEM_64) {
  value = (u32)(res->start >> 32);
  pci_write_config_dword(pdev, reg + 4, value);
  pci_read_config_dword(pdev, reg + 4, &check);
  if (check != value) {
   dev_err(&pdev->dev,
    "BAR%d: error updating upper (%#08x != %#08x)\n",
    bar, value, check);
   ret = -EIO;
  }
 }

 return ret;
}
# 427 "./drivers/char/argos/vargos_driver.c"
int vargos_wormhole_move_bar(
 struct pci_dev *pdev, int bar, struct resource *wormhole_res,
 u64 wormhole_bar_base)
{
 struct resource *bar_res = &pdev->resource[bar];
 struct resource *r, *bus_res;
 int ret, i;

 if (!wormhole_bar_base)
  return 0;
# 445 "./drivers/char/argos/vargos_driver.c"
 ret = -EINVAL;
 pci_bus_for_each_resource(pdev->bus, r, i) {
  if (!r || !resource_contains(r, bar_res))
   continue;

  bus_res = r;

  if (resource_contains(r, wormhole_res)) {
   ret = 0;
   break;
  }

  ret = adjust_resource(
   bus_res, bus_res->start,
   wormhole_res->end - bus_res->start + 1);
  if (ret) {
   dev_err(&pdev->dev,
    "%s: Failed to resize bus resource %pR to cover wormhole region %pR (ret=%d)",
    __func__, bus_res, &wormhole_res, ret);
   break;
  }

  dev_info(&pdev->dev,
   "%s: Bus resource %pR now covers wormhole region %pR",
   __func__, bus_res, wormhole_res);
 }
 if (ret)
  return ret;





 ret = release_resource(bar_res);
 if (ret) {
  dev_err(&pdev->dev,
   "%s: Failed to release the the PCI BAR%d resource %pR (ret=%d)",
   __func__, bar, bar_res, ret);
  return ret;
 }

 ret = allocate_resource(
  bus_res, bar_res, resource_size(bar_res),
  wormhole_bar_base,
  wormhole_bar_base + resource_size(bar_res) - 1,
  resource_size(bar_res), NULL, NULL);
 if (ret) {
  dev_err(&pdev->dev,
   "%s: Failed to move BAR%d to %#llx within %pR (ret=%d), resource tree corrupted",
   __func__, bar, wormhole_bar_base, bus_res, ret);
  return ret;
 }





 ret = hacky_pci_update_resource(pdev, bar);
 if (ret) {
  dev_err(&pdev->dev,
   "%s: Failed to update BAR%d's address (ret=%d)",
   __func__, bar, ret);
  return ret;
 }

 dev_info(&pdev->dev,
  "%s: Moved BAR%d to wormhole address %#llx (%pR)",
  __func__, bar, wormhole_bar_base, bar_res);

 return 0;
}
# 524 "./drivers/char/argos/vargos_driver.c"
int vargos_wormhole_setup(void)
{
 struct pci_dev *pdev = NULL;
 int ret;
 u8 __iomem *virt_base;
 u64 wormhole_base, wormhole_size;
 u64 wormhole_bar[PCI_STD_RESOURCE_END + 1];
 struct resource wormhole_res;
 int i;

 while ((pdev = pci_get_subsys(VARGOS_PCI_VENDOR_ID,
  VARGOS_PCI_DEVICE_ID, VARGOS_PCI_SUBSYSTEM_VENDOR_ID,
  VARGOS_PCI_SUBSYSTEM_DEVICE_ID, pdev)) != NULL) {
  wormhole_base = 0;
  wormhole_size = 0;

  ret = pci_enable_device(pdev);
  if (ret) {
   dev_warn(&pdev->dev,
    "Unable to enable VArgos device: %d", ret);
   continue;
  }

  ret = pci_request_region(pdev, VARGOS_FIRMWARE_BAR,
   "vargos_wormhole_setup");
  if (ret) {
   dev_warn(&pdev->dev,
    "Unable to reserve VArgos firmware BAR region: %d",
    ret);
   continue;
  }

  virt_base = ioremap_nocache(
   pci_resource_start(pdev, VARGOS_FIRMWARE_BAR),
   VARGOS_FIRMWARE_BAR_SIZE);
  if (!virt_base) {
   dev_warn(&pdev->dev,
    "Failed to map firmware BAR (%d) of size %#x",
    VARGOS_FIRMWARE_BAR, VARGOS_FIRMWARE_BAR_SIZE);
   goto release_device;
  }


  wormhole_base = readq(virt_base + VARGOS_WORMHOLE_BASE);
  wormhole_size = readq(virt_base + VARGOS_WORMHOLE_SIZE);
  wormhole_bar[0] = readq(virt_base + VARGOS_WORMHOLE_BAR_BASE_0);
  wormhole_bar[1] = readq(virt_base + VARGOS_WORMHOLE_BAR_BASE_1);
  wormhole_bar[2] = readq(virt_base + VARGOS_WORMHOLE_BAR_BASE_2);
  wormhole_bar[3] = readq(virt_base + VARGOS_WORMHOLE_BAR_BASE_3);
  wormhole_bar[4] = readq(virt_base + VARGOS_WORMHOLE_BAR_BASE_4);
  wormhole_bar[5] = readq(virt_base + VARGOS_WORMHOLE_BAR_BASE_5);

  wormhole_res.start = wormhole_base;
  wormhole_res.end = wormhole_base + wormhole_size - 1;
  wormhole_res.flags = IORESOURCE_MEM | IORESOURCE_MEM_64;

release_device:
  if (virt_base)
   iounmap(virt_base);
  pci_release_region(pdev, VARGOS_FIRMWARE_BAR);
  pci_disable_device(pdev);

  if (wormhole_base == 0 && wormhole_size == 0) {
   dev_warn(&pdev->dev,
    "%s: Wormhole base and size are zero, disabling the feature",
    __func__);
   continue;
  }
  dev_info(&pdev->dev,
   "%s: Found wormhole region %pR", __func__,
   &wormhole_res);

  for (i = 0; i < ARRAY_SIZE(wormhole_bar); i++) {
   ret = vargos_wormhole_move_bar(
    pdev, i, &wormhole_res, wormhole_bar[i]);
   if (ret) {
    pci_dev_put(pdev);
    return ret;
   }
  }
 }

 return 0;
}





static void vargos_exit(void)
{
 gasket_nodev_info("Unloading VArgos driver.");
 gasket_unregister_device(&driver_desc);
}







static int vargos_alloc_device_data(struct gasket_dev *gasket_dev)
{
 int i;
 struct vargos_device_data *vargos_device_data;
 struct argos_common_device_data *device_data;

 vargos_device_data =
  kzalloc(sizeof(struct vargos_device_data), GFP_KERNEL);
 if (vargos_device_data == NULL) {
  gasket_log_error(gasket_dev,
   "Unable to allocate device data; out of memory");
  return -ENOMEM;
 }

 device_data = &vargos_device_data->common;
 device_data->gasket_driver_desc = &driver_desc;
 device_data->gasket_dev = gasket_dev;
 device_data->device_desc = &vargos_device_desc;
 device_data->argos_cb = &vargos_argos_callbacks;

 device_data->queue_ctxs = vargos_device_data->queue_ctxs;

 mutex_init(&device_data->mutex);
 hash_init(device_data->tgid_to_open_count);

 mutex_init(&device_data->mem_alloc_mutex);

 for (i = 0; i < VARGOS_QUEUE_CTX_COUNT; i++) {
  device_data->queue_ctxs[i].index = i;
  device_data->queue_ctxs[i].id[0] = '\0';
  mutex_init(&device_data->queue_ctxs[i].mutex);
  mutex_init(&device_data->queue_ctxs[i].direct_mappings_mutex);
 }
 device_data->mode = ARGOS_MODE_NORMAL;
 device_data->total_chunks = VARGOS_DRAM_CHUNK_COUNT;
 device_data->max_chunks_per_queue_ctx =
  VARGOS_MAX_DRAM_CHUNKS_PER_CTX;
 device_data->reserved_chunks = VARGOS_DRAM_CHUNK_COUNT;
 device_data->allocated_chunks = 0;

 gasket_dev->cb_data = device_data;

 return 0;
}







static int vargos_free_device_data(struct gasket_dev *gasket_dev)
{
 struct argos_common_device_data *device_data;
 struct vargos_device_data *vargos_device_data;

 if (gasket_dev->cb_data == NULL)
  return -EINVAL;

 device_data = gasket_dev->cb_data;
 WARN_ON(device_data->argos_cb != &vargos_argos_callbacks);

 vargos_device_data = container_of(
  device_data, struct vargos_device_data, common);
 kfree(vargos_device_data);

 return 0;
}







static int vargos_add_dev(struct gasket_dev *gasket_dev)
{
 return vargos_alloc_device_data(gasket_dev);
}





static int vargos_enable_dev(struct gasket_dev *gasket_dev)
{
 struct argos_common_device_data *device_data;
 struct vargos_mailbox *mailbox;
 struct queue_ctx *queue_ctxs;
 int i;
 int ret;
 u32 value32;
 u64 value64;

 if (gasket_dev->cb_data == NULL) {
  gasket_log_error(gasket_dev, "Callback data is NULL!");
  return -EINVAL;
 }
 device_data = gasket_dev->cb_data;
 mailbox = get_mailbox(device_data);


 value64 = gasket_dev_read_64(
  gasket_dev, VARGOS_BAR, VARGOS_MAGIC_OFFSET);
 if (value64 != VARGOS_MAGIC_VALUE) {
  gasket_log_error(gasket_dev,
   "VArgos magic value did not match! (%#llx != %#llx)",
   value64, VARGOS_MAGIC_VALUE);
  return -ENODEV;
 }

 value64 = gasket_dev_read_64(
  gasket_dev, VARGOS_BAR, VARGOS_INTERFACE_OFFSET);
 if (value64 != VARGOS_INTERFACE_MAILBOX) {
  gasket_log_error(gasket_dev,
   "VArgos interface is not mailbox, but unsupported type %#llx",
   value64);
  return -ENODEV;
 }

 value32 = gasket_dev_read_32(
  gasket_dev, VARGOS_BAR, VARGOS_MAILBOX_VERSION_OFFSET);
 if (value32 != VARGOS_MAILBOX_VERSION) {
  gasket_log_error(gasket_dev,
   "VArgos device mailbox interface is unsupported version %u (expected %u)",
   value32, VARGOS_MAILBOX_VERSION);
  return -ENODEV;
 }

 queue_ctxs = device_data->queue_ctxs;
 for (i = 0; i < device_data->device_desc->queue_ctx_count; i++) {
  queue_ctxs[i].pg_tbl = gasket_dev->page_table[i];
  INIT_LIST_HEAD(&queue_ctxs[i].direct_mappings);
 }

 device_data->timeout_scaling = DEFAULT_TIMEOUT_SCALING;


 mailbox->gasket_dev = device_data->gasket_dev;
 mutex_init(&mailbox->mutex);

 mailbox->commands = dma_alloc_coherent(
  &gasket_dev->pci_dev->dev, PAGE_SIZE,
  &mailbox->commands_dma_addr, GFP_KERNEL);
 if (!mailbox->commands) {
  gasket_log_error(gasket_dev,
   "Unable to allocate coherent mailbox command buffer; out of memory");
  ret = -ENOMEM;
  goto fail;
 }

 mailbox->responses = dma_alloc_coherent(
  &gasket_dev->pci_dev->dev, PAGE_SIZE,
  &mailbox->responses_dma_addr, GFP_KERNEL);
 if (!mailbox->responses) {
  gasket_log_error(gasket_dev,
   "Unable to allocate coherent mailbox response buffer; out of memory");
  ret = -ENOMEM;
  goto fail_responses;
 }


 gasket_dev_write_32(
  gasket_dev, VARGOS_MAILBOX_CONTROL_IDLE, VARGOS_BAR,
  VARGOS_MAILBOX_CONTROL_OFFSET);

 gasket_dev_write_64(
  gasket_dev, (u64)mailbox->commands_dma_addr, VARGOS_BAR,
  VARGOS_MAILBOX_COMMAND_ADDRESS_OFFSET);
 gasket_dev_write_64(
  gasket_dev, (u64)mailbox->responses_dma_addr, VARGOS_BAR,
  VARGOS_MAILBOX_RESPONSE_ADDRESS_OFFSET);

 return 0;

fail_responses:
 dma_free_coherent(
  &gasket_dev->pci_dev->dev, PAGE_SIZE, mailbox->commands,
  mailbox->commands_dma_addr);
fail:
 return ret;
}







static int vargos_remove_dev(struct gasket_dev *gasket_dev)
{
 vargos_free_device_data(gasket_dev);

 return 0;
}





static int vargos_disable_dev(struct gasket_dev *gasket_dev)
{
 struct argos_common_device_data *device_data;
 struct vargos_mailbox *mailbox;

 if (gasket_dev->cb_data == NULL) {
  gasket_log_error(gasket_dev, "Callback data is NULL!");
  return -EINVAL;
 }
 device_data = gasket_dev->cb_data;
 mailbox = get_mailbox(device_data);


 mutex_lock(&mailbox->mutex);

 gasket_dev_write_32(
  gasket_dev, VARGOS_MAILBOX_CONTROL_IDLE, VARGOS_BAR,
  VARGOS_MAILBOX_CONTROL_OFFSET);

 gasket_dev_write_64(gasket_dev, 0, VARGOS_BAR,
  VARGOS_MAILBOX_COMMAND_ADDRESS_OFFSET);
 gasket_dev_write_64(gasket_dev, 0, VARGOS_BAR,
  VARGOS_MAILBOX_RESPONSE_ADDRESS_OFFSET);


 dma_free_coherent(
  &gasket_dev->pci_dev->dev, PAGE_SIZE,
  mailbox->commands, mailbox->commands_dma_addr);
 dma_free_coherent(
  &gasket_dev->pci_dev->dev, PAGE_SIZE,
  mailbox->responses, mailbox->responses_dma_addr);

 mutex_unlock(&mailbox->mutex);

 return 0;
}





static int vargos_close(struct gasket_dev *gasket_dev)
{
 struct argos_common_device_data *device_data;

 if (gasket_dev->cb_data == NULL) {
  gasket_log_error(gasket_dev, "Callback data is NULL!");
  return -EINVAL;
 }
 device_data = gasket_dev->cb_data;


 gasket_dev_write_64(
  gasket_dev, 0, VARGOS_FIRMWARE_BAR,
  FAILED_CODEC_INTERRUPT_CONTROL_CONTROL);
 gasket_dev_write_64(
  gasket_dev, 0, VARGOS_FIRMWARE_BAR,
  FAILED_CODEC_INTERRUPT_CONTROL_STATUS);


 argos_disable_and_deallocate_all_queues(device_data);

 return 0;
}


static void populate_queue_mappable_region(
 int queue_idx, struct gasket_mappable_region *mappable_region)
{
 mappable_region->start = get_command_queue_start(queue_idx);
 mappable_region->length_bytes = get_command_queue_size(queue_idx);
 mappable_region->flags = VM_READ | VM_WRITE;
}

static unsigned long get_command_queue_start(int queue_idx)
{
 if (queue_idx >= VARGOS_QUEUE_CTX_COUNT)
  return 0;
 return 0x01000000 + queue_idx * 0x00010000;
}

static unsigned long get_command_queue_size(int queue_idx)
{
 if (queue_idx >= VARGOS_QUEUE_CTX_COUNT)
  return 0;
 return 0x90 + sizeof(unsigned long);
}

static ulong get_queue_control_offset(int queue_idx)
{
 if (queue_idx >= VARGOS_QUEUE_CTX_COUNT)
  return 0;




 return 0x01800000 + queue_idx * 0x00010000;
}

static unsigned long get_page_table_entry_offset(
 int reg_index, int entry_index)
{
 if (reg_index >= VARGOS_QUEUE_CTX_COUNT)
  return 0;
 if (entry_index >= VARGOS_PAGE_TABLE_SIZE / sizeof(u64))
  return 0;
 return VARGOS_PAGE_TABLE_BASE + reg_index * VARGOS_PAGE_TABLE_SIZE +
  entry_index * sizeof(u64);
}
# 942 "./drivers/char/argos/vargos_driver.c"
static int vargos_get_mappable_regions_cb(
 struct gasket_dev *gasket_dev, int bar_index,
 struct gasket_mappable_region **mappable_regions,
 int *num_mappable_regions)
{
 struct argos_common_device_data *device_data;
 struct queue_ctx *queue_ctxs;
 bool return_all = false;
 int i, output_index = 0;
 int ret;

 if (gasket_dev->cb_data == NULL) {
  gasket_log_error(gasket_dev, "Callback data is NULL!");
  return -EINVAL;
 }
 device_data = gasket_dev->cb_data;

 *mappable_regions = NULL;
 *num_mappable_regions = 0;

 queue_ctxs = device_data->queue_ctxs;
 if (queue_ctxs == NULL) {
  gasket_log_error(gasket_dev,
   "device descriptor is missing queue context pointer!");
  return -EFAULT;
 }

 if (bar_index == VARGOS_FIRMWARE_BAR) {

  *num_mappable_regions = 1;


  if ((gasket_dev->ownership.owner == current->tgid ||
       capable(CAP_SYS_ADMIN)) && !gasket_dev->parent) {
   return_all = true;

   (*num_mappable_regions)++;
  }

  for (i = 0; i < VARGOS_QUEUE_CTX_COUNT; i++)
   if (argos_should_map_queue(device_data, i) ||
       return_all)
    (*num_mappable_regions)++;

  (*num_mappable_regions)++;

  *mappable_regions = kzalloc(
   sizeof(struct gasket_mappable_region) *
   *num_mappable_regions,
   GFP_KERNEL);
  if (*mappable_regions == NULL) {
   gasket_log_error(gasket_dev,
    "Unable to alloc mappable region block!");
   *num_mappable_regions = 0;
   return -ENOMEM;
  }

  for (i = 0; i < VARGOS_QUEUE_CTX_COUNT; i++)




   if (argos_should_map_queue(device_data, i) ||
       return_all) {
    populate_queue_mappable_region(
     i, &(*mappable_regions)[output_index]);
    output_index++;
   }






  (*mappable_regions)[output_index].start =
   VARGOS_CHIP_GLOBAL_START;
  (*mappable_regions)[output_index].length_bytes =
   VARGOS_CHIP_GLOBAL_SIZE;
  (*mappable_regions)[output_index].flags = VM_READ | VM_WRITE;
  output_index++;



  (*mappable_regions)[output_index].start =
   VARGOS_FAKE_STICKY_START;
  (*mappable_regions)[output_index].length_bytes =
   VARGOS_FAKE_STICKY_SIZE;
  (*mappable_regions)[output_index].flags = VM_READ | VM_WRITE;
  output_index++;


 } else if (bar_index == VARGOS_DRAM_BAR) {




  ret = argos_get_direct_mappable_regions(
   device_data, bar_index, mappable_regions,
   num_mappable_regions);
  if (ret || *num_mappable_regions > 0)
   return ret;


  if (!capable(CAP_SYS_ADMIN))
   return 0;

  *num_mappable_regions = ARRAY_SIZE(dram_bar_regions);
  *mappable_regions = kzalloc(
   sizeof(struct gasket_mappable_region) *
   *num_mappable_regions,
   GFP_KERNEL);
  memcpy(*mappable_regions, dram_bar_regions,
   sizeof(struct gasket_mappable_region) *
   *num_mappable_regions);
 } else {
  gasket_log_error(
   gasket_dev, "Invalid BAR specified: %d", bar_index);
 }

 return 0;
}


static enum gasket_status vargos_status(struct gasket_dev *gasket_dev)
{

 return GASKET_STATUS_ALIVE;
}

static int vargos_allocate_queue_ctx(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx,
 const struct argos_subcontainer_queue_ctx_config *config)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 struct vargos_mailbox *mailbox = get_mailbox(device_data);
 struct vargos_mailbox_command command;





 if (config->use_chunk_bitmap) {
  gasket_log_warn(gasket_dev,
   "Bitmap-based allocations are not supported in a VM.");
  return -EOPNOTSUPP;
 }

 command.type = VARGOS_MAILBOX_COMMAND_ALLOCATE;
 command.virtual_queue_index = queue_ctx->index;
 command.allocate.priority = config->priority;
 command.allocate.num_chunks = config->num_chunks;

 return mailbox_submit_and_wait_one(mailbox, &command);
}

static int vargos_deallocate_queue_ctx(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx)
{
 struct vargos_mailbox *mailbox = get_mailbox(device_data);
 struct vargos_mailbox_command command;

 command.type = VARGOS_MAILBOX_COMMAND_DEALLOCATE;
 command.virtual_queue_index = queue_ctx->index;

 return mailbox_submit_and_wait_one(mailbox, &command);
}


static bool vargos_is_queue_ctx_failed(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx)
{

 return false;
}

static int vargos_enable_queue_ctx(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx)
{
 struct vargos_mailbox *mailbox = get_mailbox(device_data);
 struct vargos_mailbox_command command;

 command.type = VARGOS_MAILBOX_COMMAND_ENABLE;
 command.virtual_queue_index = queue_ctx->index;

 return mailbox_submit_and_wait_one(mailbox, &command);
}

static int vargos_disable_queue_ctx(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx,
 struct gasket_mappable_region *mappable_region)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 struct vargos_mailbox *mailbox = get_mailbox(device_data);
 struct vargos_mailbox_command command;
 int ret;

 command.type = VARGOS_MAILBOX_COMMAND_DISABLE;
 command.virtual_queue_index = queue_ctx->index;
# 1146 "./drivers/char/argos/vargos_driver.c"
 ret = mailbox_submit_and_wait_one(mailbox, &command);
 if (ret)
  gasket_log_warn(gasket_dev,
   "Queue %d disable failed with %d",
   queue_ctx->index, ret);

 return 0;
}
# 1170 "./drivers/char/argos/vargos_driver.c"
static int vargos_map_buffer(
 struct gasket_dev *gasket_dev, int queue_idx, ulong dma_addr,
 ulong dev_addr, uint num_pages)
{
 struct argos_common_device_data *device_data;
 struct vargos_mailbox *mailbox;
 struct vargos_mailbox_command command;
 int ret;

 if (gasket_dev->cb_data == NULL) {
  gasket_log_error(gasket_dev,
   "device descriptor is missing queue context pointer!");
  return -EINVAL;
 }
 device_data = gasket_dev->cb_data;
 mailbox = get_mailbox(device_data);

 command.type = VARGOS_MAILBOX_COMMAND_MAP_BUFFER;
 command.virtual_queue_index = queue_idx;
 command.map_unmap_buffer.dma_addr = dma_addr;
 command.map_unmap_buffer.dev_addr = dev_addr;
 command.map_unmap_buffer.num_bytes = PAGE_SIZE * num_pages;

 ret = mailbox_submit_and_wait_one(mailbox, &command);

 gasket_log_debug(gasket_dev,
  "Map buffer returned %d for queue_idx=%d, dma_addr=%#lx, dev_addr=%#lx, num_pages=%d",
  ret, queue_idx, dma_addr, dev_addr, num_pages);

 return ret;
}
# 1216 "./drivers/char/argos/vargos_driver.c"
static int vargos_unmap_buffer(
 struct gasket_dev *gasket_dev, int queue_idx, ulong dev_addr,
 uint num_pages)
{
 struct argos_common_device_data *device_data;
 struct vargos_mailbox *mailbox;
 struct vargos_mailbox_command command;
 int ret;

 if (gasket_dev->cb_data == NULL) {
  gasket_log_error(gasket_dev,
   "device descriptor is missing queue context pointer!");
  return -EINVAL;
 }
 device_data = gasket_dev->cb_data;
 mailbox = get_mailbox(device_data);

 command.type = VARGOS_MAILBOX_COMMAND_UNMAP_BUFFER;
 command.virtual_queue_index = queue_idx;
 command.map_unmap_buffer.dev_addr = dev_addr;
 command.map_unmap_buffer.num_bytes = PAGE_SIZE * num_pages;

 ret = mailbox_submit_and_wait_one(mailbox, &command);

 gasket_log_debug(gasket_dev,
  "Unmap buffer returned %d for queue_idx=%d, dev_addr=%#lx, num_pages=%d",
  ret, queue_idx, dev_addr, num_pages);

 return ret;
}

int vargos_allocate_direct_mapping(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx,
 struct direct_mapping *direct_mapping)
{
 struct vargos_mailbox *mailbox = get_mailbox(device_data);
 struct vargos_mailbox_command command;
 struct vargos_mailbox_response response;
 int ret;
 ulong bar_offset;

 if (direct_mapping->request.bar != VARGOS_FIRMWARE_BAR &&
     direct_mapping->request.bar != VARGOS_DRAM_BAR)
  return -EINVAL;


 command.type = VARGOS_MAILBOX_COMMAND_ALLOCATE_DIRECT_MAPPING;
 command.virtual_queue_index = queue_ctx->index;
 command.direct_mapping.bar = direct_mapping->request.bar;
 command.direct_mapping.base = direct_mapping->request.base;
 command.direct_mapping.size = direct_mapping->request.size;
 command.direct_mapping.prot = direct_mapping->request.prot;
 command.direct_mapping.peer_rid_address =
  direct_mapping->request.peer_rid_address;
 command.direct_mapping.peer_rid_mask =
  direct_mapping->request.peer_rid_mask;





 mutex_lock(&mailbox->mutex);

 memcpy(mailbox->commands, &command,
        sizeof(struct vargos_mailbox_command));
 memset(mailbox->responses, 0, sizeof(struct vargos_mailbox_response));

 ret = mailbox_submit_and_wait_locked(mailbox, 1);
 memcpy(&response, mailbox->responses,
        sizeof(struct vargos_mailbox_response));

 mutex_unlock(&mailbox->mutex);

 if (!ret)
  ret = -response.status_code;

 gasket_log_debug(device_data->gasket_dev,
  "Allocate direct mapping returned %d (bar_offset=%#llx) for queue_idx=%d, bar=%d, base=%#llx, size=%#llx, prot=%d, peer_rid_address=%04x:%02x:%02x.%x, peer_rid_mask=%04x:%02x:%02x.%x",
  ret, response.direct_mapping.bar_offset,
  command.virtual_queue_index, command.direct_mapping.bar,
  command.direct_mapping.base, command.direct_mapping.size,
  command.direct_mapping.prot,
  command.direct_mapping.peer_rid_address >> 16,
  PCI_BUS_NUM(command.direct_mapping.peer_rid_address),
  PCI_SLOT(command.direct_mapping.peer_rid_address),
  PCI_FUNC(command.direct_mapping.peer_rid_address),
  command.direct_mapping.peer_rid_mask >> 16,
  PCI_BUS_NUM(command.direct_mapping.peer_rid_mask),
  PCI_SLOT(command.direct_mapping.peer_rid_mask),
  PCI_FUNC(command.direct_mapping.peer_rid_mask));

 if (ret)
  return ret;


 bar_offset = response.direct_mapping.bar_offset;

 direct_mapping->mappable_region.start = bar_offset;
 if (direct_mapping->request.bar == VARGOS_FIRMWARE_BAR)
  direct_mapping->request.mmap_offset =
   VARGOS_FIRMWARE_BAR_OFFSET + bar_offset;
 else if (direct_mapping->request.bar == VARGOS_DRAM_BAR)
  direct_mapping->request.mmap_offset =
   VARGOS_DRAM_BAR_OFFSET + bar_offset;

 return 0;
}

int vargos_deallocate_direct_mapping(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx,
 struct direct_mapping *direct_mapping)
{
 struct vargos_mailbox *mailbox = get_mailbox(device_data);
 struct vargos_mailbox_command command;
 int ret;


 command.type = VARGOS_MAILBOX_COMMAND_DEALLOCATE_DIRECT_MAPPING;
 command.virtual_queue_index = queue_ctx->index;
 command.direct_mapping.bar = direct_mapping->request.bar;
 command.direct_mapping.base = direct_mapping->request.base;
 command.direct_mapping.size = direct_mapping->request.size;
 command.direct_mapping.prot = direct_mapping->request.prot;
 command.direct_mapping.peer_rid_address =
  direct_mapping->request.peer_rid_address;
 command.direct_mapping.peer_rid_mask =
  direct_mapping->request.peer_rid_mask;

 ret = mailbox_submit_and_wait_one(mailbox, &command);

 gasket_log_debug(device_data->gasket_dev,
  "Deallocate direct mapping returned %d for queue_idx=%d, bar=%d, base=%#llx, size=%#llx, prot=%d, peer_rid_address=%04x:%02x:%02x.%x, peer_rid_mask=%04x:%02x:%02x.%x",
  ret, command.virtual_queue_index, command.direct_mapping.bar,
  command.direct_mapping.base, command.direct_mapping.size,
  command.direct_mapping.prot,
  command.direct_mapping.peer_rid_address >> 16,
  PCI_BUS_NUM(command.direct_mapping.peer_rid_address),
  PCI_SLOT(command.direct_mapping.peer_rid_address),
  PCI_FUNC(command.direct_mapping.peer_rid_address),
  command.direct_mapping.peer_rid_mask >> 16,
  PCI_BUS_NUM(command.direct_mapping.peer_rid_mask),
  PCI_SLOT(command.direct_mapping.peer_rid_mask),
  PCI_FUNC(command.direct_mapping.peer_rid_mask));

 return ret;
}




static ssize_t sysfs_show(
 struct device *device, struct device_attribute *attr, char *buf)
{
 struct gasket_dev *gasket_dev;
 struct gasket_sysfs_attribute *gasket_attr;
 enum sysfs_attribute_type attr_type;
 struct argos_common_device_data *device_data;
 struct vargos_mailbox *mailbox;
 ssize_t size;

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
 mailbox = get_mailbox(device_data);

 attr_type = (enum sysfs_attribute_type) gasket_attr->data.attr_type;
 switch (attr_type) {
 case ATTR_MAILBOX:
  mutex_lock(&mailbox->mutex);
  size = min(mailbox->last_count *
   sizeof(struct vargos_mailbox_response),
   PAGE_SIZE - 1);
  memcpy(buf, mailbox->responses, size);
  buf[size] = 0;
  mutex_unlock(&mailbox->mutex);
  return size;

 case ATTR_TIMEOUT_SCALING:
  return scnprintf(
   buf, PAGE_SIZE, "%u\n", device_data->timeout_scaling);
 default:
  gasket_log_error(
   gasket_dev, "Unknown attribute: %s", attr->attr.name);
  return 0;
 }
}


static ssize_t sysfs_store(
 struct device *device, struct device_attribute *attr, const char *buf,
 size_t count)
{
 int ret;
 int parse_buffer;
 int command_count;
 struct gasket_dev *gasket_dev;
 struct argos_common_device_data *device_data;
 struct vargos_mailbox *mailbox;
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
 mailbox = get_mailbox(device_data);

 gasket_attr = gasket_sysfs_get_attr(device, attr);
 if (gasket_attr == NULL)
  return 0;

 type = (enum sysfs_attribute_type) gasket_attr->data.attr_type;
 switch (type) {
 case ATTR_MAILBOX:
  command_count = count / sizeof(struct vargos_mailbox_command);
  if (count !=
      command_count * sizeof(struct vargos_mailbox_command)) {
   gasket_log_error(gasket_dev,
    "mailbox write is not %lu B aligned (%lu B)",
    sizeof(struct vargos_mailbox_command), count);
   ret = -EINVAL;
   break;
  }

  mutex_lock(&mailbox->mutex);





  memcpy(mailbox->commands, buf, count);
  ret = mailbox_submit_and_wait_locked(mailbox, count);

  mutex_unlock(&mailbox->mutex);

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

static inline u32 mailbox_get_state(struct vargos_mailbox *mailbox)
{
 return gasket_dev_read_32(
  mailbox->gasket_dev, VARGOS_BAR, VARGOS_MAILBOX_STATE_OFFSET);
}

static inline u32 mailbox_get_status_code(struct vargos_mailbox *mailbox)
{
 return gasket_dev_read_32(
  mailbox->gasket_dev, VARGOS_BAR,
  VARGOS_MAILBOX_STATUS_CODE_OFFSET);
}

static inline int mailbox_wait_for_stopped(struct vargos_mailbox *mailbox)
{
 struct argos_common_device_data *device_data =
  mailbox->gasket_dev->cb_data;
 u32 state;
 ulong start, deadline;
 int ret = 0;

 start = jiffies;
 deadline = start +
  VARGOS_MAILBOX_TIMEOUT * device_data->timeout_scaling;
 do {
  state = mailbox_get_state(mailbox);
  if (state == VARGOS_MAILBOX_STATE_IDLE ||
      state == VARGOS_MAILBOX_STATE_DONE ||
      state == VARGOS_MAILBOX_STATE_ERROR) {
   ret = -mailbox_get_status_code(mailbox);
   goto exit;
  }

  schedule_timeout_interruptible(msecs_to_jiffies(1));
 } while (time_before(jiffies, deadline));


 state = mailbox_get_state(mailbox);
 if (state == VARGOS_MAILBOX_STATE_IDLE ||
     state == VARGOS_MAILBOX_STATE_DONE ||
     state == VARGOS_MAILBOX_STATE_ERROR) {
  gasket_log_warn(mailbox->gasket_dev,
   "Mailbox succeeded waiting for stop (state=%#x), but took %u ms (timeout was %u ms)",
   state, jiffies_to_msecs(jiffies - start),
   jiffies_to_msecs(deadline - start));
  ret = -mailbox_get_status_code(mailbox);
  goto exit;
 }

 gasket_log_error(mailbox->gasket_dev,
  "Mailbox timed out waiting for stop (state=%#x, timeout=%u ms)",
  state, jiffies_to_msecs(deadline - start));
 ret = -ETIMEDOUT;
exit:
 trace_vargos_driver_hypercall(
  accel_dev_name(&mailbox->gasket_dev->accel_dev),
  mailbox->commands->type, mailbox->commands->virtual_queue_index,
  jiffies - start, ret);
 return ret;
}

static inline int mailbox_write_control(
 struct vargos_mailbox *mailbox, u32 value)
{
 u32 control;
 ulong deadline;

 deadline = jiffies + VARGOS_MAILBOX_TIMEOUT;
 while (true) {
  gasket_dev_write_32(
   mailbox->gasket_dev, value, VARGOS_BAR,
   VARGOS_MAILBOX_CONTROL_OFFSET);

  control = gasket_dev_read_32(
   mailbox->gasket_dev, VARGOS_BAR,
   VARGOS_MAILBOX_CONTROL_OFFSET);
  if (control == value)
   return 0;

  if (time_after(jiffies, deadline))
   break;

  schedule_timeout_interruptible(msecs_to_jiffies(1));
 }

 gasket_log_error(mailbox->gasket_dev,
  "Mailbox control write timed out (target=%#x, actual=%#x)",
  value, control);
 return -ETIMEDOUT;
}


static int mailbox_submit_and_wait_locked(
 struct vargos_mailbox *mailbox, int count)
{
 int ret;


 ret = mailbox_write_control(
  mailbox, VARGOS_MAILBOX_CONTROL_IDLE);
 if (ret)
  return ret;

 ret = mailbox_wait_for_stopped(mailbox);
 if (ret)
  return ret;

 gasket_dev_write_32(
  mailbox->gasket_dev, count, VARGOS_BAR,
  VARGOS_MAILBOX_COMMAND_COUNT_OFFSET);
 mailbox->last_count = count;

 mailbox_write_control(mailbox, VARGOS_MAILBOX_CONTROL_START);

 return mailbox_wait_for_stopped(mailbox);
}


static int mailbox_submit_and_wait_one(
 struct vargos_mailbox *mailbox,
 const struct vargos_mailbox_command *command)
{
 int ret;

 mutex_lock(&mailbox->mutex);

 memcpy(mailbox->commands, command,
        sizeof(struct vargos_mailbox_command));
 memset(mailbox->responses, 0, sizeof(struct vargos_mailbox_response));

 ret = mailbox_submit_and_wait_locked(mailbox, 1);
 if (!ret)
  ret = -mailbox->responses[0].status_code;

 mutex_unlock(&mailbox->mutex);

 return ret;
}
