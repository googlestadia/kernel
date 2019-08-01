/*
 * Copyright (C) 2019 Google LLC.
 */
# 1 "./drivers/char/argos/argos_queue.c"
#include <linux/bitops.h>
#include <linux/mman.h>

#include "../../gasket/gasket_logging.h"
#include "argos_device.h"
#include "argos_queue.h"
#include "argos_types.h"
#include "tgid_hash.h"


#define QUEUE_CONTROL_DISABLE_TIMEOUT_SEC 2ul

int argos_find_free_queue_ctx(
 struct argos_common_device_data *device_data)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 int i;
 struct queue_ctx *queue_ctxs;

 queue_ctxs = device_data->queue_ctxs;
 if (queue_ctxs == NULL) {
  gasket_log_error(gasket_dev,
   "device descriptor is missing queue context pointer!");
  return -EFAULT;
 }

 for (i = 0; i < device_data->device_desc->queue_ctx_count; i++) {
  mutex_lock(&queue_ctxs[i].mutex);




  if (!device_data->argos_cb->is_queue_ctx_failed_cb(
    device_data, &queue_ctxs[i]) &&
      !queue_ctxs[i].allocated &&
      (!gasket_dev->parent || queue_ctxs[i].reserved)) {
   queue_ctxs[i].allocated = true;
   mutex_unlock(&queue_ctxs[i].mutex);
   return i;
  }

  mutex_unlock(&queue_ctxs[i].mutex);
 }

 gasket_log_error(gasket_dev, "No healthy & free queue contexts.");
 return -ENOMEM;
}
EXPORT_SYMBOL(argos_find_free_queue_ctx);







static bool owns_queue(
 struct argos_common_device_data *device_data, int queue_idx)
{
 return !device_data->gasket_dev->parent ||
  device_data->queue_ctxs[queue_idx].reserved;
}

int argos_lookup_queue_ctx(
 struct argos_common_device_data *device_data, const char *ctx_id,
 struct queue_ctx **queue_ctx)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 int i;
 struct queue_ctx *queue_ctxs;

 queue_ctxs = device_data->queue_ctxs;
 if (queue_ctxs == NULL) {
  gasket_log_error(gasket_dev,
   "device descriptor is missing queue context pointer!");
  return -EFAULT;
 }

 *queue_ctx = NULL;
 for (i = 0; i < device_data->device_desc->queue_ctx_count; i++)
  if (strncmp(ctx_id, queue_ctxs[i].id,
   ARGOS_NAME_MAX_LENGTH) == 0) {
   *queue_ctx = &queue_ctxs[i];
   return 0;
  }

 gasket_log_error(gasket_dev, "Queue context %s not found.", ctx_id);
 return -ENODEV;
}
EXPORT_SYMBOL(argos_lookup_queue_ctx);

int argos_allocate_queue_ctx(
 struct argos_common_device_data *device_data,
 struct argos_subcontainer_queue_ctx_config *config)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 struct queue_ctx *queue_ctx;
 int i, ctx_index, ret;
 ulong requested_chunks = 0;
 uint *bitmap_ints;
 int bitmap_num_ints;

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

 ctx_index = argos_find_free_queue_ctx(device_data);
 if (ctx_index < 0)
  return ctx_index;

 queue_ctx = &device_data->queue_ctxs[ctx_index];
 mutex_lock(&queue_ctx->mutex);
 if (config->priority < 0 || config->priority > 7) {
  gasket_log_error(
   gasket_dev, "Priority must be in the range [0, 7].");
  queue_ctx->allocated = 0;
  mutex_unlock(&queue_ctx->mutex);
  return -EINVAL;
 }

 if (config->use_chunk_bitmap) {

  requested_chunks = 0;
  bitmap_ints = (uint *) config->chunk_bitmap;
  bitmap_num_ints = DIV_ROUND_UP(config->num_chunks,
            sizeof(uint) * BITS_PER_BYTE);

  for (i = 0; i < bitmap_num_ints; i++)
   requested_chunks += hweight_long(bitmap_ints[i]);
 } else {
  bitmap_ints = NULL;
  bitmap_num_ints = 0;
  requested_chunks = config->num_chunks;
 }

 if (requested_chunks < 0 ||
     requested_chunks > device_data->max_chunks_per_queue_ctx) {
  gasket_log_error(gasket_dev,
   "Invalid DRAM chunks: %d. Valid range: [0-%d].",
   config->num_chunks,
   device_data->max_chunks_per_queue_ctx);
  queue_ctx->allocated = 0;
  mutex_unlock(&queue_ctx->mutex);
  return -EINVAL;
 }


 memset(queue_ctx->id, 0, ARGOS_NAME_MAX_LENGTH);
 memcpy(queue_ctx->id, config->name, ARGOS_NAME_MAX_LENGTH);
 queue_ctx->id[ARGOS_NAME_MAX_LENGTH] = '\0';
 queue_ctx->priority = config->priority;
 queue_ctx->dram_chunks = requested_chunks;


 ret = device_data->argos_cb->allocate_queue_ctx_cb(
  device_data, queue_ctx, config);





 if (ret)
  queue_ctx->allocated = 0;
 else
  gasket_log_debug(gasket_dev, "tgid: %d: Queue %d allocated.",
   current->tgid, queue_ctx->index);

 mutex_unlock(&queue_ctx->mutex);
 return ret;
}
EXPORT_SYMBOL(argos_allocate_queue_ctx);

int argos_deallocate_queue_ctx(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 int ret = 0;

 if (gasket_dev->status == GASKET_STATUS_DEAD)
  return -EIO;

 ret = device_data->argos_cb->deallocate_queue_ctx_cb(
  device_data, queue_ctx);

 if (queue_ctx->pg_tbl)
  gasket_page_table_unmap_all(queue_ctx->pg_tbl);
 queue_ctx->allocated = false;
 gasket_log_debug(gasket_dev,
  "tgid: %d: Deallocated queue context %d",
  current->tgid, queue_ctx->index);

 return ret;
}
EXPORT_SYMBOL(argos_deallocate_queue_ctx);

int argos_enable_queue_ctx(
 struct argos_common_device_data *device_data,
 const char *name, struct argos_queue_ctx_config *config)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 int ret;
 struct queue_ctx *queue_ctx = NULL;

 ret = argos_lookup_queue_ctx(device_data, name, &queue_ctx);
 if (ret)
  return ret;





 mutex_lock(&device_data->mutex);
 mutex_lock(&queue_ctx->mutex);

 if (!queue_ctx->allocated) {
  gasket_log_error(gasket_dev,
   "Queue %d/%s is not allocated!",
   queue_ctx->index, queue_ctx->id);
  ret = -EINVAL;
  goto out;
 }

 if (queue_ctx->owner != 0) {
  gasket_log_error(gasket_dev,
   "tgid %d: Queue %d is already enabled (owner: %d)",
   current->tgid, queue_ctx->index, queue_ctx->owner);
  ret = -EBUSY;
  goto out;
 }

 if (device_data->argos_cb->is_queue_ctx_failed_cb(
  device_data, queue_ctx)) {
  gasket_log_error(gasket_dev,
   "Cannot enable queue %d - it has failed.",
   queue_ctx->index);
  ret = -EIO;
  goto out;
 }





 ret = device_data->argos_cb->enable_queue_ctx_cb(
  device_data, queue_ctx);
 if (ret) {
  gasket_log_error(gasket_dev,
   "Error enabling queue context %d: %d", queue_ctx->index,
   ret);
  goto out;
 }

 ret = tgid_hash_queue_add(device_data, queue_ctx->index);
 if (ret) {
  ret = -EINVAL;
  goto out;
 }

 queue_ctx->owner = current->tgid;
 config->index = queue_ctx->index;
 config->dram_chunks = queue_ctx->dram_chunks;

 gasket_log_debug(gasket_dev, "tgid: %d: Queue %d enabled.",
  current->tgid, queue_ctx->index);

out:
 mutex_unlock(&queue_ctx->mutex);
 mutex_unlock(&device_data->mutex);
 return ret;
}
EXPORT_SYMBOL(argos_enable_queue_ctx);
# 292 "./drivers/char/argos/argos_queue.c"
static void remove_all_mmaps(
 struct argos_common_device_data *device_data,
 int map_bar_index,
 const struct gasket_mappable_region *map_region)
{
 bool unmapped = false;
 struct vm_area_struct *vma = NULL, *next_vma;

 lockdep_assert_held_once(&current->mm->mmap_sem);

 vma = current->mm->mmap;


 while (vma != NULL) {
  next_vma = vma->vm_next;




  if (!gasket_mm_unmap_region(
   device_data->gasket_dev, vma, map_bar_index,
   map_region))
   unmapped = true;
  vma = next_vma;
 }

 if (!unmapped)
  gasket_log_debug(device_data->gasket_dev,
   "Failed to unmap region [%#llx-%#llx]",
   map_region->start,
   map_region->start + map_region->length_bytes);
}
# 337 "./drivers/char/argos/argos_queue.c"
static int remove_direct_mapping(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx,
 struct direct_mapping *direct_mapping)
{
 int ret = 0;

 if (current->mm)
  remove_all_mmaps(device_data, direct_mapping->request.bar,
     &direct_mapping->mappable_region);

 if (device_data->argos_cb->deallocate_direct_mapping_cb)
  ret = device_data->argos_cb->deallocate_direct_mapping_cb(
   device_data, queue_ctx, direct_mapping);
 if (ret) {
  gasket_log_error(device_data->gasket_dev,
   "Error occurred in deallocating direct mapping for queue %d of BAR%d [%#llx, %#llx], prot=%d, rid_filter_window=%d",
   queue_ctx->index, direct_mapping->request.bar,
   direct_mapping->request.base,
   direct_mapping->request.base +
   direct_mapping->request.size - 1,
   direct_mapping->request.prot,
   direct_mapping->rid_filter_window);
 }

 list_del(&direct_mapping->list);
 kfree(direct_mapping);

 return ret;
}

int argos_disable_queue_ctx(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 int dm_ret = 0;
 int ret;
 struct gasket_mappable_region map_region;
 struct direct_mapping *direct_mapping, *next_direct_mapping;

 if (gasket_dev->status == GASKET_STATUS_DEAD)
  return 1;

 mutex_lock(&queue_ctx->direct_mappings_mutex);
 list_for_each_entry_safe(direct_mapping, next_direct_mapping,
     &queue_ctx->direct_mappings, list) {

  dm_ret |= remove_direct_mapping(
   device_data, queue_ctx, direct_mapping);
  if (dm_ret)
   gasket_dev->status = GASKET_STATUS_DEAD;
 }
 mutex_unlock(&queue_ctx->direct_mappings_mutex);
 if (dm_ret) {




  gasket_log_error(gasket_dev,
   "Error cleaning up direct mappings for queue context %d; marking device as unhealthy.",
   queue_ctx->index);
  gasket_dev->status = GASKET_STATUS_DEAD;
 }

 ret = device_data->argos_cb->disable_queue_ctx_cb(
  device_data, queue_ctx, &map_region);
 if (ret) {




  gasket_log_error(gasket_dev,
   "Error cleaning up queue context %d; marking as unhealthy.",
   queue_ctx->index);
  gasket_dev->status = GASKET_STATUS_DEAD;
 } else
  ret = dm_ret;






 if (queue_ctx->owner == current->tgid &&
  gasket_dev->ownership.owner != current->tgid &&
  !capable(CAP_SYS_ADMIN) && current->mm) {
  remove_all_mmaps(
   device_data,
   device_data->device_desc->firmware_register_bar,
   &map_region);
 }

 if (queue_ctx->pg_tbl)
  gasket_page_table_unmap_all(queue_ctx->pg_tbl);
 queue_ctx->owner = 0;

 return ret;
}
EXPORT_SYMBOL(argos_disable_queue_ctx);





static void disable_owned_queues(
 struct argos_common_device_data *device_data,
 struct tgid_hash_entry *hash_entry)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 int i;
 struct queue_ctx *queue_ctx;

 gasket_log_debug(gasket_dev, "Disabling queues owned by TGID %d",
  hash_entry->tgid);


 if (current->mm)
  down_read(&current->mm->mmap_sem);
 for (i = 0; i < device_data->device_desc->queue_ctx_count; i++) {
  if (tgid_hash_entry_queue_is_enabled(hash_entry, i)) {
   gasket_log_debug(gasket_dev, "Disabling queue %d", i);
   queue_ctx = &device_data->queue_ctxs[i];
   mutex_lock(&queue_ctx->mutex);
   argos_disable_queue_ctx(
    device_data, queue_ctx);
   mutex_unlock(&queue_ctx->mutex);

   schedule_timeout_interruptible(msecs_to_jiffies(1));
  }
 }
 if (current->mm)
  up_read(&current->mm->mmap_sem);

 tgid_hash_entry_clear_queues(hash_entry);
}

void argos_cleanup_hash_entry(struct kref *ref)
{
 bool has_clones = false;
 int i, hash_idx;
 struct tgid_hash_entry *hash_entry = container_of(
  ref, struct tgid_hash_entry, open_count);
 struct argos_common_device_data *device_data = hash_entry->device_data;
 struct gasket_dev *gasket_dev = device_data->gasket_dev;


 struct tgid_hash_entry *iterator;

 for (i = 0; i < GASKET_MAX_CLONES; i++)
  if (gasket_dev->clones[i])
   has_clones = true;






 if (current->tgid == gasket_dev->ownership.owner &&
     (!has_clones || gasket_dev->parent)) {
  gasket_log_info(gasket_dev,
   "Master process closing; cleaning up all queues and killing all workers.");







  hash_for_each(device_data->tgid_to_open_count, hash_idx,
   iterator, hlist_node)
   disable_owned_queues(device_data, iterator);

  hash_for_each(device_data->tgid_to_open_count, hash_idx,
   iterator, hlist_node)
   tgid_hash_entry_kill_worker(iterator);

  for (i = 0; i < device_data->device_desc->queue_ctx_count;
       i++) {
   if (owns_queue(device_data, i)) {
    argos_deallocate_queue_ctx(
     device_data,
     &device_data->queue_ctxs[i]);
    schedule_timeout_interruptible(
     msecs_to_jiffies(1));
   }
  }
 } else {
  disable_owned_queues(device_data, hash_entry);
 }

 tgid_hash_entry_free(hash_entry);
}
EXPORT_SYMBOL(argos_cleanup_hash_entry);

int argos_disable_and_deallocate_all_queues(
 struct argos_common_device_data *device_data)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 struct queue_ctx *queue_ctxs;
 int i, ret = 0;





 queue_ctxs = device_data->queue_ctxs;
 if (current->mm)
  down_read(&current->mm->mmap_sem);
 for (i = 0; i < device_data->device_desc->queue_ctx_count; i++) {

  mutex_lock(&queue_ctxs[i].mutex);
  if (queue_ctxs[i].allocated) {
   if (queue_ctxs[i].owner != 0)
    ret |= argos_disable_queue_ctx(
     device_data, &queue_ctxs[i]);

   ret |= argos_deallocate_queue_ctx(
    device_data, &queue_ctxs[i]);
  }






  if (gasket_dev->parent && queue_ctxs[i].reserved) {
   gasket_page_table_unmap_all(
     queue_ctxs[i].pg_tbl);
   gasket_page_table_garbage_collect(
     queue_ctxs[i].pg_tbl);
  }
  mutex_unlock(&queue_ctxs[i].mutex);
 }
 if (current->mm)
  up_read(&current->mm->mmap_sem);

 return ret;
}
EXPORT_SYMBOL(argos_disable_and_deallocate_all_queues);


int argos_dram_request_evaluate_response(
  struct gasket_dev *gasket_dev, struct queue_ctx *queue_ctx)
{
 int status, alloced_chunks;
 const struct argos_common_device_data *device_data =
  gasket_dev->cb_data;
 const struct argos_device_desc *device_desc = device_data->device_desc;

 status = argos_asic_read_64_indexed(device_data,
  &device_desc->queue_ddr_status_value, queue_ctx->index);
 alloced_chunks = argos_asic_read_64_indexed(
  device_data, &device_desc->queue_ddr_status_current_chunks,
  queue_ctx->index);

 switch (status) {
 case DDR_STATUS_VALUE_SUCCESS:
  if (alloced_chunks != queue_ctx->dram_chunks) {
   gasket_log_error(gasket_dev,
    "HW/FW error: invalid chunks allocated. Requested %d, received %d.",
    queue_ctx->dram_chunks, alloced_chunks);
   gasket_dev->status = GASKET_STATUS_DEAD;
   return -EIO;
  } else
   return 0;
 case DDR_STATUS_VALUE_NOT_ENOUGH_AVAILABLE:
  gasket_log_error(
   gasket_dev, "Insufficient DRAM chunks available.");
  return -ENOMEM;
 case DDR_STATUS_VALUE_TOO_LARGE:
  gasket_log_error(
   gasket_dev, "Too many DRAM chunks requested.");
  return -EINVAL;
 case DDR_STATUS_VALUE_IN_PROGRESS:
  gasket_log_error(
   gasket_dev,
   "Another DDR reconfigure request was in progress.");
  return -EBUSY;
 case DDR_STATUS_VALUE_QUEUE_NOT_DISABLED:
  gasket_log_error(gasket_dev,
     "The target queue ctx (%d) is not disabled!",
     queue_ctx->index);
  return -EBUSY;
 case DDR_STATUS_VALUE_INVALID_REQUEST_TYPE:
  gasket_log_error(gasket_dev,
     "Invalid DDR config request type.");
  return -EINVAL;
 case DDR_STATUS_VALUE_CHUNK_ALREADY_RESERVED:
  gasket_log_error(gasket_dev,
     "Memory allocation conflicted with an existing allocation.");
  return -ENOMEM;
 default:
  gasket_log_error(
   gasket_dev, "Invalid DDR config status: %d", status);
  return -EFAULT;
 }
}
EXPORT_SYMBOL(argos_dram_request_evaluate_response);
# 645 "./drivers/char/argos/argos_queue.c"
static int disable_firmware_queue_context(
 struct argos_common_device_data *device_data, int queue_idx,
 ulong control_offset, ulong status_offset)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 const struct argos_device_desc *device_desc = device_data->device_desc;
 int ret;

 gasket_dev_write_64(
  gasket_dev, 0, device_desc->firmware_register_bar,
  control_offset);

 ret = argos_wait_for_value(device_data,
  device_desc->firmware_register_bar,
  status_offset, QUEUE_CONTROL_DISABLE_TIMEOUT_SEC,
  device_desc->control_status_enabled.mask, 0);
 if (ret == -ECANCELED)
  return ret;
 else if (ret == -ETIMEDOUT) {
  gasket_log_error(gasket_dev,
   "Queue %d did not become disabled within timeout; status offset 0x%lx",
   queue_idx, status_offset);
  return -ETIMEDOUT;
 }

 return 0;
}

int argos_clear_firmware_queue_status(
 struct argos_common_device_data *device_data, int queue_idx)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 const struct argos_device_desc *device_desc = device_data->device_desc;
 int ret = disable_firmware_queue_context(device_data, queue_idx,
  device_desc->control_control.get_location(queue_idx),
  device_desc->control_status_enabled.get_location(queue_idx));
 gasket_dev_write_64(
  gasket_dev, 0, device_desc->firmware_register_bar,
  device_desc->interrupt_control_control.get_location(queue_idx));
 gasket_dev_write_64(
  gasket_dev, 0, device_desc->firmware_register_bar,
  device_desc->interrupt_control_status.get_location(queue_idx));
 return ret;
}
EXPORT_SYMBOL(argos_clear_firmware_queue_status);

int argos_common_allocate_queue_ctx_callback(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx,
 const struct argos_subcontainer_queue_ctx_config *config)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 const struct argos_device_desc *device_desc = device_data->device_desc;
 uint *bitmap_ints = NULL;
 int bitmap_num_ints = 0;
 int i, ret;

 if (config->use_chunk_bitmap) {
  bitmap_ints = (uint *) config->chunk_bitmap;
  bitmap_num_ints = DIV_ROUND_UP(config->num_chunks,
            sizeof(uint) * BITS_PER_BYTE);
 }


 gasket_read_modify_write_64(gasket_dev,
  device_desc->firmware_register_bar,
  device_desc->queue_control_priority_value.get_location(
   queue_ctx->index),
  config->priority,
  hweight_long(device_desc->queue_control_priority_value.mask),
  device_desc->queue_control_priority_value.shift);


 ret = argos_configure_queue_ctx_dram(
   device_data, queue_ctx,
   bitmap_ints, bitmap_num_ints);
# 730 "./drivers/char/argos/argos_queue.c"
 if (ret)
  return ret;

 if (config->use_chunk_bitmap) {





  for (i = 0; i < device_data->total_chunks; i++) {
   if (config->chunk_bitmap[i / BITS_PER_BYTE] &
       (1 << (i % BITS_PER_BYTE)))
    device_data->chunk_map[i] =
      queue_ctx->index;
  }
 }

 return ret;
}
EXPORT_SYMBOL(argos_common_allocate_queue_ctx_callback);

int argos_common_deallocate_queue_ctx_callback(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx)
{
 int i, ret = 0;


 argos_clear_firmware_queue_status(device_data, queue_ctx->index);

 for (i = 0; i < device_data->total_chunks; i++) {
  if (device_data->chunk_map[i] == queue_ctx->index)
   device_data->chunk_map[i] = 0xff;
 }

 if (queue_ctx->dram_chunks != 0) {
  queue_ctx->dram_chunks = 0;
  ret = argos_configure_queue_ctx_dram(
    device_data, queue_ctx, NULL, 0);
 }

 return ret;
}
EXPORT_SYMBOL(argos_common_deallocate_queue_ctx_callback);

bool argos_should_map_queue(
 struct argos_common_device_data *device_data, int queue_idx)
{
 bool subcontainer_master = false;
 struct gasket_dev *gasket_dev = device_data->gasket_dev;

 if (gasket_dev->parent && gasket_dev->ownership.owner == current->tgid)
  subcontainer_master = true;

 if (device_data->queue_ctxs[queue_idx].owner == current->tgid ||
     (subcontainer_master &&
      device_data->queue_ctxs[queue_idx].reserved))
  return true;

 return false;
}
EXPORT_SYMBOL(argos_should_map_queue);





static int lookup_queue_ctx_by_index(
 struct argos_common_device_data *device_data,
 int index,
 struct queue_ctx **queue_ctx)
{
 struct queue_ctx *queue_ctxs;

 queue_ctxs = device_data->queue_ctxs;
 if (queue_ctxs == NULL) {
  gasket_log_error(device_data->gasket_dev,
   "device descriptor is missing queue context pointer!");
  return -EFAULT;
 }

 if (index < 0 || index >= device_data->device_desc->queue_ctx_count) {
  gasket_log_error(device_data->gasket_dev,
   "Invalid queue index %d, must be in the range [0, %d]",
   index, device_data->device_desc->queue_ctx_count);
  return -EINVAL;
 }

 *queue_ctx = &queue_ctxs[index];

 return 0;
}







static bool queue_is_enabled_by_current(
 struct argos_common_device_data *device_data,
 const struct queue_ctx *queue_ctx)
{
 struct tgid_hash_entry *hash_entry;

 if (!queue_ctx->allocated)
  return false;

 hash_entry = tgid_hash_find(device_data, current->tgid);
 if (!hash_entry)
  return false;

 return tgid_hash_entry_queue_is_enabled(hash_entry, queue_ctx->index);
}





static int check_allocate_direct_mapping_request(
 const struct argos_common_device_data *device_data,
 const struct argos_direct_mapping_request *request)
{
 if (request->bar != 2) {
  gasket_log_error(device_data->gasket_dev,
   "Direct mapping bar must be 2 (the DRAM BAR): %d",
   request->bar);
  return -EINVAL;
 }

 if (request->base % ARGOS_DRAM_CHUNK_BYTES != 0) {
  gasket_log_error(device_data->gasket_dev,
   "Direct mapping base must be 2 MiB aligned: %#llx",
   request->base);
  return -EINVAL;
 }

 if (request->size == 0 || request->size % ARGOS_DRAM_CHUNK_BYTES != 0) {
  gasket_log_error(device_data->gasket_dev,
   "Direct mapping size must be non-zero and 2 MiB aligned: %#llx",
   request->size);
  return -EINVAL;
 }

 if ((request->prot & (PROT_READ | PROT_WRITE)) == 0 ||
     (request->prot & ~(PROT_READ | PROT_WRITE)) != 0) {
  gasket_log_error(device_data->gasket_dev,
   "Direct mapping protection is invalid: %d",
   request->prot);
  return -EINVAL;
 }

 if (!device_data->argos_cb->allocate_direct_mapping_cb) {
  gasket_log_error(device_data->gasket_dev,
   "Direct mapping not supported on this device");
  return -EOPNOTSUPP;
 }






 return 0;
}

int argos_allocate_direct_mapping(
 struct argos_common_device_data *device_data,
 struct argos_direct_mapping_request *request)
{
 int ret = 0;
 struct queue_ctx *queue_ctx;
 struct direct_mapping *direct_mapping;


 ret = check_allocate_direct_mapping_request(device_data, request);
 if (ret)
  return ret;





 ret = lookup_queue_ctx_by_index(
  device_data, request->queue_index, &queue_ctx);
 if (ret)
  return ret;

 mutex_lock(&queue_ctx->mutex);
 mutex_lock(&queue_ctx->direct_mappings_mutex);

 if (!queue_is_enabled_by_current(device_data, queue_ctx)) {
  gasket_log_error(device_data->gasket_dev,
   "Queue %d is not enabled and owned by the process",
   request->queue_index);
  ret = -EINVAL;
  goto exit;
 }


 if (request->base + request->size >
     ARGOS_DRAM_CHUNK_BYTES * (u64)queue_ctx->dram_chunks) {
  gasket_log_error(device_data->gasket_dev,
   "Direct mapping cannot exceed the queue %d's DRAM allocation: %#llx + %#llx > %#llx",
   request->queue_index, request->base, request->size,
   ARGOS_DRAM_CHUNK_BYTES * (u64)queue_ctx->dram_chunks);
  ret = -EINVAL;
  goto exit;
 }


 direct_mapping = kzalloc(sizeof(struct direct_mapping), GFP_KERNEL);
 if (!direct_mapping) {
  gasket_log_error(device_data->gasket_dev,
   "Unable to allocate direct mapping object; out of memory");
  ret = -ENOMEM;
  goto exit;
 }

 memcpy(&direct_mapping->request, request,
        sizeof(direct_mapping->request));
 direct_mapping->request.mmap_offset = -1;
 direct_mapping->rid_filter_window = -1;
 direct_mapping->mappable_region.start = -1;
 direct_mapping->mappable_region.length_bytes = request->size;
 direct_mapping->mappable_region.flags =
  (request->prot & PROT_READ ? VM_READ : 0) |
  (request->prot & PROT_WRITE ? VM_WRITE : 0);

 ret = device_data->argos_cb->allocate_direct_mapping_cb(
  device_data, queue_ctx, direct_mapping);
 if (ret) {
  kfree(direct_mapping);
  goto exit;
 }

 request->mmap_offset = direct_mapping->request.mmap_offset;

 list_add_tail(&direct_mapping->list, &queue_ctx->direct_mappings);

 gasket_log_info(device_data->gasket_dev,
  "Queue %d direct mapping allocated for BAR%d [%#llx-%#llx], prot=%d, rid_filter_window=%d, mmap_offset=%#llx, mappable_region=[%#llx, %#llx]",
  request->queue_index, request->bar, request->base,
  request->base + request->size - 1, request->prot,
  direct_mapping->rid_filter_window,
  request->mmap_offset,
  direct_mapping->mappable_region.start,
  direct_mapping->mappable_region.start +
  direct_mapping->mappable_region.length_bytes - 1);

exit:
 mutex_unlock(&queue_ctx->direct_mappings_mutex);
 mutex_unlock(&queue_ctx->mutex);
 return ret;
}
EXPORT_SYMBOL(argos_allocate_direct_mapping);

int argos_deallocate_direct_mapping(
 struct argos_common_device_data *device_data,
 const struct argos_direct_mapping_request *request)
{
 int ret = 0;
 bool removed_entry = false;
 struct queue_ctx *queue_ctx;
 struct direct_mapping *direct_mapping;





 ret = lookup_queue_ctx_by_index(
  device_data, request->queue_index, &queue_ctx);
 if (ret)
  return ret;

 mutex_lock(&queue_ctx->mutex);
 mutex_lock(&queue_ctx->direct_mappings_mutex);

 if (!queue_is_enabled_by_current(device_data, queue_ctx)) {
  gasket_log_error(device_data->gasket_dev,
   "Queue %d is not enabled and owned by the process",
   request->queue_index);
  ret = -EINVAL;
  goto exit;
 }


 list_for_each_entry(direct_mapping, &queue_ctx->direct_mappings, list) {
  const struct argos_direct_mapping_request *expected =
   &direct_mapping->request;

  gasket_log_debug(device_data->gasket_dev,
   "Checking direct mapping for queue %d of BAR%d [%#llx, %#llx], prot=%d",
   request->queue_index, expected->bar, expected->base,
   expected->base + expected->size - 1, expected->prot);
  if (expected->bar != request->bar ||
      expected->base != request->base ||
      expected->size != request->size ||
      expected->prot != request->prot ||
      expected->peer_rid_address != request->peer_rid_address ||
      expected->peer_rid_mask != request->peer_rid_mask)
   continue;

  ret = remove_direct_mapping(
   device_data, queue_ctx, direct_mapping);
  removed_entry = true;
  break;
 }
 if (!removed_entry) {
  gasket_log_error(device_data->gasket_dev,
   "Failed to find a direct mapping to deallocate for queue %d of BAR%d [%#llx, %#llx], prot=%d",
   request->queue_index, request->bar, request->base,
   request->base + request->size - 1,
   request->prot);
  ret = -EINVAL;
 }

exit:
 mutex_unlock(&queue_ctx->direct_mappings_mutex);
 mutex_unlock(&queue_ctx->mutex);
 return ret;
}
EXPORT_SYMBOL(argos_deallocate_direct_mapping);

int argos_get_direct_mappings_for_bar(
 struct argos_common_device_data *device_data,
 int bar_index, int *num_mappings,
 struct gasket_mappable_region *mappable_regions)
{
 int ret = 0, i, output_index = 0;
 struct queue_ctx *queue_ctx;
 struct direct_mapping *direct_mapping;

 for (i = 0; !ret && i < device_data->device_desc->queue_ctx_count;
      i++) {


#ifndef YETI_GUEST_KERNEL
  if (!argos_should_map_queue(device_data, i))
   continue;
#endif

  queue_ctx = &device_data->queue_ctxs[i];

  mutex_lock(&queue_ctx->direct_mappings_mutex);

  list_for_each_entry(direct_mapping, &queue_ctx->direct_mappings,
        list) {
   if (direct_mapping->request.bar != bar_index)
    continue;

   if (mappable_regions) {
    struct gasket_mappable_region *map_region =
     &direct_mapping->mappable_region;

    if (output_index >= *num_mappings) {
     gasket_log_error(
      device_data->gasket_dev,
      "No space to put direct mapping for queue %d! Direct mappings may have changed during mmap.",
      i);
     ret = -ENOMEM;
     break;
    }

    memcpy(&mappable_regions[output_index],
           map_region, sizeof(*map_region));

    gasket_log_debug(device_data->gasket_dev,
     "Added direct mappable region %d for queue %d: BAR%d [%#llx, %#llx], flags=%ld",
     output_index, i,
     direct_mapping->request.bar,
     map_region->start,
     map_region->start +
     map_region->length_bytes,
     map_region->flags);
   }

   output_index++;
  }

  mutex_unlock(&queue_ctx->direct_mappings_mutex);
 }

 *num_mappings = output_index;

 return ret;
}
EXPORT_SYMBOL(argos_get_direct_mappings_for_bar);

int argos_get_direct_mappable_regions(
 struct argos_common_device_data *device_data,
 int bar_index, struct gasket_mappable_region **mappable_regions,
 int *num_mappable_regions)
{
 int ret;

 ret = argos_get_direct_mappings_for_bar(
  device_data, bar_index, num_mappable_regions,
  NULL);
 if (ret || *num_mappable_regions == 0)
  return ret;

 *mappable_regions = kzalloc(
  sizeof(struct gasket_mappable_region) *
  *num_mappable_regions,
  GFP_KERNEL);
 if (*mappable_regions == NULL) {
  gasket_log_error(device_data->gasket_dev,
     "Unable to alloc mappable region block!");
  *num_mappable_regions = 0;
  return -ENOMEM;
 }

 ret = argos_get_direct_mappings_for_bar(
  device_data, bar_index, num_mappable_regions,
  *mappable_regions);
 if (ret) {
  kfree(*mappable_regions);
  *mappable_regions = NULL;
  return ret;
 }

 return 0;
}
EXPORT_SYMBOL(argos_get_direct_mappable_regions);

MODULE_DESCRIPTION("Argos shared components");
MODULE_VERSION(ARGOS_SHARED_VERSION);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rob Springer <rspringer@google.com>");
