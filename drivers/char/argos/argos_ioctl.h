/*
 * Copyright (C) 2019 Google LLC.
 */
# 2 "./drivers/char/argos/argos_ioctl.h"
#ifndef __ARGOS_IOCTL_H__
#define __ARGOS_IOCTL_H__ 

#include "argos_types.h"
# 26 "./drivers/char/argos/argos_ioctl.h"
int argos_queue_ioctl_dispatch(
 struct argos_common_device_data *device_data, uint cmd, ulong arg);
# 38 "./drivers/char/argos/argos_ioctl.h"
int argos_check_gasket_ioctl_permissions(
 struct file *filp, uint cmd, ulong arg);


#endif
