/*
 * Copyright (C) 2021 Google LLC.
 */
# 1 "./drivers/gasket/gasket_constants.h"
#ifndef __GASKET_CONSTANTS_H__
#define __GASKET_CONSTANTS_H__ 

#define GASKET_FRAMEWORK_VERSION "1.1.1"




#define GASKET_FRAMEWORK_DESC_MAX 4


#define GASKET_DEV_MAX 256


#define GASKET_NUM_BARS 6


#define GASKET_MAX_NUM_PAGE_TABLES 128


#define GASKET_NAME_MAX 32


enum gasket_status {



 GASKET_STATUS_DEAD = 0,




 GASKET_STATUS_LAMED,


 GASKET_STATUS_ALIVE,





 GASKET_STATUS_DRIVER_EXIT,
};

#endif
