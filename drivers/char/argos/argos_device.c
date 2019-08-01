/*
 * Copyright (C) 2019 Google LLC.
 */
# 1 "./drivers/char/argos/argos_device.c"
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/uaccess.h>

#include "../../gasket/gasket_interrupt.h"
#include "../../gasket/gasket_logging.h"
#include "../../gasket/gasket_sysfs.h"
#include "argos_device.h"
#include "argos_ioctl.h"
#include "argos_overseer.h"
#include "argos_queue.h"
#include "argos_types.h"
#include "rid_filter.h"
#include "tgid_hash.h"


#define DEFAULT_TIMEOUT_SCALING 1ul
#define FAKE_HARDWARE_TIMEOUT_SCALING 1000ul


#define DDR_CHUNK_ACK_TIMEOUT_SEC 4ul


#define DDR_CHUNK_CONFIG_TIMEOUT_SEC 2ul

int argos_wait_for_value(
 struct argos_common_device_data *device_data, int bar,
 ulong offset, ulong timeout, ulong mask, ulong expected_value)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 ulong value, deadline;

 deadline = jiffies + timeout * device_data->timeout_scaling * HZ;
 do {
  if (gasket_dev->status == GASKET_STATUS_DRIVER_EXIT) {
   gasket_log_warn(gasket_dev,
    "Aborting FW handshake due to driver exit.");
   return -ECANCELED;
  }

  value = gasket_dev_read_64(gasket_dev, bar, offset);
  if ((value & mask) == expected_value)
   return 0;
  schedule_timeout_interruptible(msecs_to_jiffies(1));
 } while (time_before(jiffies, deadline));


 value = gasket_dev_read_64(gasket_dev, bar, offset);
 if ((value & mask) == expected_value)
  return 0;

 return -ETIMEDOUT;
}
EXPORT_SYMBOL(argos_wait_for_value);

int argos_sysfs_setup_cb(struct gasket_dev *gasket_dev)
{
 struct argos_common_device_data *device_data;
 int ret;

 if (gasket_dev->cb_data == NULL)
  return -EINVAL;
 device_data = gasket_dev->cb_data;

 ret = gasket_sysfs_create_entries(
  &gasket_dev->accel_dev.dev,
  device_data->device_desc->sysfs_attrs);
 if (ret)
  return ret;

 ret = rid_filter_sysfs_setup(device_data);

 return ret;
}
EXPORT_SYMBOL(argos_sysfs_setup_cb);





int argos_device_enable_dev(struct gasket_dev *gasket_dev)
{
 struct argos_common_device_data *device_data;
 struct queue_ctx *queue_ctxs;
 int i;
 ulong fw_api_version;
 int ret;

 if (gasket_dev->cb_data == NULL) {
  gasket_log_error(gasket_dev, "Callback data is NULL!");
  return -EINVAL;
 }
 device_data = gasket_dev->cb_data;

 fw_api_version = argos_asic_read_64(
  device_data,
  &device_data->device_desc->firmware_api_version_register);
 if (fw_api_version != ARGOS_DEVICE_DRIVER_FIRMWARE_API_VERSION) {
  gasket_log_error(gasket_dev,
   "Firmware API version mismatch! Driver: %lu, firmware: %lu",
   ARGOS_DEVICE_DRIVER_FIRMWARE_API_VERSION,
   fw_api_version);
  return -EINVAL;
 }

 if (device_data->device_desc->rid_filter.count) {
  ret = rid_filter_setup(device_data);
  if (ret)
   return ret;
 } else
  device_data->rid_filter_assignments = NULL;

 queue_ctxs = device_data->queue_ctxs;
 for (i = 0; i < device_data->device_desc->queue_ctx_count; i++) {
  queue_ctxs[i].pg_tbl = gasket_dev->page_table[i];
  INIT_LIST_HEAD(&queue_ctxs[i].direct_mappings);
 }

 device_data->is_real_hardware = argos_asic_read_64(device_data,
  &device_data->device_desc->is_fake_hardware_register) == 0;
 if (device_data->is_real_hardware)
  device_data->timeout_scaling = DEFAULT_TIMEOUT_SCALING;
 else
  device_data->timeout_scaling = FAKE_HARDWARE_TIMEOUT_SCALING;

 return 0;
}
EXPORT_SYMBOL(argos_device_enable_dev);

int argos_device_disable_dev(struct gasket_dev *gasket_dev)
{
 struct argos_common_device_data *device_data;
 int ret = 0;

 if (gasket_dev->cb_data == NULL) {
  gasket_log_error(gasket_dev, "Callback data is NULL!");
  return -EINVAL;
 }
 device_data = gasket_dev->cb_data;

 ret = rid_filter_disable_and_clear(device_data);

 return ret;
}
EXPORT_SYMBOL(argos_device_disable_dev);

int argos_device_open(struct gasket_dev *gasket_dev, struct file *filp)
{
 struct argos_common_device_data *device_data;
 int ret = 0;

 if (gasket_dev->cb_data == NULL) {
  gasket_log_error(gasket_dev, "Callback data is NULL!");
  return -EINVAL;
 }
 device_data = gasket_dev->cb_data;

 mutex_lock(&device_data->mutex);
 if (tgid_hash_get_or_create(device_data, current->tgid) == NULL) {
  ret = -ENOMEM;
  goto exit;
 }





 if (gasket_dev->ownership.write_open_count == 0)
  legacy_gasket_interrupt_setup(gasket_dev);

exit:
 mutex_unlock(&device_data->mutex);
 return ret;
}
EXPORT_SYMBOL(argos_device_open);

int argos_device_release(struct gasket_dev *gasket_dev, struct file *filp)
{
 struct argos_common_device_data *device_data;
 int ret;

 if (gasket_dev->cb_data == NULL) {
  gasket_log_error(gasket_dev, "Callback data is NULL!");
  return -EINVAL;
 }
 device_data = gasket_dev->cb_data;

 mutex_lock(&device_data->mutex);
 ret = tgid_hash_put(device_data, current->tgid);
 mutex_unlock(&device_data->mutex);

 return ret;
}
EXPORT_SYMBOL(argos_device_release);






static int ioctl_set_priority_algorithm(
 struct argos_common_device_data *device_data, ulong arg)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 struct argos_priority_algorithm_config config;
 ulong value;
 ulong shift;

 gasket_log_debug(gasket_dev,
    "Recvd ioctl ARGOS_IOCTL_SET_PRIORITY_ALGORITHM");


 if (!device_data->device_desc->priority_algorithm_config.location)
  return -ENOTTY;

 if (!argos_check_ownership(gasket_dev)) {
  gasket_log_error(gasket_dev,
   "device is owned by tgid %d; tgid %d can not modify.",
   gasket_dev->ownership.owner, current->tgid);
  return -EPERM;
 }

 if (gasket_dev->status == GASKET_STATUS_DEAD) {
  gasket_log_error(gasket_dev, "The device has failed.");
  return -EIO;
 }

 if (copy_from_user(&config, (void __user *)arg, sizeof(config)))
  return -EFAULT;

 if (config.priority < 0 || config.priority > 7) {
  gasket_log_error(gasket_dev, "invalid priority specified: %d",
   config.priority);
  return -EINVAL;
 }

 if (config.algorithm < ARGOS_PRIORITY_ALGORITHM_ROUND_ROBIN ||
     config.algorithm > ARGOS_PRIORITY_ALGORITHM_WATERFALL) {
  gasket_log_error(gasket_dev, "invalid algorithm specified: %d",
   config.algorithm);
  return -EINVAL;
 }

 shift = config.priority * 8;
 value = (ulong) config.algorithm;
 gasket_read_modify_write_64(gasket_dev,
  device_data->device_desc->firmware_register_bar,
  device_data->device_desc->priority_algorithm_config.location,
  value, __builtin_popcountl(0xFF), shift);

 return 0;
}

long argos_device_ioctl(struct file *filp, uint cmd, ulong arg)
{
 struct gasket_dev *gasket_dev = filp->private_data;
 struct argos_common_device_data *device_data;
 int ret;

 if (filp->f_inode == NULL)
  return -EFAULT;

 if (gasket_dev == NULL) {
  gasket_nodev_error("Gasket dev is NULL!");
  return -EINVAL;
 }

 if (gasket_dev->cb_data == NULL) {
  gasket_log_error(gasket_dev,
   "device descriptor is missing queue context pointer!");
  return -EFAULT;
 }
 device_data = gasket_dev->cb_data;

 if (gasket_dev->parent &&
  !argos_subcontainer_argos_ioctl_has_permission(
   device_data, cmd, arg))
  return -EPERM;
 else if (device_data->mode == ARGOS_MODE_OVERSEER &&
  !argos_overseer_argos_ioctl_has_permission(
   device_data, cmd, arg))
  return -EPERM;

 ret = argos_queue_ioctl_dispatch(device_data, cmd, arg);
 if (ret != -EOPNOTSUPP)
  return ret;

 if (device_data->device_desc->overseer_supported) {
  ret = argos_overseer_ioctl_dispatch(device_data, cmd, arg);
  if (ret != -EOPNOTSUPP)
   return ret;
 }

 switch (cmd) {
 case ARGOS_IOCTL_SET_PRIORITY_ALGORITHM:
  return ioctl_set_priority_algorithm(device_data, arg);

 default:
  return -ENOTTY;
 }
}
EXPORT_SYMBOL(argos_device_ioctl);
# 313 "./drivers/char/argos/argos_device.c"
static int argos_dram_request_send_count_based(
  struct argos_common_device_data *device_data,
  struct queue_ctx *queue_ctx,
  int original_chunks, int *chunk_delta)
{
 int available_chunks;
 ulong value;
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 const struct argos_device_desc *device_desc = device_data->device_desc;


 available_chunks = argos_asic_read_64(device_data,
  &device_desc->global_ddr_state_available_chunks);
 *chunk_delta = queue_ctx->dram_chunks - original_chunks;
 if (available_chunks < *chunk_delta) {
  gasket_log_error(gasket_dev,
   "Requesting more DDR chunks than available on the device!");
  return -ENOMEM;
 }



 value = device_desc->count_based_request | queue_ctx->dram_chunks;
 gasket_dev_write_64(
  gasket_dev, value, device_desc->firmware_register_bar,
  device_desc->queue_ddr_control.get_location(queue_ctx->index));

 return 0;
}
# 352 "./drivers/char/argos/argos_device.c"
static int argos_dram_request_send_bitmap_based(
  struct argos_common_device_data *device_data,
  struct queue_ctx *queue_ctx, int original_chunks,
  uint *bitmap, int bitmap_elems, int *chunk_delta)
{
 int i;
 ulong value;
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 const struct argos_device_desc *device_desc = device_data->device_desc;






 *chunk_delta = -original_chunks;
 for (i = 0; i < bitmap_elems; i++) {
  gasket_dev_write_32(gasket_dev, bitmap[i],
        device_desc->firmware_register_bar,
        device_desc->dram_chunk_bitmap.location +
        i * sizeof(uint));
  *chunk_delta += hweight_long(bitmap[i]);
 }


 if (device_data->reserved_chunks <
     device_data->allocated_chunks + *chunk_delta) {
  gasket_log_error(gasket_dev,
   "Requesting more DDR chunks than reserved!");
  return -ENOMEM;
 }







 value = device_desc->bitmap_based_request;
 gasket_dev_write_64(
  gasket_dev, value, device_desc->firmware_register_bar,
  device_desc->queue_ddr_control.get_location(queue_ctx->index));

 return 0;
}
# 405 "./drivers/char/argos/argos_device.c"
static int get_dram_configuration_response(
 struct argos_common_device_data *device_data,
 int queue_index)
{
 int ret;
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 const struct argos_device_desc *device_desc = device_data->device_desc;

 ret = argos_wait_for_value(device_data,
   device_desc->firmware_register_bar,
   device_desc->queue_ddr_control_change_requested.
    get_location(queue_index),
   DDR_CHUNK_ACK_TIMEOUT_SEC,
   device_desc->queue_ddr_control_change_requested.mask,
   0);
 if (ret == -ECANCELED)
  return ret;
 else if (ret == -ETIMEDOUT) {
  gasket_log_error(gasket_dev,
   "HW/FW error: DDR config request not acked! Marking device unhealthy.");
  gasket_dev->status = GASKET_STATUS_DEAD;
  return -EIO;
 }


 ret = argos_wait_for_value(device_data,
   device_desc->firmware_register_bar,
   device_desc->queue_ddr_status_pending.get_location(
    queue_index),
   device_data->max_chunks_per_queue_ctx *
   DDR_CHUNK_ACK_TIMEOUT_SEC,
   device_desc->queue_ddr_status_pending.mask, 0);
 if (ret == -ECANCELED)
  return ret;
 else if (ret == -ETIMEDOUT) {
  gasket_log_error(
   gasket_dev,
   "HW/FW error: DDR config timed out! Marking device unhealthy.");
  gasket_dev->status = GASKET_STATUS_DEAD;
  return -EIO;
 }

 return 0;
}

int argos_configure_queue_ctx_dram(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx, uint *bitmap, int bitmap_elems)
{
 struct mutex *mutex;
 struct argos_common_device_data *parent_device_data;
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 const struct argos_device_desc *device_desc = device_data->device_desc;
 int ret = 0, status, pending_config, original_chunks, chunk_delta;

 if (gasket_dev->parent) {
  parent_device_data = gasket_dev->parent->cb_data;
  if (!parent_device_data) {
   gasket_log_error(gasket_dev,
      "Parent callback data is NULL!");
   return -EINVAL;
  }
  mutex = &parent_device_data->mem_alloc_mutex;
 } else
  mutex = &device_data->mem_alloc_mutex;
 mutex_lock(mutex);


 pending_config = argos_asic_read_64_indexed(device_data,
  &device_desc->queue_ddr_status_pending, queue_ctx->index);
 status = argos_asic_read_64_indexed(device_data,
  &device_desc->queue_ddr_status_value, queue_ctx->index);
 if (pending_config || status == DDR_STATUS_VALUE_IN_PROGRESS) {
  if (gasket_dev->status != GASKET_STATUS_DRIVER_EXIT) {
   gasket_log_error(gasket_dev,
    "Pending DDR config in progress. This should not be possible! Marking device unhealthy.");
   gasket_dev->status = GASKET_STATUS_DEAD;
   ret = -EIO;
   goto out;
  }
  ret = -EBUSY;
  goto out;
 }

 original_chunks = argos_asic_read_64_indexed(
  device_data, &device_desc->queue_ddr_status_current_chunks,
  queue_ctx->index);

 if (bitmap == NULL) {
  ret = argos_dram_request_send_count_based(device_data,
          queue_ctx,
          original_chunks,
          &chunk_delta);
 } else {




  ret = argos_dram_request_send_bitmap_based(
    device_data, queue_ctx,
    original_chunks, bitmap, bitmap_elems,
    &chunk_delta);
 }
 if (ret < 0)
  goto out;

 ret = get_dram_configuration_response(device_data, queue_ctx->index);
 if (ret < 0)
  goto out;


 ret = argos_dram_request_evaluate_response(gasket_dev, queue_ctx);

 if (ret == 0 || ret == -EIO)
  device_data->allocated_chunks += chunk_delta;
out:
 mutex_unlock(mutex);
 return ret;
}
EXPORT_SYMBOL(argos_configure_queue_ctx_dram);

void argos_populate_queue_mappable_region(
 struct argos_common_device_data *device_data,
 int queue_idx, struct gasket_mappable_region *mappable_region)
{
 const struct argos_device_desc *device_desc = device_data->device_desc;

 mappable_region->start =
  device_desc->mappable_regions.get_queue_start(queue_idx);
 mappable_region->length_bytes =
  device_desc->mappable_regions.get_queue_length(queue_idx);
 mappable_region->flags = VM_READ | VM_WRITE;
}
EXPORT_SYMBOL(argos_populate_queue_mappable_region);
# 547 "./drivers/char/argos/argos_device.c"
int argos_get_mappable_regions_cb(
 struct gasket_dev *gasket_dev, int bar_index,
 struct gasket_mappable_region **mappable_regions,
 int *num_mappable_regions)
{
 struct argos_common_device_data *device_data;
 const struct argos_device_desc *device_desc;
 const struct gasket_driver_desc *driver_desc;
 const struct gasket_bar_desc *bar_desc;
 const struct argos_mappable_regions *desc_map_regions;
 struct queue_ctx *queue_ctxs;
 bool return_all = false;
 int i, output_index = 0;
 int ret;

 if (gasket_dev->cb_data == NULL) {
  gasket_log_error(gasket_dev, "Callback data is NULL!");
  return -EINVAL;
 }
 device_data = gasket_dev->cb_data;
 device_desc = device_data->device_desc;
 driver_desc = device_data->gasket_driver_desc;
 desc_map_regions = &device_desc->mappable_regions;

 *mappable_regions = NULL;
 *num_mappable_regions = 0;

 queue_ctxs = device_data->queue_ctxs;
 if (queue_ctxs == NULL) {
  gasket_log_error(gasket_dev,
   "device descriptor is missing queue context pointer!");
  return -EFAULT;
 }

 if (bar_index == device_desc->firmware_register_bar) {

  *num_mappable_regions = 1;


  if ((gasket_dev->ownership.owner == current->tgid ||
       capable(CAP_SYS_ADMIN)) && !gasket_dev->parent) {
   return_all = true;

   (*num_mappable_regions)++;
  }

  for (i = 0; i < device_desc->queue_ctx_count; i++)
   if (argos_should_map_queue(device_data, i) ||
       return_all)
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

  for (i = 0; i < device_desc->queue_ctx_count; i++)




   if (argos_should_map_queue(device_data, i) ||
       return_all) {
    argos_populate_queue_mappable_region(
     device_data,
     i, &(*mappable_regions)[output_index]);
    output_index++;
    gasket_log_debug(gasket_dev,
      "Adding queue region %d", i);
   }







  (*mappable_regions)[output_index] =
   device_desc->mappable_regions.global_region;
  output_index++;

  if (return_all) {
   (*mappable_regions)[output_index] =
    device_desc->mappable_regions.master_region;
   output_index++;
  }
 } else if (bar_index == device_desc->dram_bar) {




  ret = argos_get_direct_mappable_regions(
   device_data, bar_index, mappable_regions,
   num_mappable_regions);
  if (ret || *num_mappable_regions > 0)
   return ret;



#ifndef YETI_GUEST_KERNEL

  if (!capable(CAP_SYS_ADMIN))
   return 0;
#endif

  *num_mappable_regions =
   desc_map_regions->num_mappable_dram_regions;
  *mappable_regions = kmalloc(
   sizeof(struct gasket_mappable_region) *
    *num_mappable_regions, GFP_KERNEL);
  bar_desc =
   &driver_desc->bar_descriptions[device_desc->dram_bar];
  memcpy(*mappable_regions, &bar_desc->mappable_regions,
   sizeof(struct gasket_mappable_region) *
    *num_mappable_regions);

 } else if (bar_index == device_desc->debug_bar) {

  if ((gasket_dev->ownership.owner != current->tgid &&
   !capable(CAP_SYS_ADMIN)) || gasket_dev->parent) {
   return 0;
  }

  *num_mappable_regions =
   desc_map_regions->num_mappable_debug_regions;
  *mappable_regions = kmalloc(
   sizeof(struct gasket_mappable_region) *
    *num_mappable_regions, GFP_KERNEL);
  bar_desc =
   &driver_desc->bar_descriptions[device_desc->debug_bar];
  memcpy(*mappable_regions,
   &bar_desc->mappable_regions,
   sizeof(struct gasket_mappable_region) *
    *num_mappable_regions);
 } else {
  gasket_log_error(
   gasket_dev, "Invalid BAR specified: %d", bar_index);
 }

 return 0;
}
EXPORT_SYMBOL(argos_get_mappable_regions_cb);

bool argos_owns_page_table(struct gasket_dev *gasket_dev,
      int page_table_index)
{
 struct argos_common_device_data *device_data;
 const struct argos_device_desc *device_desc;
 struct queue_ctx *queue_ctxs;

 device_data = gasket_dev->cb_data;
 if (device_data == NULL) {
  gasket_log_error(gasket_dev, "Callback data is NULL!");
  return false;
 }

 device_desc = device_data->device_desc;
 queue_ctxs = device_data->queue_ctxs;


 if (gasket_dev_is_overseer(gasket_dev))
  return false;


 if (!gasket_dev->parent)
  return true;

 if (page_table_index >= device_desc->queue_ctx_count) {
  gasket_log_error(gasket_dev,
   "Requested page table ownership for invalid index: %d",
   page_table_index);
  return false;
 }


 if (queue_ctxs[page_table_index].reserved)
  return true;

 return false;
}
EXPORT_SYMBOL(argos_owns_page_table);
