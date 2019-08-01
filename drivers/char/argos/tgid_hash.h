/*
 * Copyright (C) 2019 Google LLC.
 */
# 2 "./drivers/char/argos/tgid_hash.h"
#ifndef __TGID_HASH_H__
#define __TGID_HASH_H__ 

#include <linux/hashtable.h>
#include <linux/kref.h>
#include <linux/mutex.h>
#include <linux/types.h>

#define TGID_HASH_MAX_QUEUE_COUNT 128
#define TGID_HASH_CHAR_BIT 8

struct argos_common_device_data;






struct tgid_hash_entry {

 pid_t tgid;


 struct argos_common_device_data *device_data;


 struct kref open_count;





 char enabled_queues[TGID_HASH_MAX_QUEUE_COUNT / TGID_HASH_CHAR_BIT];


 struct hlist_node hlist_node;
};
# 47 "./drivers/char/argos/tgid_hash.h"
struct tgid_hash_entry *tgid_hash_find(
 struct argos_common_device_data *device_data, pid_t tgid);
# 58 "./drivers/char/argos/tgid_hash.h"
struct tgid_hash_entry *tgid_hash_get_or_create(
 struct argos_common_device_data *device_data, pid_t tgid);
# 68 "./drivers/char/argos/tgid_hash.h"
int tgid_hash_put(struct argos_common_device_data *device_data, pid_t tgid);
# 77 "./drivers/char/argos/tgid_hash.h"
void tgid_hash_entry_free(struct tgid_hash_entry *hash_entry);







void tgid_hash_entry_kill_worker(struct tgid_hash_entry *hash_entry);
# 98 "./drivers/char/argos/tgid_hash.h"
int tgid_hash_queue_add(
 struct argos_common_device_data *device_data, int queue_idx);
# 113 "./drivers/char/argos/tgid_hash.h"
int tgid_hash_queue_remove(
 struct argos_common_device_data *device_data, int queue_idx,
 pid_t tgid);
# 128 "./drivers/char/argos/tgid_hash.h"
bool tgid_hash_entry_queue_is_enabled(
 struct tgid_hash_entry *hash_entry, int queue_idx);
# 139 "./drivers/char/argos/tgid_hash.h"
void tgid_hash_entry_clear_queues(struct tgid_hash_entry *hash_entry);

#endif
