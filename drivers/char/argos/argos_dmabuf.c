/*
 * Copyright (C) 2022 Google LLC.
 */
# 1 "./drivers/char/argos/argos_dmabuf.c"
#include <linux/dma-buf.h>
#include <linux/fs.h>
#include <linux/scatterlist.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 7, 19)
#include <linux/dma-resv.h>
#endif

#include "../../gasket/gasket_core.h"
#include "../../gasket/gasket_dmabuf.h"
#include "../../gasket/gasket_logging.h"
#include "argos_device.h"
#include "argos_ioctl.h"
#include "argos_queue.h"

static void argos_dma_buf_release(
 struct gasket_dma_buf_device_data *dbuf_device_data);

struct dma_buf *argos_create_dma_buf(
 struct argos_filp_data *filp_data,
 struct argos_direct_mapping_dma_buf_request *request)
{
 struct argos_common_device_data *device_data = filp_data->device_data;
 int ret;
 struct dma_buf *dbuf;
 struct argos_dma_buf_object *argos_dbuf;
# 43 "./drivers/char/argos/argos_dmabuf.c"
 ret = argos_get_direct_mapping_mmap_offset(
  filp_data, &request->direct_mapping);
 if (ret) {
  gasket_log_error(device_data->gasket_dev,
   "failed to get mmap_offset for dma_buf backing storage: %d",
   ret);
  return ERR_PTR(ret);
 }

 if ((request->offset + request->size) > request->direct_mapping.size) {
  gasket_log_error(device_data->gasket_dev,
   "Invalid offset and size (0x%llx, 0x%llx) for dma_buf backing storage.",
   request->offset, request->size);
  return ERR_PTR(-EINVAL);
 }

 argos_dbuf = kzalloc(sizeof(*argos_dbuf), GFP_KERNEL);
 if (!argos_dbuf)
  return ERR_PTR(-ENOMEM);

 INIT_LIST_HEAD(&argos_dbuf->list);
 argos_dbuf->request = *request;
 argos_dbuf->filp_data = filp_data;
 argos_dbuf->dbuf_device_data.release_cb = argos_dma_buf_release;

 dbuf = gasket_create_mmap_dma_buf(device_data->gasket_dev,
  (request->offset + request->direct_mapping.mmap_offset),
  request->size, request->flags, &argos_dbuf->dbuf_device_data);
 if (IS_ERR(dbuf)) {
  kfree(argos_dbuf);
  return dbuf;
 }

 argos_dbuf->dbuf = dbuf;
 ret = argos_add_dma_buf_to_direct_mapping(argos_dbuf);
 if (ret < 0) {
  gasket_log_error(device_data->gasket_dev,
   "failed to add dma_buf to direct mapping: %d", ret);
  dma_buf_put(dbuf);
  return ERR_PTR(ret);
 }

 return dbuf;
}
EXPORT_SYMBOL(argos_create_dma_buf);

void argos_dma_buf_move_notify(struct list_head *dma_buf)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 0)
 struct argos_dma_buf_object *argos_dbuf =
  list_entry(dma_buf, struct argos_dma_buf_object, list);
 dma_resv_lock(argos_dbuf->dbuf->resv, NULL);
 dma_buf_move_notify(argos_dbuf->dbuf);
 dma_resv_unlock(argos_dbuf->dbuf->resv);
#endif
}
EXPORT_SYMBOL(argos_dma_buf_move_notify);







static void argos_dma_buf_release(
 struct gasket_dma_buf_device_data *dbuf_device_data)
{
 struct argos_dma_buf_object *argos_dbuf = container_of(dbuf_device_data,
  struct argos_dma_buf_object, dbuf_device_data);
 if (!list_empty(&argos_dbuf->list))
  argos_remove_dma_buf_from_direct_mapping(argos_dbuf);

 kfree(argos_dbuf);
}
