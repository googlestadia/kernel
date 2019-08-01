/*
 * Copyright (C) 2019 Google LLC.
 */
# 1 "./drivers/gasket/gasket_ioctl.h"
#ifndef __GASKET_IOCTL_H__
#define __GASKET_IOCTL_H__ 

#include "gasket_core.h"
# 14 "./drivers/gasket/gasket_ioctl.h"
long gasket_handle_ioctl(struct file *filp, uint cmd, ulong arg);







long gasket_is_supported_ioctl(uint cmd);

#endif
