subdir-ccflags-y += \
	-DDRM_VER=4 \
	-DDRM_PATCH=19 \
	-DDRM_SUB="0"

DKMS_INCLUDE_PREFIX = \
	-I$(src)/include \
	-I$(src)/include/drm \
	-I$(src)/include/uapi \
	-include $(src)/include/kcl/kcl_version.h \
	-include $(src)/include/rename_symbol.h

export OS_NAME OS_VERSION DKMS_INCLUDE_PREFIX

export CONFIG_HSA_AMD=m
export CONFIG_DRM_TTM=m
export CONFIG_DRM_AMDGPU=m
export CONFIG_CHASH=m
export CONFIG_DRM_SCHED=m
export BUILD_AS_DKMS=y
export CONFIG_DRM_AMDGPU_CIK=n
export CONFIG_DRM_AMDGPU_SI=n
export CONFIG_DRM_AMDGPU_USERPTR=y
export CONFIG_DRM_AMD_DC=y
export CONFIG_DRM_AMD_DC_DCN1_0=y
export CONFIG_DRM_AMD_DC_DCN1_01=y

subdir-ccflags-y += -DCONFIG_HSA_AMD
subdir-ccflags-y += -DBUILD_AS_DKMS
subdir-ccflags-y += -DCONFIG_DRM_AMDGPU_USERPTR
subdir-ccflags-y += -DCONFIG_DRM_AMD_DC
subdir-ccflags-y += -DCONFIG_DRM_AMD_DC_DCN1_0
subdir-ccflags-y += -DCONFIG_DRM_AMD_DC_DCN1_01

obj-m += scheduler/ amd/amdgpu/ ttm/ amd/amdkcl/
