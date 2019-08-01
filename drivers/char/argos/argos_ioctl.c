/*
 * Copyright (C) 2019 Google LLC.
 */
# 1 "./drivers/char/argos/argos_ioctl.c"
#include <linux/fs.h>
#include <linux/uaccess.h>

#include "../../gasket/gasket_core.h"
#include "../../gasket/gasket_logging.h"
#include "argos_device.h"
#include "argos_ioctl.h"
#include "argos_overseer.h"
#include "argos_queue.h"
#include "argos_types.h"
#include "tgid_hash.h"






static int argos_ioctl_check_is_master(struct gasket_dev *gasket_dev, ulong arg)
{
 int is_master;

 is_master = argos_check_ownership(gasket_dev);
 if (copy_to_user((void __user *)arg, &is_master, sizeof(is_master)))
  return -EFAULT;

 return 0;
}
# 36 "./drivers/char/argos/argos_ioctl.c"
static int argos_ioctl_allocate_queue_ctx(
 struct argos_common_device_data *device_data, ulong arg)
{
 struct argos_subcontainer_queue_ctx_config subcontainer_alloc_cfg;
 struct argos_queue_ctx_config alloc_cfg;


 if (copy_from_user(&alloc_cfg, (void __user *)arg,
  sizeof(struct argos_queue_ctx_config)))
  return -EFAULT;
 memcpy(subcontainer_alloc_cfg.name, alloc_cfg.name,
  ARGOS_NAME_MAX_LENGTH);
 subcontainer_alloc_cfg.priority = alloc_cfg.priority;
 subcontainer_alloc_cfg.num_chunks =
  alloc_cfg.dram_chunks;
 subcontainer_alloc_cfg.use_chunk_bitmap = false;
 subcontainer_alloc_cfg.index = alloc_cfg.index;

 return argos_allocate_queue_ctx(
  device_data, &subcontainer_alloc_cfg);
}
# 65 "./drivers/char/argos/argos_ioctl.c"
static int argos_ioctl_subcontainer_allocate_queue_ctx(
 struct argos_common_device_data *device_data, ulong arg)
{
 int i, ret, bitmap_ints;
 struct argos_subcontainer_queue_ctx_config alloc_cfg;
 __u8 *bitmap;






 if (!device_data->gasket_dev->parent) {
  gasket_log_error(device_data->gasket_dev,
   "ARGOS_IOCTL_SUBCONTAINER_ALLOCATE_QUEUE_CTX may only be called by subcontainers!");
  return -EPERM;
 }

 if (copy_from_user(&alloc_cfg, (void __user *)arg,
  sizeof(struct argos_subcontainer_queue_ctx_config)))
  return -EFAULT;

 if (alloc_cfg.num_chunks != device_data->total_chunks) {
  gasket_log_error(device_data->gasket_dev,
     "Chunk buffer wrong size: %u vs. %u",
     alloc_cfg.num_chunks,
     device_data->total_chunks);
  return -EFAULT;
 }


 bitmap_ints = DIV_ROUND_UP(alloc_cfg.num_chunks,
       sizeof(uint) * BITS_PER_BYTE);
 bitmap = kmalloc_array(bitmap_ints, sizeof(uint), GFP_KERNEL);
 if (!bitmap)
  return -ENOMEM;

 if (copy_from_user(bitmap, (void __user *) alloc_cfg.chunk_bitmap,
      bitmap_ints * sizeof(uint))) {
  ret = -EFAULT;
  goto out;
 }



 for (i = 0; i < device_data->total_chunks; i++)
  if (test_bit(i, (ulong *) bitmap) &&
   device_data->chunk_map[i] != 0xFF) {
   gasket_log_info(device_data->gasket_dev,
    "Requesting already-allocated chunk %d", i);
   ret = -ENOMEM;
   goto out;
  }

 alloc_cfg.chunk_bitmap = bitmap;
 ret = argos_allocate_queue_ctx(device_data, &alloc_cfg);
out:
 kfree(bitmap);
 return ret;
}






static int argos_ioctl_deallocate_queue_ctx(
 struct argos_common_device_data *device_data, ulong arg)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 char ctx_id[ARGOS_NAME_MAX_LENGTH];
 struct queue_ctx *queue_ctx = NULL;
 int ret;
 pid_t queue_owner;

 if (!argos_check_ownership(gasket_dev)) {
  gasket_log_error(gasket_dev,
   "device is owned by tgid %d; tgid %d can not modify.",
   gasket_dev->ownership.owner, current->tgid);
  return -EPERM;
 }

 if (copy_from_user(&ctx_id, (void __user *)arg, ARGOS_NAME_MAX_LENGTH))
  return -EFAULT;

 ret = argos_lookup_queue_ctx(device_data, ctx_id, &queue_ctx);
 if (ret)
  return ret;

 mutex_lock(&queue_ctx->mutex);
 if (!queue_ctx->allocated) {
  mutex_unlock(&queue_ctx->mutex);
  return 0;
 }

 queue_owner = queue_ctx->owner;
 if (queue_owner != 0) {

  gasket_log_debug(gasket_dev,
   "Disabling queue context %s (%i) during deallocation.",
   ctx_id, queue_ctx->index);
  ret = argos_disable_queue_ctx(device_data, queue_ctx);





  ret |= tgid_hash_queue_remove(
   device_data, queue_ctx->index, queue_owner);
 }

 ret |= argos_deallocate_queue_ctx(device_data, queue_ctx);
 mutex_unlock(&queue_ctx->mutex);
 return ret;
}






static int argos_ioctl_enable_queue_ctx(
 struct argos_common_device_data *device_data, ulong arg)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 int ret;
 struct argos_queue_ctx_config config;

 if (gasket_dev->status == GASKET_STATUS_DEAD) {
  gasket_log_error(gasket_dev, "The device has failed.");
  return -EIO;
 }

 if (copy_from_user(&config, (void __user *)arg,
  sizeof(struct argos_queue_ctx_config))) {
  gasket_log_error(gasket_dev,
   "Unable to copy data from user pointer.");
  return -EFAULT;
 }

 ret = argos_enable_queue_ctx(device_data, config.name, &config);
 if (ret)
  return ret;

 if (copy_to_user((void __user *)arg, &config, sizeof(config))) {
  struct queue_ctx *queue_ctx;

  gasket_log_error(gasket_dev,
   "Unable to copy data to user pointer.");
  if (!argos_lookup_queue_ctx(
    device_data, config.name, &queue_ctx))
   argos_disable_queue_ctx(device_data, queue_ctx);
  return -EFAULT;
 }

 return 0;
}






static int argos_ioctl_disable_queue_ctx(
 struct argos_common_device_data *device_data, ulong arg)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 int ret = 0;
 pid_t queue_owner;
 char ctx_id[ARGOS_NAME_MAX_LENGTH];
 struct queue_ctx *queue_ctx = NULL;

 if (copy_from_user(&ctx_id, (void __user *)arg, ARGOS_NAME_MAX_LENGTH))
  return -EFAULT;

 ret = argos_lookup_queue_ctx(device_data, ctx_id, &queue_ctx);
 if (ret)
  return ret;

 if (current->mm)
  down_read(&current->mm->mmap_sem);
 mutex_lock(&queue_ctx->mutex);
 if (queue_ctx->owner == 0) {
  gasket_log_info(gasket_dev,
    "Queue %s/%d is already disabled!",
    queue_ctx->id, queue_ctx->index);
  ret = 0;
  goto early_exit;
 }

 if (gasket_dev->ownership.is_owned &&
  gasket_dev->ownership.owner != current->tgid &&
  queue_ctx->owner != current->tgid) {
  gasket_log_error(gasket_dev,
   "Cannot disable queue: device is owned by TGID %d and queue %s is owned by TGID %d",
   gasket_dev->ownership.owner, queue_ctx->id,
   queue_ctx->owner);

  ret = -EPERM;
  goto early_exit;
 } else if (!gasket_dev->ownership.is_owned) {
  gasket_log_error(gasket_dev,
   "Cannot disable an unowned queue (id %d)",
   queue_ctx->index);
  ret = -EINVAL;
  goto early_exit;
 }

 queue_owner = queue_ctx->owner;
 ret = argos_disable_queue_ctx(device_data, queue_ctx);
 mutex_unlock(&queue_ctx->mutex);
 if (current->mm)
  up_read(&current->mm->mmap_sem);

 mutex_lock(&device_data->mutex);
 ret = tgid_hash_queue_remove(
  device_data, queue_ctx->index, current->tgid);
 mutex_unlock(&device_data->mutex);

 gasket_log_debug(gasket_dev, "tgid: %d: queue %d disabled (was owned by %d)",
  current->tgid, queue_ctx->index, queue_owner);
 return ret;

early_exit:
 mutex_unlock(&queue_ctx->mutex);
 if (current->mm)
  up_read(&current->mm->mmap_sem);

 return ret;
}






static int argos_ioctl_allocate_direct_mapping(
 struct argos_common_device_data *device_data, ulong arg)
{
 struct argos_direct_mapping_request request;
 int ret;

 if (copy_from_user(&request, (void __user *)arg, sizeof(request))) {
  gasket_log_error(device_data->gasket_dev,
   "Unable to copy data from user pointer.");
  return -EFAULT;
 }

 ret = argos_allocate_direct_mapping(device_data, &request);
 if (ret)
  return ret;

 if (copy_to_user((void __user *)arg, &request, sizeof(request))) {
  gasket_log_error(device_data->gasket_dev,
   "Unable to copy data to user pointer.");
  argos_deallocate_direct_mapping(device_data, &request);
  return -EFAULT;
 }

 return 0;
}






static int argos_ioctl_deallocate_direct_mapping(
 struct argos_common_device_data *device_data, ulong arg)
{
 struct argos_direct_mapping_request request;

 if (copy_from_user(&request, (void __user *)arg, sizeof(request))) {
  gasket_log_error(device_data->gasket_dev,
   "Unable to copy data from user pointer.");
  return -EFAULT;
 }

 return argos_deallocate_direct_mapping(device_data, &request);
}

int argos_queue_ioctl_dispatch(
 struct argos_common_device_data *device_data, uint cmd, ulong arg)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;

 switch (cmd) {
 case ARGOS_IOCTL_PROCESS_IS_MASTER:
  gasket_log_debug(gasket_dev,
     "Recvd ioctl ARGOS_IOCTL_PROCESS_IS_MASTER");
  return argos_ioctl_check_is_master(gasket_dev, arg);
 case ARGOS_IOCTL_ALLOCATE_QUEUE_CTX:
  gasket_log_debug(gasket_dev,
     "Recvd ioctl ARGOS_IOCTL_ALLOCATE_QUEUE_CTX");
  return argos_ioctl_allocate_queue_ctx(device_data, arg);
 case ARGOS_IOCTL_DEALLOCATE_QUEUE_CTX:
  gasket_log_debug(gasket_dev,
     "Recvd ioctl ARGOS_IOCTL_DEALLOCATE_QUEUE_CTX");
  return argos_ioctl_deallocate_queue_ctx(device_data, arg);
 case ARGOS_IOCTL_ENABLE_QUEUE_CTX:
  gasket_log_debug(gasket_dev,
     "Recvd ioctl ARGOS_IOCTL_ENABLE_QUEUE_CTX");
  return argos_ioctl_enable_queue_ctx(device_data, arg);
 case ARGOS_IOCTL_DISABLE_QUEUE_CTX:
  gasket_log_debug(gasket_dev,
     "Recvd ioctl ARGOS_IOCTL_DISABLE_QUEUE_CTX");
  return argos_ioctl_disable_queue_ctx(device_data, arg);
 case ARGOS_IOCTL_SUBCONTAINER_ALLOCATE_QUEUE_CTX:
  gasket_log_debug(gasket_dev,
     "Recvd ioctl ARGOS_IOCTL_SUBCONTAINER_ALLOCATE_QUEUE_CTX");
  return argos_ioctl_subcontainer_allocate_queue_ctx(
    device_data, arg);
 case ARGOS_IOCTL_ALLOCATE_DIRECT_MAPPING:
  gasket_log_debug(gasket_dev,
   "Recvd ioctl ARGOS_IOCTL_ALLOCATE_DIRECT_MAPPING");
  return argos_ioctl_allocate_direct_mapping(device_data, arg);
 case ARGOS_IOCTL_DEALLOCATE_DIRECT_MAPPING:
  gasket_log_debug(gasket_dev,
   "Recvd ioctl ARGOS_IOCTL_DEALLOCATE_DIRECT_MAPPING");
  return argos_ioctl_deallocate_direct_mapping(device_data, arg);
 default:
  return -EOPNOTSUPP;
 }
}
EXPORT_SYMBOL(argos_queue_ioctl_dispatch);

int argos_check_gasket_ioctl_permissions(
 struct file *filp, uint cmd, ulong arg)
{

 struct argos_common_device_data *device_data;
 struct gasket_dev *gasket_dev = filp->private_data;
 struct gasket_interrupt_eventfd interrupt_data;
 struct gasket_page_table_ioctl page_table_data;
 int is_master =
  argos_check_ownership(gasket_dev) || capable(CAP_SYS_ADMIN);
 struct queue_ctx *queue_ctxs;

 if (gasket_dev->cb_data == NULL) {
  gasket_log_error(gasket_dev, "Callback data is NULL!");
  return -EINVAL;
 }
 device_data = gasket_dev->cb_data;


 if (gasket_dev->parent)
  return argos_subcontainer_gasket_ioctl_has_permission(
   device_data, cmd, arg);
 else if (device_data->mode == ARGOS_MODE_OVERSEER)
  return argos_overseer_gasket_ioctl_has_permission(
   device_data, cmd, arg);

 queue_ctxs = device_data->queue_ctxs;

 switch (cmd) {
 case GASKET_IOCTL_RESET:
 case GASKET_IOCTL_CLEAR_INTERRUPT_COUNTS:
  return is_master;

 case GASKET_IOCTL_SET_EVENTFD:
 case GASKET_IOCTL_CLEAR_EVENTFD:
  if (copy_from_user(&interrupt_data, (void __user *)arg,
   sizeof(struct gasket_interrupt_eventfd)))
   return -EFAULT;

  if (interrupt_data.interrupt >=
   device_data->gasket_driver_desc->num_interrupts)
   return -EINVAL;
# 441 "./drivers/char/argos/argos_ioctl.c"
  if (is_master)
   return 1;
  else if (interrupt_data.interrupt ==
   device_data->device_desc->failed_codec_interrupt)
   return 0;
  else if (queue_ctxs[interrupt_data.interrupt].owner ==
   current->tgid)
   return 1;

  return 0;

 case GASKET_IOCTL_MAP_BUFFER:
 case GASKET_IOCTL_UNMAP_BUFFER:
  if (copy_from_user(&page_table_data, (void __user *)arg,
   sizeof(struct gasket_page_table_ioctl)))
   return -EFAULT;

  if (page_table_data.page_table_index >=
      device_data->gasket_driver_desc->num_page_tables)
   return -EINVAL;

  if (is_master ||
   queue_ctxs[page_table_data.page_table_index].owner ==
   current->tgid)
   return 1;

  return 0;

 case GASKET_IOCTL_NUMBER_PAGE_TABLES:
 case GASKET_IOCTL_PAGE_TABLE_SIZE:
 case GASKET_IOCTL_SIMPLE_PAGE_TABLE_SIZE:
  return 1;

 case GASKET_IOCTL_PARTITION_PAGE_TABLE:
  return 0;

 default:
  gasket_log_error(
   gasket_dev, "Invalid ioctl specified: %d", cmd);
  return -EINVAL;
 }
}
EXPORT_SYMBOL(argos_check_gasket_ioctl_permissions);
