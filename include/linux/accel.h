/*
 * Copyright (C) 2019 Google LLC.
 */
# 4 "./include/linux/accel.h"
#ifndef __ACCEL_H__
#define __ACCEL_H__ 

#include <linux/module.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/statfs.h>


#define ACCEL_MAJOR 121
#define ACCEL_BLOCK_MAJOR 121

struct accel_dev;
# 43 "./include/linux/accel.h"
struct accel_dev {

 const char *accel_type;
 const char *chip_vendor;
 const char *chip_model;
 const char *chip_revision;
 unsigned int fw_version[4];
 const char *logic_vendor;
 const char *logic_model;
 const char *logic_revision;
 const char *logic_build_cl;
 const char *logic_build_time;
 const struct file_operations *fops;


 int id;
 struct device dev;
 struct cdev cdev;
 void (*release)(struct accel_dev *);


 const char *state;
};

static inline const char *accel_dev_name(const struct accel_dev *adev)
{
 return dev_name(&adev->dev);
}





static inline struct accel_dev *accel_dev_get(struct accel_dev *adev)
{
 if (adev)
  get_device(&adev->dev);
 return adev;
}
# 90 "./include/linux/accel.h"
static inline void accel_dev_put(struct accel_dev *adev)
{
 if (adev)
  put_device(&adev->dev);
}
# 107 "./include/linux/accel.h"
static inline void accel_dev_set_state(struct accel_dev *adev,
           const char *state)
{
 adev->state = state;
}

extern int accel_dev_init(struct accel_dev *dev, struct device *parent,
     void (*release)(struct accel_dev *));
extern int accel_dev_register(struct accel_dev *adev);
extern void accel_dev_unregister(struct accel_dev *adev);

extern struct accel_dev *accel_dev_get_by_devt(dev_t devt);
extern struct accel_dev *accel_dev_get_by_parent(struct device *parent);

#define to_accel_dev(dev) ((struct accel_dev *)dev_get_drvdata(dev))

#endif
