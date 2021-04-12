/*
 * Copyright (C) 2021 Google LLC.
 */
# 1 "./drivers/gasket/gasket_dmabuf.h"
#ifndef __GASKET_DMABUF_H__
#define __GASKET_DMABUF_H__ 

#include "gasket_types.h"

struct gasket_dma_buf_device_data;







struct gasket_dma_buf_device_data {

 void (*release_cb)(struct gasket_dma_buf_device_data *device_data);
};
# 27 "./drivers/gasket/gasket_dmabuf.h"
struct dma_buf *gasket_create_mmap_dma_buf(struct gasket_dev *gasket_dev,
 u64 mmap_offset, size_t buf_size, int flags,
 struct gasket_dma_buf_device_data *device_data);
#endif
