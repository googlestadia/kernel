/*
 * Copyright (C) 2019 Google LLC.
 */
# 2 "./include/linux/google/argos.h"
#ifndef __LINUX_ARGOS_H__
#define __LINUX_ARGOS_H__ 

#include <linux/ioctl.h>
#include <linux/types.h>



#define ARGOS_NAME_MAX_LENGTH 64


#define ARGOS_DRAM_CHUNK_BYTES (2ull << 20)


#define ARGOS_PRIORITY_ALGORITHM_ROUND_ROBIN 0
#define ARGOS_PRIORITY_ALGORITHM_WATERFALL 1


#define ARGOS_RESET_HARD 0

#define ARGOS_RESET_REINIT 1





#define ARGOS_CHUNKS_PER_SYSFS_NODE 2048



struct argos_queue_ctx_config {

 char name[ARGOS_NAME_MAX_LENGTH];


 int priority;


 int dram_chunks;


 int index;
};







struct argos_subcontainer_queue_ctx_config {

 char name[ARGOS_NAME_MAX_LENGTH];


 int priority;



 bool use_chunk_bitmap;






 __u32 num_chunks;






 __u8 *chunk_bitmap;





 int index;
};


struct argos_priority_algorithm_config {
 int priority;
 int algorithm;
};





struct argos_overseer_reservation_request {
# 103 "./include/linux/google/argos.h"
 __u32 subcontainer_index;


 __u32 num_queues;


 __u32 num_chunks;
};

struct argos_direct_mapping_request {




 int queue_index;






 int bar;






 __u64 base;





 __u64 size;







 int prot;
# 155 "./include/linux/google/argos.h"
 __u32 peer_rid_address;
 __u32 peer_rid_mask;






 __u64 mmap_offset;
};


#define ARGOS_IOCTL_BASE 0xDE


#define ARGOS_IOCTL_PROCESS_IS_MASTER _IOR(ARGOS_IOCTL_BASE, 0, int *)





#define ARGOS_IOCTL_ALLOCATE_QUEUE_CTX \
 _IOW(ARGOS_IOCTL_BASE, 1, struct argos_queue_ctx_config)






#define ARGOS_IOCTL_DEALLOCATE_QUEUE_CTX _IOW(ARGOS_IOCTL_BASE, 2, char *)







#define ARGOS_IOCTL_ENABLE_QUEUE_CTX \
 _IOWR(ARGOS_IOCTL_BASE, 3, struct argos_queue_ctx_config)






#define ARGOS_IOCTL_DISABLE_QUEUE_CTX _IOW(ARGOS_IOCTL_BASE, 4, char *)





#define ARGOS_IOCTL_SET_PRIORITY_ALGORITHM \
 _IOW(ARGOS_IOCTL_BASE, 5, struct argos_priority_algorithm_config)
# 219 "./include/linux/google/argos.h"
#define ARGOS_IOCTL_OVERSEER_RESERVE_RESOURCES \
 _IOW(ARGOS_IOCTL_BASE, 6, struct argos_overseer_reservation_request)






#define ARGOS_MODE_NORMAL 0
#define ARGOS_MODE_OVERSEER 1
#define ARGOS_IOCTL_OVERSEER_SET_MODE \
 _IOW(ARGOS_IOCTL_BASE, 7, int)
# 239 "./include/linux/google/argos.h"
#define ARGOS_IOCTL_SUBCONTAINER_ALLOCATE_QUEUE_CTX \
 _IOW(ARGOS_IOCTL_BASE, 8, struct argos_subcontainer_queue_ctx_config)






#define ARGOS_IOCTL_ALLOCATE_DIRECT_MAPPING \
 _IOWR(ARGOS_IOCTL_BASE, 9, struct argos_direct_mapping_request)
#define ARGOS_IOCTL_DEALLOCATE_DIRECT_MAPPING \
 _IOW(ARGOS_IOCTL_BASE, 10, struct argos_direct_mapping_request)

#endif
