/*
 * Copyright (C) 2021 Google LLC.
 */
# 3 "./drivers/gasket/gasket_types.h"
#ifndef PLATFORMS_ASIC_SW_KERNEL_COMMON_GASKET_GASKET_TYPES_H_
#define PLATFORMS_ASIC_SW_KERNEL_COMMON_GASKET_GASKET_TYPES_H_ 

#ifdef __KERNEL__

#include <linux/types.h>

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef unsigned long long uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef long long int64;

#else

#include <stddef.h>

#include "base/integral_types.h"

#endif

#endif
