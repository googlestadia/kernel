/*
 * Copyright (C) 2022 Google LLC.
 */
# 1 "./drivers/char/argos/argos_overseer.h"

#ifndef __ARGOS_OVERSEER_H__
#define __ARGOS_OVERSEER_H__ 

#include "../../gasket/gasket_core.h"
#include "argos_types.h"
# 21 "./drivers/char/argos/argos_overseer.h"
int argos_overseer_ioctl_dispatch(
 struct argos_filp_data *filp_data, uint cmd, ulong arg);
# 31 "./drivers/char/argos/argos_overseer.h"
int argos_overseer_gasket_ioctl_has_permission(
 struct argos_filp_data *filp_data, uint cmd);
# 41 "./drivers/char/argos/argos_overseer.h"
int argos_subcontainer_gasket_ioctl_has_permission(
 struct argos_filp_data *filp_data, uint cmd);
# 51 "./drivers/char/argos/argos_overseer.h"
int argos_overseer_argos_ioctl_has_permission(
 struct argos_filp_data *filp_data, uint cmd);







int argos_subcontainer_argos_ioctl_has_permission(
 struct argos_filp_data *filp_data, uint cmd);
# 71 "./drivers/char/argos/argos_overseer.h"
bool argos_overseer_subcontainer_owns_all_parent_resources(
 struct argos_common_device_data *subcontainer,
 struct argos_common_device_data *parent);

#endif
