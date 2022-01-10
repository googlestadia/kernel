/*
 * Copyright (C) 2022 Google LLC.
 */
# 1 "./drivers/char/argos/argos_ioctl.h"

#ifndef __ARGOS_IOCTL_H__
#define __ARGOS_IOCTL_H__ 

#include "argos_types.h"
# 26 "./drivers/char/argos/argos_ioctl.h"
int argos_queue_ioctl_dispatch(
 struct argos_filp_data *filp_data, uint cmd, ulong arg);
# 40 "./drivers/char/argos/argos_ioctl.h"
int argos_check_gasket_ioctl_permissions(struct file *filp, uint cmd);


#endif
