/*
 * Copyright (C) 2019 Google LLC.
 */
# 2 "./drivers/char/argos/argos_overseer.c"
#include <linux/accel.h>
#include <linux/fs.h>
#include <linux/google/argos.h>
#include <linux/uaccess.h>

#include "argos_overseer.h"
#include "argos_types.h"

#include "../../gasket/gasket_logging.h"
#include "../../gasket/gasket_sysfs.h"

enum sysfs_attribute_type {
 ATTR_SUBCONTAINERS,
 ATTR_SUBCONTAINER_ID,
 ATTR_SUBCONTAINER_MEMORY_0,
 ATTR_SUBCONTAINER_MEMORY_1,
};

static ssize_t sysfs_show(
 struct device *device, struct device_attribute *attr, char *buf);
static ssize_t sysfs_show_subcontainers(
 struct argos_common_device_data *device_data, char *buf);
static int argos_overseer_start(struct argos_common_device_data *device_data);
static int argos_overseer_stop(struct argos_common_device_data *device_data);

static const struct gasket_sysfs_attribute parent_sysfs_attrs[] = {
 GASKET_SYSFS_RO(subcontainers, sysfs_show, ATTR_SUBCONTAINERS),
 GASKET_SYSFS_RO(subcontainer_memory_0, sysfs_show,
   ATTR_SUBCONTAINER_MEMORY_0),
 GASKET_SYSFS_RO(subcontainer_memory_1, sysfs_show,
   ATTR_SUBCONTAINER_MEMORY_1),
 GASKET_END_OF_ATTR_ARRAY
};

static const struct gasket_sysfs_attribute subcontainer_sysfs_attrs[] = {
 GASKET_SYSFS_RO(subcontainer_id, sysfs_show,
   ATTR_SUBCONTAINER_ID),
 GASKET_SYSFS_RO(subcontainer_memory_0, sysfs_show,
   ATTR_SUBCONTAINER_MEMORY_0),
 GASKET_SYSFS_RO(subcontainer_memory_1, sysfs_show,
   ATTR_SUBCONTAINER_MEMORY_1),
 GASKET_END_OF_ATTR_ARRAY
};

static void init_device_data(
 struct argos_common_device_data *src_data,
 struct argos_common_device_data *dst_data)
{
 int i;

 for (i = 0; i < dst_data->device_desc->queue_ctx_count; i++) {
  dst_data->queue_ctxs[i].pg_tbl =
    dst_data->gasket_dev->page_table[i];
  INIT_LIST_HEAD(&dst_data->queue_ctxs[i].direct_mappings);
 }

 dst_data->is_real_hardware = src_data->is_real_hardware;
 dst_data->timeout_scaling = src_data->timeout_scaling;
 dst_data->chunk_map = src_data->chunk_map;
}





static void remove_dev_nodes(
 struct argos_common_device_data *device_data, int num_to_remove)
{
 struct argos_overseer_subcontainer *subcontainer;
 int i, j;

 for (i = 0; i < num_to_remove; i++) {
  subcontainer = &device_data->subcontainers[i];
  if (subcontainer->registered) {
   gasket_clone_cleanup(&subcontainer->gasket_dev);
   subcontainer->registered = false;
  }
 }

 for (i = 0; i < GASKET_MAX_CLONES; i++) {
  struct argos_common_device_data *sub_data =
   device_data->subcontainers[i].gasket_dev.cb_data;
  if (!sub_data)
   continue;

  gasket_sysfs_remove_entries(
    &sub_data->gasket_dev->accel_dev.dev,
    subcontainer_sysfs_attrs);

  sub_data->reserved_chunks = 0;
  for (j = 0; j < device_data->device_desc->queue_ctx_count; j++)
   sub_data->queue_ctxs[i].reserved = false;

  sysfs_remove_link(&sub_data->gasket_dev->accel_dev.dev.kobj,
      "subcontainer_parent");
 }
}







static int create_dev_nodes(struct argos_common_device_data *device_data)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 struct argos_overseer_subcontainer *subcontainer;
 int i, ret = 0, created_clones = 0;

 memset(device_data->subcontainers, 0,
        sizeof(struct argos_overseer_subcontainer) *
        GASKET_MAX_CLONES);
 for (i = 0; i < GASKET_MAX_CLONES; i++) {
  subcontainer = &device_data->subcontainers[i];

  ret = gasket_clone_create(gasket_dev,
       &subcontainer->gasket_dev);
  if (ret) {
   gasket_log_error(gasket_dev,
    "Cannot register subdevice %d", i);
   goto fail;
  }
  created_clones++;
  subcontainer->registered = true;

  init_device_data(device_data, subcontainer->gasket_dev.cb_data);
# 139 "./drivers/char/argos/argos_overseer.c"
  gasket_sysfs_create_entries(
    &subcontainer->gasket_dev.accel_dev.dev,
    subcontainer_sysfs_attrs);


  ret = sysfs_create_link(
    &subcontainer->gasket_dev.accel_dev.dev.kobj,
    &gasket_dev->accel_dev.dev.kobj,
    "subcontainer_parent");
  if (ret)
   goto fail;
 }
 return 0;

fail:
 remove_dev_nodes(device_data, created_clones);
 return ret;
}

static int argos_overseer_start(struct argos_common_device_data *device_data)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 int ret;

 ret = create_dev_nodes(device_data);
 if (ret)
  return ret;

 device_data->mode = ARGOS_MODE_OVERSEER;


 gasket_sysfs_create_entries(&gasket_dev->accel_dev.dev,
        parent_sysfs_attrs);

 gasket_log_info(gasket_dev, "Entered overseer mode.");
 return 0;
}

static int argos_overseer_stop(struct argos_common_device_data *device_data)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 int i;

 gasket_sysfs_remove_entries(&gasket_dev->accel_dev.dev,
        parent_sysfs_attrs);
 remove_dev_nodes(device_data, GASKET_MAX_CLONES);

 device_data->mode = ARGOS_MODE_NORMAL;
 device_data->reserved_chunks = device_data->total_chunks;
 for (i = 0; i < device_data->device_desc->queue_ctx_count; i++)
  device_data->queue_ctxs[i].reserved = false;
 gasket_log_info(gasket_dev, "Entered normal execution mode.");
 return 0;
}
# 208 "./drivers/char/argos/argos_overseer.c"
static int argos_overseer_set_mode(
 struct argos_common_device_data *device_data, ulong arg)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 int i, ret = 0;

 if (gasket_dev->parent != NULL) {
  gasket_log_error(gasket_dev,
     "Subcontainers cannot set overseer modes!");
  return -EINVAL;
 }

 mutex_lock(&gasket_dev->mutex);
 mutex_lock(&device_data->mutex);
 if (arg == ARGOS_MODE_NORMAL &&
     device_data->mode == ARGOS_MODE_OVERSEER) {




  for (i = 0; i < GASKET_MAX_CLONES; i++) {
   if (device_data->subcontainers[i].gasket_dev
     .ownership.is_owned) {
    gasket_log_error(gasket_dev,
     "Cannot enter normal mode; subcontainer %d is in use",
     i);
    ret = -EINVAL;
    goto out;
   }
  }

  ret = argos_overseer_stop(device_data);

 } else if (arg == ARGOS_MODE_OVERSEER &&
     device_data->mode == ARGOS_MODE_NORMAL) {




  for (i = 0; i < device_data->device_desc->queue_ctx_count;
       i++) {
   if (device_data->queue_ctxs[i].allocated) {
    gasket_log_error(gasket_dev,
     "Cannot enter overseer mode; queue ctx %d is allocated",
     i);
    ret = -EINVAL;
    goto out;
   }
  }

  ret = argos_overseer_start(device_data);
 }

out:
 mutex_unlock(&device_data->mutex);
 mutex_unlock(&gasket_dev->mutex);
 return ret;
}
# 281 "./drivers/char/argos/argos_overseer.c"
static int argos_overseer_reserve_resources(
 struct argos_common_device_data *device_data, ulong arg)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 struct argos_overseer_reservation_request request;
 struct argos_common_device_data *subcontainer;
 int i, queue_delta, chunk_delta;
 int free_queues = 0, reserved_queues = 0, ret = 0;

 if (device_data->mode != ARGOS_MODE_OVERSEER) {
  gasket_log_error(gasket_dev,
   "Resources can only be reserved in overseer mode.");
  return -EPERM;
 }

 if (copy_from_user(&request, (void __user *)arg, sizeof(request)))
  return -EFAULT;

 if (request.subcontainer_index > GASKET_MAX_CLONES) {
  gasket_log_error(gasket_dev,
     "Subcontainer index out-of-bounds: %d",
     request.subcontainer_index);
  return -EINVAL;
 }

 mutex_lock(&device_data->mutex);
 subcontainer =
  device_data->subcontainers[request.subcontainer_index]
  .gasket_dev.cb_data;
 chunk_delta = request.num_chunks - subcontainer->reserved_chunks;

 if (chunk_delta > 0 && device_data->reserved_chunks < chunk_delta) {
  gasket_log_error(gasket_dev,
   "Insufficient free DRAM chunks for request: avail/req delta: %d/%d (total req %d)",
   device_data->reserved_chunks, chunk_delta,
   request.num_chunks);
  ret = -ENOMEM;
  goto exit;
 }

 for (i = 0; i < device_data->device_desc->queue_ctx_count; i++)
  if (subcontainer->queue_ctxs[i].reserved) {
   reserved_queues++;
   if (subcontainer->queue_ctxs[i].owner) {
    gasket_log_warn(gasket_dev,
       "Can't adjust reservations with a queue allocated!");
    ret = -EBUSY;
    goto exit;
   }
  }

 queue_delta = request.num_queues - reserved_queues;

 for (i = 0; i < device_data->device_desc->queue_ctx_count; i++)
  if (!device_data->queue_ctxs[i].reserved)
   free_queues++;

 if (queue_delta > 0 && free_queues < queue_delta) {
  gasket_log_error(gasket_dev,
     "Insufficient free queues for request: avail/req delta: %d/%d (total req %d)",
     free_queues, queue_delta, request.num_queues);
  ret = -EBUSY;
  goto exit;
 }





 if (chunk_delta != 0) {
  subcontainer->reserved_chunks += chunk_delta;
  device_data->reserved_chunks -= chunk_delta;
  gasket_log_debug(gasket_dev,
     "Subcontainer %d to %d chunks. %d remaining in overseer.",
     request.subcontainer_index,
     subcontainer->reserved_chunks,
     device_data->reserved_chunks);
 }

 if (queue_delta > 0) {

  for (i = 0; i < device_data->device_desc->queue_ctx_count &&
       queue_delta > 0; i++)
   if (!device_data->queue_ctxs[i].reserved) {
    device_data->queue_ctxs[i].reserved = true;
    subcontainer->queue_ctxs[i].reserved = true;
    queue_delta--;
    gasket_log_debug(gasket_dev,
       "Reserving queue ctx %d for subcontainer %d",
       i, request.subcontainer_index);
   }
 } else if (queue_delta < 0) {

  for (i = 0; i < device_data->device_desc->queue_ctx_count &&
       queue_delta < 0; i++)
   if (subcontainer->queue_ctxs[i].reserved &&
       !subcontainer->queue_ctxs[i].owner) {
    subcontainer->queue_ctxs[i].reserved = false;
    device_data->queue_ctxs[i].reserved = false;
    queue_delta++;
    gasket_log_debug(gasket_dev,
       "Returning queue ctx %d from subcontainer %d",
       i, request.subcontainer_index);
   }
 }

 if (queue_delta != 0)
  gasket_log_error(gasket_dev,
     "INTERNAL ERROR: Resource leak - queue_delta should be 0 after reservation (is %d)!",
     queue_delta);

exit:
 mutex_unlock(&device_data->mutex);
 return ret;
}

int argos_overseer_gasket_ioctl_has_permission(
 struct argos_common_device_data *device_data, uint cmd, ulong arg)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;

 switch (cmd) {
 case GASKET_IOCTL_PARTITION_PAGE_TABLE:
 case GASKET_IOCTL_CLEAR_EVENTFD:
 case GASKET_IOCTL_SET_EVENTFD:
 case GASKET_IOCTL_MAP_BUFFER:
 case GASKET_IOCTL_UNMAP_BUFFER:

  return 0;
 case GASKET_IOCTL_CLEAR_INTERRUPT_COUNTS:
 case GASKET_IOCTL_RESET:
 case GASKET_IOCTL_NUMBER_PAGE_TABLES:
 case GASKET_IOCTL_PAGE_TABLE_SIZE:
 case GASKET_IOCTL_SIMPLE_PAGE_TABLE_SIZE:

  return 1;
 default:
  gasket_log_warn(gasket_dev, "Unknown ioctl: 0x%x", cmd);
  return 0;
 }
}
EXPORT_SYMBOL(argos_overseer_gasket_ioctl_has_permission);

int argos_subcontainer_gasket_ioctl_has_permission(
 struct argos_common_device_data *device_data, uint cmd, ulong arg)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;
 struct gasket_interrupt_eventfd interrupt_data;
 struct gasket_page_table_ioctl page_table_data;
 struct queue_ctx *queue_ctx;




 int is_master = (gasket_dev->ownership.is_owned &&
    gasket_dev->ownership.owner == current->tgid) ||
   capable(CAP_SYS_ADMIN);

 switch (cmd) {
 case GASKET_IOCTL_CLEAR_INTERRUPT_COUNTS:
 case GASKET_IOCTL_PARTITION_PAGE_TABLE:
 case GASKET_IOCTL_RESET:

  return 0;
 case GASKET_IOCTL_NUMBER_PAGE_TABLES:
 case GASKET_IOCTL_PAGE_TABLE_SIZE:
 case GASKET_IOCTL_SIMPLE_PAGE_TABLE_SIZE:

  return 1;
 case GASKET_IOCTL_SET_EVENTFD:




  if (copy_from_user(&interrupt_data, (void __user *)arg,
   sizeof(interrupt_data)))
   return -EFAULT;

  if (interrupt_data.interrupt >=
   device_data->gasket_driver_desc->num_interrupts)
   return -EINVAL;

  if (interrupt_data.interrupt <
      device_data->device_desc->queue_ctx_count) {
   queue_ctx = &device_data->queue_ctxs[
     interrupt_data.interrupt];
   if (!queue_ctx->reserved)
    return 0;
   return is_master || queue_ctx->owner == current->tgid;
  }





  return -EPERM;

 case GASKET_IOCTL_CLEAR_EVENTFD:
  if (arg >= device_data->gasket_driver_desc->num_interrupts)
   return -EINVAL;

  queue_ctx = &device_data->queue_ctxs[arg];
  if (!queue_ctx->reserved)
   return 0;

  return is_master || queue_ctx->owner == current->tgid;
 case GASKET_IOCTL_MAP_BUFFER:
 case GASKET_IOCTL_UNMAP_BUFFER:

  if (copy_from_user(&page_table_data, (void __user *)arg,
   sizeof(page_table_data)))
   return -EFAULT;

  if (page_table_data.page_table_index >=
   device_data->device_desc->queue_ctx_count)
   return -EINVAL;

  queue_ctx = &device_data->queue_ctxs[
   page_table_data.page_table_index];

  if (!queue_ctx->reserved)
   return 0;
  return is_master || queue_ctx->owner == current->tgid;

 default:
  gasket_log_warn(gasket_dev, "Unknown ioctl: 0x%x", cmd);
  return 0;
 }
}
EXPORT_SYMBOL(argos_subcontainer_gasket_ioctl_has_permission);

int argos_overseer_argos_ioctl_has_permission(
 struct argos_common_device_data *device_data, uint cmd, ulong arg)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;

 switch (cmd) {
 case ARGOS_IOCTL_ALLOCATE_QUEUE_CTX:
 case ARGOS_IOCTL_DEALLOCATE_QUEUE_CTX:
 case ARGOS_IOCTL_ENABLE_QUEUE_CTX:
 case ARGOS_IOCTL_DISABLE_QUEUE_CTX:
 case ARGOS_IOCTL_SUBCONTAINER_ALLOCATE_QUEUE_CTX:
 case ARGOS_IOCTL_ALLOCATE_DIRECT_MAPPING:
 case ARGOS_IOCTL_DEALLOCATE_DIRECT_MAPPING:

  return 0;
 case ARGOS_IOCTL_PROCESS_IS_MASTER:
 case ARGOS_IOCTL_SET_PRIORITY_ALGORITHM:
 case ARGOS_IOCTL_OVERSEER_RESERVE_RESOURCES:
 case ARGOS_IOCTL_OVERSEER_SET_MODE:

  return 1;
 default:
  gasket_log_warn(gasket_dev, "Unknown ioctl: 0x%x", cmd);
  return 0;
 }
}
EXPORT_SYMBOL(argos_overseer_argos_ioctl_has_permission);

int argos_subcontainer_argos_ioctl_has_permission(
 struct argos_common_device_data *device_data, uint cmd, ulong arg)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;

 switch (cmd) {
 case ARGOS_IOCTL_ALLOCATE_QUEUE_CTX:
  gasket_log_warn(gasket_dev,
   "ARGOS_IOCTL_SUBCONTAINER_ALLOCATE_QUEUE_CTX must be used when in a subcontainer.");
 case ARGOS_IOCTL_SET_PRIORITY_ALGORITHM:
 case ARGOS_IOCTL_OVERSEER_RESERVE_RESOURCES:
 case ARGOS_IOCTL_OVERSEER_SET_MODE:

  return 0;
 case ARGOS_IOCTL_PROCESS_IS_MASTER:
 case ARGOS_IOCTL_DEALLOCATE_QUEUE_CTX:
 case ARGOS_IOCTL_ENABLE_QUEUE_CTX:
 case ARGOS_IOCTL_DISABLE_QUEUE_CTX:
 case ARGOS_IOCTL_SUBCONTAINER_ALLOCATE_QUEUE_CTX:
 case ARGOS_IOCTL_ALLOCATE_DIRECT_MAPPING:
 case ARGOS_IOCTL_DEALLOCATE_DIRECT_MAPPING:

  return 1;
 default:
  gasket_log_warn(gasket_dev, "Unknown ioctl: 0x%x", cmd);
  return 0;
 }
}
EXPORT_SYMBOL(argos_subcontainer_argos_ioctl_has_permission);
# 580 "./drivers/char/argos/argos_overseer.c"
static ssize_t sysfs_show_subcontainers(
 struct argos_common_device_data *device_data, char *buf)
{
 int i, j;
 bool first_queue;
 ssize_t bytes_written = 0;
 struct argos_common_device_data *sub_device_data;
 struct queue_ctx *queue_ctx;
 char *cur_buf = buf;





 if (!device_data->subcontainers[0].registered) {
  for (i = 0; i < GASKET_MAX_CLONES; i++) {
   bytes_written += scnprintf(
     cur_buf + bytes_written,
     PAGE_SIZE - bytes_written,
     "n/a\n");
  }
  return bytes_written;
 }

 for (i = 0; i < GASKET_MAX_CLONES; i++) {

  bytes_written += scnprintf(
    cur_buf + bytes_written,
    PAGE_SIZE - bytes_written,
    "%s ",
    dev_name(&device_data->subcontainers[i]
      .gasket_dev.accel_dev.dev));

  sub_device_data = device_data->subcontainers[i]
    .gasket_dev.cb_data;

  bytes_written += scnprintf(
    cur_buf + bytes_written,
    PAGE_SIZE - bytes_written,
    "%d/%d ",
    sub_device_data->allocated_chunks,
    sub_device_data->reserved_chunks);
  first_queue = true;
  for (j = 0; j < device_data->device_desc->queue_ctx_count;
       j++) {
   queue_ctx = &sub_device_data->queue_ctxs[j];
   if (queue_ctx->reserved) {
    bytes_written += scnprintf(
      cur_buf + bytes_written,
      PAGE_SIZE - bytes_written,
      first_queue ? "%d" : ",%d",
      queue_ctx->index);
    first_queue = false;
   }
  }
  cur_buf[bytes_written++] = '\n';
 }
 cur_buf[bytes_written] = 0;

 return bytes_written;
}
# 651 "./drivers/char/argos/argos_overseer.c"
static ssize_t sysfs_show_mem_alloc(
  struct argos_common_device_data *device_data, int mem_alloc,
  char *buf)
{
 memcpy(buf,
        &device_data->chunk_map[mem_alloc * ARGOS_CHUNKS_PER_SYSFS_NODE],
        ARGOS_CHUNKS_PER_SYSFS_NODE);
 buf[ARGOS_CHUNKS_PER_SYSFS_NODE] = '\n';
 return ARGOS_CHUNKS_PER_SYSFS_NODE + 1;
}

#if ARGOS_CHUNKS_PER_SYSFS_NODE + 1 >= PAGE_SIZE
#error "Overseer chunks per node exceeds what sysfs supports on this system"
#endif

static ssize_t sysfs_show(struct device *device, struct device_attribute *attr,
     char *buf)
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
 case ATTR_SUBCONTAINERS:
  return sysfs_show_subcontainers(device_data, buf);
 case ATTR_SUBCONTAINER_ID:
  if (gasket_dev->parent)
   return scnprintf(buf, PAGE_SIZE, "%d\n",
     gasket_dev->clone_index);
  else {
   gasket_log_warn(gasket_dev,
    "subcontainer_id sysfs node read on master!");
   return scnprintf(buf, PAGE_SIZE, "-1\n");
  }
 case ATTR_SUBCONTAINER_MEMORY_0:
  return sysfs_show_mem_alloc(device_data, 0, buf);
 case ATTR_SUBCONTAINER_MEMORY_1:
  return sysfs_show_mem_alloc(device_data, 1, buf);
 default:
  gasket_log_error(
   gasket_dev, "Unknown attribute: %s", attr->attr.name);
  return 0;
 }
}

int argos_overseer_ioctl_dispatch(
 struct argos_common_device_data *device_data, uint cmd, ulong arg)
{
 struct gasket_dev *gasket_dev = device_data->gasket_dev;

 switch (cmd) {
 case ARGOS_IOCTL_OVERSEER_RESERVE_RESOURCES:
  gasket_log_debug(gasket_dev,
   "Recvd ioctl ARGOS_IOCTL_OVERSEER_RESERVE_RESOURCES");
  return argos_overseer_reserve_resources(device_data, arg);
 case ARGOS_IOCTL_OVERSEER_SET_MODE:
  gasket_log_debug(gasket_dev,
   "Recvd ioctl ARGOS_IOCTL_OVERSEER_SET_MODE");
  return argos_overseer_set_mode(device_data, arg);
 default:
  return -EOPNOTSUPP;
 }
}
