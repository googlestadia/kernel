/*
 * Copyright (C) 2019 Google LLC.
 */
# 2 "./drivers/char/argos/argos_queue.h"
#ifndef __ARGOS_COMMON_H__
#define __ARGOS_COMMON_H__ 

#include "argos_types.h"

#define ARGOS_SHARED_VERSION "1"
# 19 "./drivers/char/argos/argos_queue.h"
int argos_find_free_queue_ctx(
 struct argos_common_device_data *device_data);
# 30 "./drivers/char/argos/argos_queue.h"
int argos_lookup_queue_ctx(struct argos_common_device_data *device_data,
 const char *ctx_id, struct queue_ctx **queue_ctx_ptr);
# 40 "./drivers/char/argos/argos_queue.h"
int argos_allocate_queue_ctx(
 struct argos_common_device_data *device_data,
 struct argos_subcontainer_queue_ctx_config *config);
# 53 "./drivers/char/argos/argos_queue.h"
int argos_deallocate_queue_ctx(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx);
# 66 "./drivers/char/argos/argos_queue.h"
int argos_enable_queue_ctx(
 struct argos_common_device_data *device_data,
 const char *name, struct argos_queue_ctx_config *config);
# 80 "./drivers/char/argos/argos_queue.h"
int argos_disable_queue_ctx(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx);
# 91 "./drivers/char/argos/argos_queue.h"
void argos_cleanup_hash_entry(struct kref *ref);
# 101 "./drivers/char/argos/argos_queue.h"
int argos_dram_request_evaluate_response(
  struct gasket_dev *gasket_dev, struct queue_ctx *queue_ctx);
# 111 "./drivers/char/argos/argos_queue.h"
int argos_clear_firmware_queue_status(
 struct argos_common_device_data *device_data, int queue_idx);
# 124 "./drivers/char/argos/argos_queue.h"
int argos_common_allocate_queue_ctx_callback(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx,
 const struct argos_subcontainer_queue_ctx_config *config);
# 139 "./drivers/char/argos/argos_queue.h"
int argos_allocate_direct_mapping(
 struct argos_common_device_data *device_data,
 struct argos_direct_mapping_request *request);
# 153 "./drivers/char/argos/argos_queue.h"
int argos_deallocate_direct_mapping(
 struct argos_common_device_data *device_data,
 const struct argos_direct_mapping_request *request);
# 175 "./drivers/char/argos/argos_queue.h"
int argos_get_direct_mappings_for_bar(
 struct argos_common_device_data *device_data,
 int bar_index, int *num_mappings,
 struct gasket_mappable_region *mappable_regions);
# 195 "./drivers/char/argos/argos_queue.h"
int argos_get_direct_mappable_regions(
 struct argos_common_device_data *device_data,
 int bar_index, struct gasket_mappable_region **mappable_regions,
 int *num_mappable_regions);
# 208 "./drivers/char/argos/argos_queue.h"
int argos_common_deallocate_queue_ctx_callback(
 struct argos_common_device_data *device_data,
 struct queue_ctx *queue_ctx);
# 220 "./drivers/char/argos/argos_queue.h"
bool argos_should_map_queue(
 struct argos_common_device_data *device_data, int queue_idx);

int argos_disable_and_deallocate_all_queues(
 struct argos_common_device_data *device_data);

#endif
