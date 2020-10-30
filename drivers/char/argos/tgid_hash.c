/*
 * Copyright (C) 2020 Google LLC.
 */
# 1 "./drivers/char/argos/tgid_hash.c"
#include <linux/sched.h>
#include <linux/sched/signal.h>

#include "../../gasket/gasket_core.h"
#include "../../gasket/gasket_logging.h"
#include "argos_queue.h"
#include "argos_types.h"
#include "tgid_hash.h"

struct tgid_hash_entry *tgid_hash_find(
 struct argos_common_device_data *device_data, pid_t tgid)
{
 struct tgid_hash_entry *hash_entry = NULL;
 struct tgid_hash_entry *found_entry = NULL;
 int found = 0;

 hash_for_each_possible(device_data->tgid_to_open_count, hash_entry,
  hlist_node, tgid) {
  if (hash_entry->tgid == tgid) {
   if (found) {
    gasket_log_error(
     device_data->gasket_dev,
     "TGID %d present < 1x in TGID table!",
     tgid);
   }

   found = 1;
   found_entry = hash_entry;




  }
 }

 return found_entry;
}

struct tgid_hash_entry *tgid_hash_get_or_create(
 struct argos_common_device_data *device_data, pid_t tgid)
{
 struct tgid_hash_entry *hash_entry = NULL;

 hash_entry = tgid_hash_find(device_data, tgid);
 if (hash_entry) {
  kref_get(&hash_entry->open_count);
  return hash_entry;
 }

 hash_entry = kzalloc(sizeof(struct tgid_hash_entry), GFP_KERNEL);
 if (!hash_entry)
  return NULL;

 hash_entry->tgid = current->tgid;
 hash_entry->device_data = device_data;
 kref_init(&hash_entry->open_count);

 hash_add(device_data->tgid_to_open_count,
  &hash_entry->hlist_node, hash_entry->tgid);

 return hash_entry;
}

int tgid_hash_put(struct argos_common_device_data *device_data, pid_t tgid)
{
 struct tgid_hash_entry *hash_entry;

 hash_entry = tgid_hash_find(device_data, tgid);
 if (!hash_entry) {
  gasket_log_info(
   device_data->gasket_dev,
   "TGID/file entry not found for %d",
   current->tgid);
  return -EINVAL;
 }

 kref_put(&hash_entry->open_count, argos_cleanup_hash_entry);

 return 0;
}

void tgid_hash_entry_free(struct tgid_hash_entry *hash_entry)
{
 hash_del(&hash_entry->hlist_node);
 kfree(hash_entry);
}

void tgid_hash_entry_kill_worker(struct tgid_hash_entry *hash_entry)
{

 if (hash_entry->tgid != current->tgid)
  kill_pid(find_vpid(hash_entry->tgid), SIGKILL, 1);
}
