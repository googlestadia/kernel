/*
 * Copyright (C) 2019 Google LLC.
 */
# 2 "./drivers/char/argos/rid_filter.h"
#ifndef __RID_FILTER_H__
#define __RID_FILTER_H__ 

#include "argos_types.h"

#define ARGOS_RID_FILTER_FREE 0xFF
#define ARGOS_RID_FILTER_RESERVED 0xFE
# 17 "./drivers/char/argos/rid_filter.h"
int rid_filter_sysfs_setup(struct argos_common_device_data *device_data);
# 28 "./drivers/char/argos/rid_filter.h"
int rid_filter_disable_and_clear(
 struct argos_common_device_data *device_data);
# 41 "./drivers/char/argos/rid_filter.h"
int rid_filter_setup(struct argos_common_device_data *device_data);
# 65 "./drivers/char/argos/rid_filter.h"
int rid_filter_allocate(struct argos_common_device_data *device_data,
 u8 queue_idx, uint rid, uint rid_mask, int prot, int bar,
 ulong offset, ulong size);
# 81 "./drivers/char/argos/rid_filter.h"
int rid_filter_deallocate(struct argos_common_device_data *device_data,
 int idx, u8 queue_idx);


#endif
