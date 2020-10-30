/*
 * Copyright (C) 2020 Google LLC.
 */
# 1 "./drivers/char/argos/argos_device.c"
#include <linux/fs.h>
#include <linux/google/argos.h>
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
#include "chip_global_accessors.h"
#include "firmware_version_accessors.h"
#include "kernel_chip_global_accessors.h"
#include "kernel_queue_accessors.h"
#include "rid_filter.h"
#include "sticky_register_accessors.h"
#include "tgid_hash.h"


#define DEFAULT_TIMEOUT_SCALING 1ul
#define FAKE_HARDWARE_TIMEOUT_SCALING 1000ul


#define RESET_TIMEOUT_SEC 60ul


#define DDR_CHUNK_ACK_TIMEOUT_SEC 4ul


#define DDR_CHUNK_CONFIG_TIMEOUT_SEC 2ul





#define DEFAULT_MAX_DDR_CHUNKS_PER_CTX 1024ul


ARGOS_FIELD_DECODER_WRAPPER(kernel_queue_ddr_control_change_requested)
ARGOS_FIELD_DECODER_WRAPPER(kernel_queue_ddr_status_pending_config)
ARGOS_FIELD_DECODER_WRAPPER(chip_global_state_value)
ARGOS_FIELD_DECODER_WRAPPER(kernel_chip_global_chip_reset_value)

int argos_wait_for_expected_value(
 struct argos_common_device_data *device_data, int bar,
 ulong offset, ulong timeout, ulong (*get_decoded_value)(uint64),
 ulong expected_value)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 ulong value, deadline;

 deadline = jiffies + timeout * device_data->timeout_scaling * HZ;
 do {
  value = gasket_dev_read_64(gasket_dev, bar, offset);
  if (get_decoded_value(value) == expected_value)
   return 0;
  schedule_timeout_interruptible(msecs_to_jiffies(1));
 } while (time_before(jiffies, deadline));


 value = gasket_dev_read_64(gasket_dev, bar, offset);
 if (get_decoded_value(value) == expected_value)
  return 0;

 return -ETIMEDOUT;
}
EXPORT_SYMBOL(argos_wait_for_expected_value);

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

void argos_initialize_gasket_mappable_region(
 struct gasket_mappable_region *region, uint64 start, uint64 end)
{
 region->start = start;
 region->length_bytes = end - start + sizeof(ulong);
 region->flags = VM_READ | VM_WRITE;
}
EXPORT_SYMBOL(argos_initialize_gasket_mappable_region);





int argos_device_enable_dev(struct gasket_dev *gasket_dev)
{
 struct argos_common_device_data *device_data;
 const struct argos_device_desc *device_desc;
 struct queue_ctx *queue_ctxs;
 int i;
 ulong fw_api_version;
 int ret, fw_bar;
 u64 value;
 bool valid;

 if (gasket_dev->cb_data == NULL) {
  gasket_log_error(gasket_dev, "Callback data is NULL!");
  return -EINVAL;
 }
 device_data = gasket_dev->cb_data;
 device_desc = device_data->device_desc;
 fw_bar = device_desc->firmware_register_bar;

 value = gasket_dev_read_64(gasket_dev, fw_bar,
  device_desc->firmware_api_version_location);
 fw_api_version = chip_global_api_version_version(value);
 if (fw_api_version != ARGOS_DEVICE_DRIVER_FIRMWARE_API_VERSION) {
  gasket_log_error(gasket_dev,
   "Firmware API version mismatch! Driver: %lu, firmware: %lu",
   ARGOS_DEVICE_DRIVER_FIRMWARE_API_VERSION,
   fw_api_version);
  return -EINVAL;
 }

 value = gasket_dev_read_64(gasket_dev, fw_bar,
  device_desc->max_ddr_chunks_per_ctx_location);
 valid = kernel_chip_global_max_ddr_chunks_per_ctx_valid(value) ==
   kKernelChipGlobalMaxDdrChunksPerCtxValidValueValid;
 device_data->max_chunks_per_queue_ctx = valid ?
  kernel_chip_global_max_ddr_chunks_per_ctx_chunks(value) :
  DEFAULT_MAX_DDR_CHUNKS_PER_CTX;

 if (device_desc->rid_filter.count) {
  ret = rid_filter_setup(device_data);
  if (ret)
   return ret;
 } else
  device_data->rid_filter_assignments = NULL;

 queue_ctxs = device_data->queue_ctxs;
 for (i = 0; i < device_desc->queue_ctx_count; i++) {
  queue_ctxs[i].pg_tbl = gasket_dev->page_table[i];
  INIT_LIST_HEAD(&queue_ctxs[i].direct_mappings);
 }

 value = gasket_dev_read_64(gasket_dev, fw_bar,
  device_desc->firmware_control_enable_location);
 device_data->is_real_hardware =
  sticky_registers_firmware_control_enable_cl(value) == 0;
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
 const struct argos_device_desc *device_desc = device_data->device_desc;
 struct argos_priority_algorithm_config config;
 u64 tmp;
 const int fw_bar = device_desc->firmware_register_bar;

 gasket_log_debug(gasket_dev,
    "Recvd ioctl ARGOS_IOCTL_SET_PRIORITY_ALGORITHM");


 if (!device_desc->priority_algorithm_config_supported)
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

 if (config.priority < 0 || config.priority >
  kernel_chip_global_priority_algorithm_priority_size() - 1) {
  gasket_log_error(gasket_dev, "invalid priority specified: %d",
   config.priority);
  return -EINVAL;
 }

 if (!kernel_chip_global_priority_algorithm_priority_value_is_valid(
  config.algorithm)) {
  gasket_log_error(gasket_dev, "invalid algorithm specified: %d",
   config.algorithm);
  return -EINVAL;
 }

 tmp = gasket_dev_read_64(gasket_dev, fw_bar,
  device_desc->priority_algorithm_config_location);
 set_kernel_chip_global_priority_algorithm_priority(
  &tmp, config.priority, config.algorithm);
 gasket_dev_write_64(gasket_dev, tmp, fw_bar,
  device_desc->priority_algorithm_config_location);

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

int argos_device_reset(struct gasket_dev *gasket_dev, uint reset_type)
{
 struct argos_common_device_data *device_data;
 const struct argos_device_desc *device_desc;
 int i, fw_bar;
 u64 value;

 if (gasket_dev->cb_data == NULL) {
  gasket_log_error(gasket_dev, "Callback data is NULL!");
  return -EINVAL;
 }
 device_data = gasket_dev->cb_data;
 device_desc = device_data->device_desc;
 fw_bar = device_desc->firmware_register_bar;

 if (reset_type != ARGOS_RESET_REINIT) {
  gasket_log_error(gasket_dev,
   "Invalid reset type specified: %u; only reinit reset is supported in the kernel.",
   reset_type);
  return -EINVAL;
 }


 value = gasket_dev_read_64(gasket_dev, fw_bar,
  device_desc->chip_reset_location);
 set_kernel_chip_global_chip_reset_value(&value,
  kKernelChipGlobalChipResetValueValueFunctionalReset);
 gasket_dev_write_64(gasket_dev, value, fw_bar,
  device_desc->chip_reset_location);


 if (argos_wait_for_expected_value(device_data, fw_bar,
   device_desc->chip_reset_location,
   RESET_TIMEOUT_SEC,
   ARGOS_NAME_FIELD_DECODER_WRAPPER(
    kernel_chip_global_chip_reset_value),
   kKernelChipGlobalChipResetValueValueResetAccepted))
  return -ETIMEDOUT;


 if (argos_wait_for_expected_value(device_data, fw_bar,
   device_desc->global_chip_state_location,
   RESET_TIMEOUT_SEC,
   ARGOS_NAME_FIELD_DECODER_WRAPPER(
    chip_global_state_value),
   kChipGlobalStateValueValueInitialized))
  return -ETIMEDOUT;


 for (i = 0; i < device_desc->queue_ctx_count; i++) {
  device_data->queue_ctxs[i].index = i;
  device_data->queue_ctxs[i].id[0] = '\0';
  device_data->queue_ctxs[i].owner = 0;
 }
 return 0;
}
EXPORT_SYMBOL(argos_device_reset);
# 413 "./drivers/char/argos/argos_device.c"
static int argos_dram_request_send_count_based(
  struct argos_common_device_data *device_data,
  const struct queue_ctx *queue_ctx,
  int original_chunks, int *chunk_delta)
{
 int available_chunks;
 u64 value;
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 const struct argos_device_desc *device_desc = device_data->device_desc;
 const int fw_bar = device_desc->firmware_register_bar;
 const int ddr_control_location =
  device_desc->kernel_queue_ddr_control_location(
   queue_ctx->index);


 value = gasket_dev_read_64(gasket_dev, fw_bar,
  device_desc->global_chip_ddr_state_location);
 available_chunks = chip_global_ddr_state_available_chunks(value);
 *chunk_delta = queue_ctx->dram_chunks - original_chunks;
 if (available_chunks < *chunk_delta) {
  gasket_log_error(gasket_dev,
   "Requesting more DDR chunks than available on the device!");
  return -ENOMEM;
 }


 value = gasket_dev_read_64(gasket_dev, fw_bar, ddr_control_location);
 set_kernel_queue_ddr_control_change_requested(
  &value,
  kKernelQueueDdrControlChangeRequestedValueCountBasedRequest);
 set_kernel_queue_ddr_control_requested_chunks(
  &value, queue_ctx->dram_chunks);
 gasket_dev_write_64(gasket_dev, value, fw_bar, ddr_control_location);

 return 0;
}
# 459 "./drivers/char/argos/argos_device.c"
static int argos_dram_request_send_bitmap_based(
  struct argos_common_device_data *device_data,
  struct queue_ctx *queue_ctx, int original_chunks,
  uint *bitmap, int bitmap_elems, int *chunk_delta)
{
 int i;
 u64 value;
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 const struct argos_device_desc *device_desc = device_data->device_desc;
 const int fw_bar = device_desc->firmware_register_bar;
 const int ddr_control_location =
  device_desc->kernel_queue_ddr_control_location(
   queue_ctx->index);






 *chunk_delta = -original_chunks;
 for (i = 0; i < bitmap_elems; i++) {
  gasket_dev_write_32(gasket_dev, bitmap[i], fw_bar,
        device_desc->dram_chunk_bitmap_location +
        i * sizeof(uint));
  *chunk_delta += hweight_long(bitmap[i]);
 }


 if (device_data->reserved_chunks <
     device_data->allocated_chunks + *chunk_delta) {
  gasket_log_error(gasket_dev,
   "Requesting more DDR chunks than reserved!");
  return -ENOMEM;
 }







 value = gasket_dev_read_64(gasket_dev, fw_bar, ddr_control_location);
 set_kernel_queue_ddr_control_change_requested(
  &value,
  kKernelQueueDdrControlChangeRequestedValueBitmapBasedRequest);
 gasket_dev_write_64(gasket_dev, value, fw_bar, ddr_control_location);

 return 0;
}
# 516 "./drivers/char/argos/argos_device.c"
static int get_dram_configuration_response(
 struct argos_common_device_data *device_data,
 int queue_index)
{
 int ret;
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 const struct argos_device_desc *device_desc = device_data->device_desc;
 const int fw_bar = device_desc->firmware_register_bar;

 ret = argos_wait_for_expected_value(device_data, fw_bar,
   device_desc->kernel_queue_ddr_control_location(
    queue_index),
   DDR_CHUNK_ACK_TIMEOUT_SEC,
   ARGOS_NAME_FIELD_DECODER_WRAPPER(
    kernel_queue_ddr_control_change_requested), 0);
 if (ret == -ETIMEDOUT) {
  gasket_log_error(gasket_dev,
   "HW/FW error: DDR config request not acked! Marking device unhealthy.");
  gasket_dev->status = GASKET_STATUS_DEAD;
  return -EIO;
 }


 ret = argos_wait_for_expected_value(device_data, fw_bar,
   device_desc->kernel_queue_ddr_status_location(
    queue_index),
   device_data->max_chunks_per_queue_ctx *
   DDR_CHUNK_ACK_TIMEOUT_SEC,
   ARGOS_NAME_FIELD_DECODER_WRAPPER(
    kernel_queue_ddr_status_pending_config), 0);
 if (ret == -ETIMEDOUT) {
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
 const int fw_bar = device_desc->firmware_register_bar;
 int ret = 0, pending_config, original_chunks, chunk_delta;
 u64 value;
 kernel_queue_ddr_status_value_value status;

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


 value = gasket_dev_read_64(gasket_dev, fw_bar,
  device_desc->kernel_queue_ddr_status_location(
   queue_ctx->index));
 pending_config = kernel_queue_ddr_status_pending_config(value);
 value = gasket_dev_read_64(gasket_dev, fw_bar,
  device_desc->kernel_queue_ddr_status_location(
   queue_ctx->index));
 status = kernel_queue_ddr_status_value(value);
 if (pending_config || status ==
  kKernelQueueDdrStatusValueValuePendingConfigInProgress) {
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

 value = gasket_dev_read_64(gasket_dev, fw_bar,
  device_desc->kernel_queue_ddr_status_location(
   queue_ctx->index));
 original_chunks = kernel_queue_ddr_status_current_chunks(value);

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
# 663 "./drivers/char/argos/argos_device.c"
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
 const struct gasket_mappable_region *mappable_bar_region;
 enum argos_security_level security_level;
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

 if (capable(CAP_SYS_ADMIN)) {
  security_level = ARGOS_SECURITY_LEVEL_ROOT;
 } else {
  security_level = ARGOS_SECURITY_LEVEL_USER;
 }

 if (bar_index == device_desc->firmware_register_bar) {

  *num_mappable_regions = 1;


  if ((gasket_dev->ownership.owner == current->tgid ||
   security_level == ARGOS_SECURITY_LEVEL_ROOT) &&
   !gasket_dev->parent) {
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



#ifndef STADIA_KERNEL

  if (security_level != ARGOS_SECURITY_LEVEL_ROOT)
   return 0;
#endif

  *num_mappable_regions =
   desc_map_regions->num_mappable_dram_regions;
  *mappable_regions = kmalloc(
   sizeof(struct gasket_mappable_region) *
    *num_mappable_regions, GFP_KERNEL);
  bar_desc =
   &driver_desc->bar_descriptions[device_desc->dram_bar];
  memcpy(*mappable_regions, bar_desc->mappable_regions,
   sizeof(struct gasket_mappable_region) *
    *num_mappable_regions);

 } else if (bar_index == device_desc->debug_bar) {

  if ((gasket_dev->ownership.owner != current->tgid &&
   security_level != ARGOS_SECURITY_LEVEL_ROOT) ||
   gasket_dev->parent) {
   return 0;
  }

  *num_mappable_regions = device_desc->get_bar_region_count(
    bar_index, security_level);
  if (*num_mappable_regions <= 0)
   return *num_mappable_regions;

  mappable_bar_region = device_desc->get_bar_regions(
    bar_index, security_level);
  if (mappable_bar_region == NULL) {
   gasket_log_error(gasket_dev,
    "Could not determine mappable regions for debug bar!");
   return -EINVAL;
  }

  *mappable_regions = kmalloc(
   sizeof(struct gasket_mappable_region) *
    *num_mappable_regions, GFP_KERNEL);
  if (*mappable_regions == NULL) {
   gasket_log_error(gasket_dev,
    "Unable to alloc mappable region block!");
   *num_mappable_regions = 0;
   return -ENOMEM;
  }

  for (i = 0; i < *num_mappable_regions; i++)
   (*mappable_regions)[i] = mappable_bar_region[i];

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

int argos_device_firmware_version_cb(struct gasket_dev *gasket_dev,
         unsigned int *major,
         unsigned int *minor,
         unsigned int *point,
         unsigned int *subpoint)
{
 struct argos_common_device_data *device_data;
 const struct argos_device_desc *device_desc;
 int fw_bar;
 u64 value;
 firmware_version_image_info_current_type_value image_type;

 if (gasket_dev->cb_data == NULL) {
  gasket_log_error(gasket_dev, "Callback data is NULL!");
  return -EINVAL;
 }
 device_data = gasket_dev->cb_data;
 device_desc = device_data->device_desc;
 fw_bar = device_desc->firmware_register_bar;


 value = gasket_dev_read_64(gasket_dev, fw_bar,
  device_desc->firmware_image_info_location);
 image_type = firmware_version_image_info_current_type(value);

 switch (image_type) {
 case kFirmwareVersionImageInfoCurrentTypeValuePrimary:
  value = gasket_dev_read_64(gasket_dev, fw_bar,
   device_desc->firmware_primary_version_location);
  *major = firmware_version_primary_version_major(value);
  *minor = firmware_version_primary_version_minor(value);
  *point = firmware_version_primary_version_point(value);
  *subpoint = firmware_version_primary_version_subpoint(value);
  return 0;
 case kFirmwareVersionImageInfoCurrentTypeValueSecondary:
  value = gasket_dev_read_64(gasket_dev, fw_bar,
   device_desc->firmware_secondary_version_location);
  *major = firmware_version_secondary_version_major(value);
  *minor = firmware_version_secondary_version_minor(value);
  *point = firmware_version_secondary_version_point(value);
  *subpoint = firmware_version_secondary_version_subpoint(value);
  return 0;
 default:




  gasket_log_warn(gasket_dev,
   "Cannot determine firmware version because image type is unknown!");
  *major = 0;
  *minor = 0;
  *point = 0;
  *subpoint = 0;
  return 0;
 }
}
EXPORT_SYMBOL(argos_device_firmware_version_cb);
