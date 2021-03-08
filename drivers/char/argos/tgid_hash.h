/*
 * Copyright (C) 2021 Google LLC.
 */
# 2 "./drivers/char/argos/tgid_hash.h"
#ifndef __TGID_HASH_H__
#define __TGID_HASH_H__ 

#include <linux/hashtable.h>
#include <linux/kref.h>
#include <linux/mutex.h>
#include <linux/types.h>

struct argos_common_device_data;






struct tgid_hash_entry {

 pid_t tgid;


 struct argos_common_device_data *device_data;


 struct kref open_count;


 struct hlist_node hlist_node;
};
# 38 "./drivers/char/argos/tgid_hash.h"
struct tgid_hash_entry *tgid_hash_find(
 struct argos_common_device_data *device_data, pid_t tgid);
# 49 "./drivers/char/argos/tgid_hash.h"
struct tgid_hash_entry *tgid_hash_get_or_create(
 struct argos_common_device_data *device_data, pid_t tgid);
# 59 "./drivers/char/argos/tgid_hash.h"
int tgid_hash_put(struct argos_common_device_data *device_data, pid_t tgid);
# 68 "./drivers/char/argos/tgid_hash.h"
void tgid_hash_entry_free(struct tgid_hash_entry *hash_entry);







void tgid_hash_entry_kill_worker(struct tgid_hash_entry *hash_entry);

#endif
