/*
 * Copyright (C) 2021 Google LLC.
 */
# 1 "./drivers/char/argos/argos_dmabuf.h"
#ifndef __ARGOS_DMABUF_H__
#define __ARGOS_DMABUF_H__ 

#include "argos_types.h"


struct argos_dma_buf_object;
# 17 "./drivers/char/argos/argos_dmabuf.h"
struct dma_buf *argos_create_dma_buf(
 struct argos_common_device_data *device_data,
 struct argos_direct_mapping_dma_buf_request *request);







void argos_dma_buf_move_notify(struct list_head *dma_buf);
#endif
