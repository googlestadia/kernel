# Makefile for Argos drivers.

obj-$(CONFIG_ARGOS_SHARED)	+= argos_shared.o

argos_shared-objs		:= argos_device.o argos_ioctl.o argos_overseer.o argos_common.o argos_queue.o rid_filter.o tgid_hash.o

obj-$(CONFIG_VARGOS_DRIVER) 	+= vargos.o

vargos-objs                     := vargos_driver.o
