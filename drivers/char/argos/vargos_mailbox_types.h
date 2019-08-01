/*
 * Copyright (C) 2019 Google LLC.
 */
# 5 "./drivers/char/argos/vargos_mailbox_types.h"
#ifndef __VARGOS_MAILBOX_H__
#define __VARGOS_MAILBOX_H__ 

#include <linux/types.h>

#ifdef __cplusplus
extern "C" {
#endif





#define VARGOS_MAILBOX_VERSION 4

enum {
 VARGOS_MAILBOX_CONTROL_IDLE = 0,
 VARGOS_MAILBOX_CONTROL_START = 1,
};

enum {
 VARGOS_MAILBOX_STATE_IDLE = 0,
 VARGOS_MAILBOX_STATE_RUNNING = 1,
 VARGOS_MAILBOX_STATE_DONE = 2,
 VARGOS_MAILBOX_STATE_ERROR = 0xffffffff,
};

struct vargos_mailbox_command {

 enum command {
  VARGOS_MAILBOX_COMMAND_NOP = 0,





  VARGOS_MAILBOX_COMMAND_ALLOCATE = 1,





  VARGOS_MAILBOX_COMMAND_DEALLOCATE = 2,





  VARGOS_MAILBOX_COMMAND_ENABLE = 3,





  VARGOS_MAILBOX_COMMAND_DISABLE = 4,
# 68 "./drivers/char/argos/vargos_mailbox_types.h"
  VARGOS_MAILBOX_COMMAND_ALLOCATE_DIRECT_MAPPING = 9,





  VARGOS_MAILBOX_COMMAND_DEALLOCATE_DIRECT_MAPPING = 10,





  VARGOS_MAILBOX_COMMAND_MAP_BUFFER = 0x1008,





  VARGOS_MAILBOX_COMMAND_UNMAP_BUFFER = 0x1009,

  VARGOS_MAILBOX_COMMAND_MAX = 0xffffffff
 } type;


 int virtual_queue_index;

 union {
  struct {

   int priority;





   __u32 num_chunks;
  } allocate;

  struct {




   __u64 dma_addr;





   __u64 dev_addr;




   __u32 num_bytes;
  } map_unmap_buffer;





  struct {



   int bar;


   __u64 base;


   __u64 size;




   int prot;






   __u32 peer_rid_address;
   __u32 peer_rid_mask;
  } direct_mapping;
 };
};






#define VARGOS_MAILBOX_MAX_COMMAND_ENTRIES \
 (4096 / sizeof(struct vargos_mailbox_command))

struct vargos_mailbox_response {

 __u32 status_code;

 struct {







  __u64 bar_offset;
 } direct_mapping;
};

#ifdef __cpp_static_assert
static_assert(sizeof(vargos_mailbox_response) <= sizeof(vargos_mailbox_command),
       "The size of vargos_mailbox_response must be at most the size of vargos_mailbox_command!");
#endif

#ifdef __cplusplus
}
#endif

#endif
