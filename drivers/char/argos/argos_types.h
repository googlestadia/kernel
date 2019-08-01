/*
 * Copyright (C) 2019 Google LLC.
 */
# 2 "./drivers/char/argos/argos_types.h"
#ifndef __ARGOS_TYPES_H__
#define __ARGOS_TYPES_H__ 

#include <linux/google/argos.h>
#include <linux/hashtable.h>
#include <linux/list.h>
#include <linux/mutex.h>

#include "../../gasket/gasket_core.h"
#include "../../gasket/gasket_page_table.h"

struct argos_common_device_data;
struct argos_subcontainer_queue_ctx_config;
# 26 "./drivers/char/argos/argos_types.h"
struct direct_mapping {





 struct list_head list;






 struct argos_direct_mapping_request request;






 int rid_filter_window;






 struct gasket_mappable_region mappable_region;
};


struct queue_ctx {

 int index;


 char id[ARGOS_NAME_MAX_LENGTH + 1];


 int priority;


 int dram_chunks;


 pid_t owner;






 struct gasket_page_table *pg_tbl;


 struct mutex mutex;


 bool allocated;







 bool reserved;
# 103 "./drivers/char/argos/argos_types.h"
 struct mutex direct_mappings_mutex;


 struct list_head direct_mappings;
};


struct argos_overseer_subcontainer {

 struct gasket_dev gasket_dev;


 bool registered;
};
# 127 "./drivers/char/argos/argos_types.h"
struct argos_register_field_desc {



 ulong location;







 ulong (*get_location)(int reg_index);





 int shift;





 u64 mask;
};


struct argos_mappable_regions {



 const struct gasket_mappable_region global_region;




 const struct gasket_mappable_region master_region;




 ulong (*get_queue_start)(int queue_index);
 ulong (*get_queue_length)(int queue_index);




 int num_mappable_dram_regions;
 int num_mappable_debug_regions;
};




struct argos_device_desc {



 int queue_ctx_count;


 int failed_codec_interrupt;




 bool overseer_supported;





 const struct gasket_sysfs_attribute *sysfs_attrs;




 int firmware_register_bar;


 int dram_bar;


 int debug_bar;





 struct argos_register_field_desc firmware_api_version_register;





 struct argos_register_field_desc is_fake_hardware_register;





 struct argos_register_field_desc queue_ddr_status_value;





 struct argos_register_field_desc queue_ddr_status_pending;





 struct argos_register_field_desc queue_ddr_status_current_chunks;




 struct argos_register_field_desc queue_ddr_control;






 ulong bitmap_based_request;
 ulong count_based_request;





 struct argos_register_field_desc queue_ddr_control_change_requested;




 struct argos_register_field_desc queue_control_priority_value;





 struct argos_register_field_desc control_control;





 struct argos_register_field_desc control_status_enabled;




 struct argos_register_field_desc interrupt_control_control;





 struct argos_register_field_desc interrupt_control_status;







 struct argos_register_field_desc priority_algorithm_config;




 struct argos_register_field_desc global_ddr_state_available_chunks;





 struct argos_register_field_desc dram_chunk_bitmap;


 struct {





  struct argos_register_field_desc control;


  int count;





  struct argos_register_field_desc rid_address;





  struct argos_register_field_desc rid_mask;





  struct argos_register_field_desc read_valid;





  struct argos_register_field_desc write_valid;





  struct argos_register_field_desc base_addr;


  struct argos_register_field_desc size;





  struct argos_register_field_desc read_fault;
  struct argos_register_field_desc write_fault;


  struct argos_register_field_desc read_fault_clear;
  struct argos_register_field_desc write_fault_clear;


  struct argos_register_field_desc read_fault_address;
  struct argos_register_field_desc write_fault_address;


  struct argos_register_field_desc read_fault_rid;
  struct argos_register_field_desc write_fault_rid;


  ulong bar_base_addr[GASKET_NUM_BARS];
 } rid_filter;


 const struct argos_mappable_regions mappable_regions;
};
# 388 "./drivers/char/argos/argos_types.h"
struct argos_device_callbacks {
# 399 "./drivers/char/argos/argos_types.h"
 bool (*is_queue_ctx_failed_cb)(
  struct argos_common_device_data *device_data,
  struct queue_ctx *queue_ctx);
# 414 "./drivers/char/argos/argos_types.h"
 int (*allocate_queue_ctx_cb)(
  struct argos_common_device_data *device_data,
  struct queue_ctx *queue_ctx,
  const struct argos_subcontainer_queue_ctx_config *config);
# 432 "./drivers/char/argos/argos_types.h"
 int (*enable_queue_ctx_cb)(
  struct argos_common_device_data *device_data,
  struct queue_ctx *queue_ctx);
# 449 "./drivers/char/argos/argos_types.h"
 int (*disable_queue_ctx_cb)(
  struct argos_common_device_data *device_data,
  struct queue_ctx *queue_ctx,
  struct gasket_mappable_region *mappable_region);
# 464 "./drivers/char/argos/argos_types.h"
 int (*deallocate_queue_ctx_cb)(
  struct argos_common_device_data *device_data,
  struct queue_ctx *queue_ctx);
# 492 "./drivers/char/argos/argos_types.h"
 int (*allocate_direct_mapping_cb)(
  struct argos_common_device_data *device_data,
  struct queue_ctx *queue_ctx,
  struct direct_mapping *direct_mapping);
# 509 "./drivers/char/argos/argos_types.h"
 int (*deallocate_direct_mapping_cb)(
  struct argos_common_device_data *device_data,
  struct queue_ctx *queue_ctx,
  struct direct_mapping *direct_mapping);
};


struct argos_common_device_data {

 const struct gasket_driver_desc *gasket_driver_desc;


 const struct argos_device_desc *device_desc;


 struct gasket_dev *gasket_dev;


 struct argos_device_callbacks *argos_cb;


 struct queue_ctx *queue_ctxs;



 DECLARE_HASHTABLE(tgid_to_open_count, 5);
# 544 "./drivers/char/argos/argos_types.h"
 struct mutex mutex;





 int is_real_hardware;





 int timeout_scaling;


 struct mutex mem_alloc_mutex;


 int mode;


 struct argos_overseer_subcontainer subcontainers[
   GASKET_MAX_CLONES];


 int total_chunks;





 int max_chunks_per_queue_ctx;





 int reserved_chunks;


 int allocated_chunks;
# 602 "./drivers/char/argos/argos_types.h"
 u8 *chunk_map;





 struct mutex rid_filter_lock;
# 617 "./drivers/char/argos/argos_types.h"
 u8 *rid_filter_assignments;
};





enum ddr_status_value {
 DDR_STATUS_VALUE_SUCCESS = 0,
 DDR_STATUS_VALUE_NOT_ENOUGH_AVAILABLE = 1,
 DDR_STATUS_VALUE_TOO_LARGE = 2,
 DDR_STATUS_VALUE_IN_PROGRESS = 3,
 DDR_STATUS_VALUE_QUEUE_NOT_DISABLED = 4,
 DDR_STATUS_VALUE_INVALID_REQUEST_TYPE = 5,
 DDR_STATUS_VALUE_CHUNK_ALREADY_RESERVED = 6,
};



#endif
