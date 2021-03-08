/*
 * Copyright (C) 2021 Google LLC.
 */
# 2 "./drivers/char/argos/rid_filter.h"
#ifndef __RID_FILTER_H__
#define __RID_FILTER_H__ 

#include "argos_types.h"

#define ARGOS_RID_FILTER_FREE 0xFF
#define ARGOS_RID_FILTER_RESERVED 0xFE


#define ARGOS_RID_FILTER_NAME_OFFSET_WRAPPER(chip,register_name,field_name) \
 get_##chip##_##register_name##_##field_name##_offset
# 33 "./drivers/char/argos/rid_filter.h"
#define ARGOS_RID_FILTER_OFFSET_WRAPPER(chip,register_name,field_name) \
static ulong \
ARGOS_RID_FILTER_NAME_OFFSET_WRAPPER(chip, register_name, field_name) \
(int reg_index) \
{ \
 const struct chip##_##register_name##_offsets *offset = \
  chip##_##register_name##_offsets_get(0); \
 switch (reg_index) { \
 case 0: return offset->field_name##_0; \
 case 1: return offset->field_name##_1; \
 case 2: return offset->field_name##_2; \
 case 3: return offset->field_name##_3; \
 case 4: return offset->field_name##_4; \
 case 5: return offset->field_name##_5; \
 case 6: return offset->field_name##_6; \
 case 7: return offset->field_name##_7; \
 case 8: return offset->field_name##_8; \
 case 9: return offset->field_name##_9; \
 case 10: return offset->field_name##_10; \
 case 11: return offset->field_name##_11; \
 case 12: return offset->field_name##_12; \
 case 13: return offset->field_name##_13; \
 case 14: return offset->field_name##_14; \
 case 15: return offset->field_name##_15; \
 case 16: return offset->field_name##_16; \
 case 17: return offset->field_name##_17; \
 case 18: return offset->field_name##_18; \
 case 19: return offset->field_name##_19; \
 case 20: return offset->field_name##_20; \
 case 21: return offset->field_name##_21; \
 case 22: return offset->field_name##_22; \
 case 23: return offset->field_name##_23; \
 default: return 0; \
 } \
 return 0; \
}
# 77 "./drivers/char/argos/rid_filter.h"
int rid_filter_sysfs_setup(struct argos_common_device_data *device_data);
# 88 "./drivers/char/argos/rid_filter.h"
int rid_filter_disable_and_clear(
 struct argos_common_device_data *device_data);
# 101 "./drivers/char/argos/rid_filter.h"
int rid_filter_setup(struct argos_common_device_data *device_data);
# 113 "./drivers/char/argos/rid_filter.h"
int rid_filter_direct_mapping_setup(struct argos_common_device_data *device_data,
                                    struct direct_mapping *direct_mapping,
                                    u8 queue_idx, ulong bar_offset);
# 126 "./drivers/char/argos/rid_filter.h"
int rid_filter_direct_mapping_teardown(struct argos_common_device_data *device_data,
                                       struct direct_mapping *direct_mapping,
                                       u8 queue_idx);

#endif
