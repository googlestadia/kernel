/*
 * Copyright (C) 2021 Google LLC.
 */
# 10 "./drivers/gasket/gasket_interrupt.h"
#ifndef __GASKET_INTERRUPT_H__
#define __GASKET_INTERRUPT_H__ 

#include <linux/eventfd.h>
#include <linux/pci.h>

#include "gasket_core.h"


struct gasket_interrupt_data;
# 44 "./drivers/gasket/gasket_interrupt.h"
int legacy_gasket_interrupt_init(struct gasket_dev *gasket_dev,
 const char *name, const struct legacy_gasket_interrupt_desc *interrupts,
 int num_interrupts, int pack_width, int bar_index);
# 58 "./drivers/gasket/gasket_interrupt.h"
int legacy_gasket_interrupt_setup(struct gasket_dev *gasket_dev);
# 77 "./drivers/gasket/gasket_interrupt.h"
int gasket_interrupt_init(struct gasket_dev *gasket_dev,
 const char *name, const struct gasket_interrupt_desc *interrupts,
 int num_interrupts, int num_msix_interrupts);







void gasket_interrupt_cleanup(struct gasket_dev *gasket_dev);
# 97 "./drivers/gasket/gasket_interrupt.h"
int gasket_interrupt_reinit(struct gasket_dev *gasket_dev);







int gasket_interrupt_reset_counts(struct gasket_dev *gasket_dev);
# 121 "./drivers/gasket/gasket_interrupt.h"
int legacy_gasket_interrupt_set_eventfd(
 struct gasket_dev *gasket_dev, int interrupt, int event_fd);
# 133 "./drivers/gasket/gasket_interrupt.h"
int legacy_gasket_interrupt_clear_eventfd(
 struct gasket_dev *gasket_dev, int interrupt);
# 144 "./drivers/gasket/gasket_interrupt.h"
int gasket_interrupt_system_status(struct gasket_dev *gasket_dev);
# 155 "./drivers/gasket/gasket_interrupt.h"
struct eventfd_ctx **gasket_interrupt_get_eventfd_ctxs(
 struct gasket_interrupt_data *interrupt_data);
# 171 "./drivers/gasket/gasket_interrupt.h"
int gasket_interrupt_register_mapping(struct gasket_dev *gasket_dev,
 int interrupt, int event_fd, int bar_index, u64 reg);






int gasket_interrupt_unregister_mapping(struct gasket_dev *gasket_dev,
 int interrupt);

#endif
