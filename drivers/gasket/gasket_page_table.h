/*
 * Copyright (C) 2019 Google LLC.
 */
# 10 "./drivers/gasket/gasket_page_table.h"
#ifndef __GASKET_PAGE_TABLE_H__
#define __GASKET_PAGE_TABLE_H__ 

#include <linux/pci.h>
#include <linux/types.h>

#include "gasket_constants.h"
#include "gasket_core.h"





struct gasket_page_table;
# 39 "./drivers/gasket/gasket_page_table.h"
int gasket_page_table_init(struct gasket_page_table **pg_tbl,
 const struct gasket_bar_data *bar_data,
 const struct gasket_page_table_config *page_table_config,
 struct gasket_dev *gasket_dev);
# 55 "./drivers/gasket/gasket_page_table.h"
void gasket_page_table_cleanup(struct gasket_page_table *page_table);
# 70 "./drivers/gasket/gasket_page_table.h"
int gasket_page_table_partition(
 struct gasket_page_table *page_table, uint num_simple_entries);
# 88 "./drivers/gasket/gasket_page_table.h"
int gasket_page_table_map(struct gasket_page_table *page_table, ulong host_addr,
 ulong dev_addr, uint num_pages);
# 102 "./drivers/gasket/gasket_page_table.h"
int gasket_page_table_unmap(
 struct gasket_page_table *page_table, ulong dev_addr, uint num_pages);





void gasket_page_table_unmap_all(struct gasket_page_table *page_table);






void gasket_page_table_reset(struct gasket_page_table *page_table);
# 125 "./drivers/gasket/gasket_page_table.h"
void gasket_page_table_garbage_collect(struct gasket_page_table *page_table);
# 140 "./drivers/gasket/gasket_page_table.h"
int gasket_page_table_are_addrs_bad(struct gasket_page_table *page_table,
 ulong host_addr, ulong dev_addr, ulong bytes);
# 155 "./drivers/gasket/gasket_page_table.h"
int gasket_page_table_is_dev_addr_bad(
 struct gasket_page_table *page_table, ulong dev_addr, ulong bytes);





uint gasket_page_table_max_size(struct gasket_page_table *page_table);





uint gasket_page_table_num_entries(struct gasket_page_table *page_table);





uint gasket_page_table_num_simple_entries(struct gasket_page_table *page_table);





uint gasket_page_table_num_extended_entries(
 struct gasket_page_table *page_table);





uint gasket_page_table_num_active_pages(struct gasket_page_table *page_table);





int gasket_page_table_system_status(struct gasket_page_table *page_table);

#endif
