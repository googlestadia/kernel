BACKPORT_OBJS := kcl_mmu_notifier.o

amdgpu-y += $(addprefix ../backport/,$(BACKPORT_OBJS))

ccflags-y += \
	-I$(FULL_AMD_PATH)/backport/include \
	-include $(FULL_AMD_PATH)/dkms/config/config.h \
	-include ../backport/backport.h
