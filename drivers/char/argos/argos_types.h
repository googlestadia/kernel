/*
 * Copyright (C) 2021 Google LLC.
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
#include "../../gasket/gasket_types.h"

struct argos_common_device_data;
struct argos_subcontainer_queue_ctx_config;






enum argos_security_level {



 ARGOS_SECURITY_LEVEL_ROOT,



 ARGOS_SECURITY_LEVEL_USER,
};
# 41 "./drivers/char/argos/argos_types.h"
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
# 120 "./drivers/char/argos/argos_types.h"
 struct mutex direct_mappings_mutex;


 struct list_head direct_mappings;
};


struct argos_overseer_subcontainer {

 struct gasket_dev gasket_dev;


 bool registered;
};


struct argos_mappable_regions {



 struct gasket_mappable_region global_region;




 ulong (*get_queue_start)(int queue_index);
 ulong (*get_queue_length)(int queue_index);




 int num_mappable_dram_regions;
 int num_mappable_debug_regions;
};




struct argos_device_desc {



 int queue_ctx_count;




 bool overseer_supported;





 bool bitmap_allocation_allowed;





 const struct gasket_sysfs_attribute *sysfs_attrs;




 int firmware_register_bar;


 int dram_bar;


 int debug_bar;





 ulong firmware_api_version_location;





 ulong firmware_control_enable_location;







 ulong (*kernel_queue_ddr_status_location)(int reg_index);





 ulong (*kernel_queue_ddr_control_location)(int reg_index);




 ulong (*kernel_queue_control_location)(int reg_index);





 ulong (*queue_control_control_location)(int reg_index);





 ulong (*queue_control_status_location)(int reg_index);




 ulong (*interrupt_control_control_location)(int reg_index);





 ulong (*interrupt_control_status_location)(int reg_index);






 bool priority_algorithm_config_supported;
 ulong priority_algorithm_config_location;





 ulong chip_reset_location;





 ulong max_ddr_chunks_per_ctx_location;





 ulong global_chip_state_location;




 ulong global_chip_ddr_state_location;





 ulong dram_chunk_bitmap_location;





 ulong firmware_primary_version_location;
 ulong firmware_secondary_version_location;
 ulong firmware_image_info_location;


 struct {





  ulong control_location;


  int count;





  ulong (*rid_address_location)(int reg_index);





  ulong (*rid_mask_location)(int reg_index);






  ulong (*base_addr_location)(int reg_index);


  ulong (*size_location)(int reg_index);





  ulong fault_location;


  ulong read_fault_address_location;
  ulong write_fault_address_location;


  ulong read_fault_rid_location;
  ulong write_fault_rid_location;


  ulong bar_base_addr[GASKET_NUM_BARS];
 } rid_filter;


 struct argos_mappable_regions mappable_regions;
 int (*get_bar_region_count)(
  int bar,
  enum argos_security_level security_level);
 const struct gasket_mappable_region *
 (*get_bar_regions)(
  int bar,
  enum argos_security_level security_level);
};
# 361 "./drivers/char/argos/argos_types.h"
struct argos_device_callbacks {
# 372 "./drivers/char/argos/argos_types.h"
 bool (*is_queue_ctx_failed_cb)(
  struct argos_common_device_data *device_data,
  struct queue_ctx *queue_ctx);
# 387 "./drivers/char/argos/argos_types.h"
 int (*allocate_queue_ctx_cb)(
  struct argos_common_device_data *device_data,
  struct queue_ctx *queue_ctx,
  const struct argos_subcontainer_queue_ctx_config *config);
# 405 "./drivers/char/argos/argos_types.h"
 int (*enable_queue_ctx_cb)(
  struct argos_common_device_data *device_data,
  struct queue_ctx *queue_ctx);
# 422 "./drivers/char/argos/argos_types.h"
 int (*disable_queue_ctx_cb)(
  struct argos_common_device_data *device_data,
  struct queue_ctx *queue_ctx,
  struct gasket_mappable_region *mappable_region);
# 437 "./drivers/char/argos/argos_types.h"
 int (*deallocate_queue_ctx_cb)(
  struct argos_common_device_data *device_data,
  struct queue_ctx *queue_ctx);
# 465 "./drivers/char/argos/argos_types.h"
 int (*allocate_direct_mapping_cb)(
  struct argos_common_device_data *device_data,
  struct queue_ctx *queue_ctx,
  struct direct_mapping *direct_mapping);
# 482 "./drivers/char/argos/argos_types.h"
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
# 517 "./drivers/char/argos/argos_types.h"
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
# 575 "./drivers/char/argos/argos_types.h"
 u8 *chunk_map;





 struct mutex rid_filter_lock;
# 590 "./drivers/char/argos/argos_types.h"
 u8 *rid_filter_assignments;


 void *chip_specific_data;
};


#endif
