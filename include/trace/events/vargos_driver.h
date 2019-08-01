/*
 * Copyright (C) 2019 Google LLC.
 */
# 9 "./include/trace/events/vargos_driver.h"
#undef TRACE_SYSTEM
#define TRACE_SYSTEM vargos_driver

#if !defined(_TRACE_VARGOS_DRIVER_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_VARGOS_DRIVER_H 

#include <linux/tracepoint.h>

TRACE_EVENT(vargos_driver_hypercall,
 TP_PROTO(
  const char *name,
  uint vargos_command_type,
  int vargos_virtual_queue,
  unsigned long jiffies_elapsed,
  int status
 ),

 TP_ARGS(
  name,
  vargos_command_type,
  vargos_virtual_queue,
  jiffies_elapsed,
  status
 ),

 TP_STRUCT__entry(
  __string(device_name, name)
  __field(uint, vargos_command_type)
  __field(int, vargos_virtual_queue)
  __field(unsigned long, jiffies_elapsed)
  __field(int, status)
 ),

 TP_fast_assign(
  __assign_str(device_name, name);
  __entry->vargos_command_type = vargos_command_type;
  __entry->vargos_virtual_queue = vargos_virtual_queue;
  __entry->jiffies_elapsed = jiffies_elapsed;
  __entry->status = status;
 ),

 TP_printk("device %s, processed command %d for queue %u and took %ums with status %d",
  __get_str(device_name), __entry->vargos_command_type,
  __entry->vargos_virtual_queue,
  jiffies_to_msecs(__entry->jiffies_elapsed), __entry->status)
);
#endif


#include <trace/define_trace.h>
