/*
 * Copyright (C) 2019 Google LLC.
 */
# 2 "./drivers/char/argos/argos_overseer.h"
#ifndef __ARGOS_OVERSEER_H__
#define __ARGOS_OVERSEER_H__ 

#include "../../gasket/gasket_core.h"
#include "argos_types.h"
# 21 "./drivers/char/argos/argos_overseer.h"
int argos_overseer_ioctl_dispatch(
 struct argos_common_device_data *device_data, uint cmd, ulong arg);






int argos_overseer_gasket_ioctl_has_permission(
 struct argos_common_device_data *device_data, uint cmd, ulong arg);





int argos_subcontainer_gasket_ioctl_has_permission(
 struct argos_common_device_data *device_data, uint cmd, ulong arg);






int argos_overseer_argos_ioctl_has_permission(
 struct argos_common_device_data *device_data, uint cmd, ulong arg);





int argos_subcontainer_argos_ioctl_has_permission(
 struct argos_common_device_data *device_data, uint cmd, ulong arg);

#endif
