/*
 * Copyright 2016 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <linux/module.h>

#include <drm/drm_drv.h>

#include "amdgpu.h"
#include <linux/debugfs.h>
static const char *autodump_query_list[MAX_AUTODUMP_NODE] = {"dummy", "hang", "name"};


bool amdgpu_virt_mmio_blocked(struct amdgpu_device *adev)
{
	/* By now all MMIO pages except mailbox are blocked */
	/* if blocking is enabled in hypervisor. Choose the */
	/* SCRATCH_REG0 to test. */
	return RREG32_NO_KIQ(0xc040) == 0xffffffff;
}

static bool autodump_is_booked_alive_locked(struct amdgpu_virt *virt)
{
       struct task_struct *ref;
       struct pid *pid;

       if (virt->autodump.task == NULL || virt->autodump.pid == 0)
               return false;

       pid = find_get_pid(virt->autodump.pid);
       ref = pid_task(pid, PIDTYPE_PID);

       return ref == virt->autodump.task;
}

/* only the registered task can do write with val 0 or 2,
 * new task can do val 1 if previous registered task died
 */
static int autodump_action_write(void *data, uint64_t val)
{
       struct amdgpu_device *adev = data;
       struct amdgpu_virt *virt = &adev->virt;
       int ret = -EPERM;

       mutex_lock(&adev->virt.dump_mutex);
       switch (val) {
       case 1:
               /* unbook first if previous one died */
               if (!autodump_is_booked_alive_locked(virt)) {
                       if (virt->autodump.task) {
                               put_task_struct(virt->autodump.task);
                       }
                       memset(&virt->autodump, 0 ,sizeof(virt->autodump));
                       goto book_new;
               }

               /* disallow new booker as long as previous not died */
               goto quit;

book_new:
               get_task_struct(current);
               virt->autodump.task = current;
               virt->autodump.tgid = current->tgid;
               virt->autodump.pid = current->pid;
               get_task_comm(virt->autodump.process_name, current->group_leader);
               printk("autodump:%s book autodump --> tgid=%u, pid=%u\n", virt->autodump.process_name, current->tgid, current->pid);
               ret = 0;
               break;
       case 0:
               if (current->pid != virt->autodump.pid ||
                   current != virt->autodump.task)
                       goto quit;

               printk("autodump:%s unbook autodump --> tgid=%u, pid=%u\n", virt->autodump.process_name, current->tgid, current->pid);
               if (virt->autodump.task)
                       put_task_struct(virt->autodump.task);

               memset(&virt->autodump, 0 ,sizeof(virt->autodump));
               ret = 0;
               break;
       case 2:
               if (current->pid != virt->autodump.pid ||
                   current != virt->autodump.task)
                       goto quit;

               complete(&virt->dump_cpl);
               printk("autodump:%s unblock GPU recovery --> tgid=%u, pid=%u\n", virt->autodump.process_name, current->tgid, current->pid);
               ret = 0;
               break;
       default:
               ret = -EINVAL;
               break;
       }

quit:
       mutex_unlock(&adev->virt.dump_mutex);
       return ret;
}

DEFINE_SIMPLE_ATTRIBUTE(autodump_action_fops,
                               NULL,
                               autodump_action_write, "%llu\n");

static ssize_t autodump_query_read(struct file *f, char __user *buf,
                                       size_t size, loff_t *pos)
{
       char tmp[16] = {0};
       struct amdgpu_device *adev = f->private_data;

       mutex_lock(&adev->virt.dump_mutex);
       switch (adev->virt.autodump.query) {
               case 1:
                       snprintf(tmp, sizeof(tmp), "%s", adev->virt.autodump.ring_name==NULL? "no":"yes");
                       break;
               case 2:
                       snprintf(tmp, sizeof(tmp), "%s", adev->virt.autodump.ring_name);
                       break;
               default:
                       mutex_unlock(&adev->virt.dump_mutex);
                       return -EINVAL;
       }

       mutex_unlock(&adev->virt.dump_mutex);
       return simple_read_from_buffer(buf, size, pos, tmp, sizeof(tmp));
}

static ssize_t autodump_query_write(struct file *f, const char __user *buf,
                                        size_t size, loff_t *pos)
{
       int i;
       ssize_t ret;
       char tmp[16] = {0};
       unsigned long done;
       struct amdgpu_device *adev = f->private_data;

       ret = simple_write_to_buffer(tmp, sizeof(tmp), pos, buf, size);

       i = 1;
       while (i < sizeof(autodump_query_list)/sizeof(autodump_query_list[0])) {
               if (!memcmp(tmp, autodump_query_list[i], strlen(autodump_query_list[i])))
                       break;
               i++;
       }

       if (i < sizeof(autodump_query_list)/sizeof(autodump_query_list[0])) {
               mutex_lock(&adev->virt.dump_mutex);
               adev->virt.autodump.query = i;
               mutex_unlock(&adev->virt.dump_mutex);
               return ret;
       }

       return -EINVAL;
}

static const struct file_operations autodump_query_fops = {
       .owner = THIS_MODULE,
       .open  = simple_open,
       .read = autodump_query_read,
       .write = autodump_query_write,
       .llseek = default_llseek,
};

int amdgpu_virt_create_debugs(struct amdgpu_device *adev)
{
       struct dentry *entry;
       struct amdgpu_virt *virt = &adev->virt;

       init_completion(&adev->virt.dump_cpl);

       entry = debugfs_create_file("autodump_action",
                       S_IWUSR,
                       adev->ddev->primary->debugfs_root,
                       adev, &autodump_action_fops);

       virt->dump_dentries[1] = entry;

       entry = debugfs_create_file("autodump_query",
                       S_IRUSR | S_IWUSR,
                       adev->ddev->primary->debugfs_root,
                       adev, &autodump_query_fops);

       virt->dump_dentries[2] = entry;
err:
       return -EFAULT;
}

int amdgpu_virt_notify_booked(struct amdgpu_device *adev, struct amdgpu_job *job)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
       siginfo_t info;
#else
       struct kernel_siginfo info;
#endif
       int ret = -ENOENT;

       mutex_lock(&adev->virt.dump_mutex);
       if (adev->virt.autodump.task == NULL ||
               adev->virt.autodump.pid == 0)
               goto quit;

       if (!autodump_is_booked_alive_locked(&adev->virt)) {
               /* looks the pid is used on another task
                * which indiates the virt.task may already
                * killed
                */
               put_task_struct(adev->virt.autodump.task);
               memset(&adev->virt.autodump, 0 ,sizeof(adev->virt.autodump));
               goto quit;
       }

       memset(&info, 0, sizeof(info));
       info.si_signo = SIGUSR1;
#if 0
       info.si_errno = 0;
       info.si_code = 0;
       info.si_addr = 0;
       info.si_addr_lsb = 0;
#endif

       adev->virt.autodump.ring_name = job->base.sched->name;

       ret = send_sig_info(info.si_signo, &info, adev->virt.autodump.task);
quit:
       mutex_unlock(&adev->virt.dump_mutex);
       return ret;
}

int amdgpu_virt_wait_dump(struct amdgpu_device *adev, unsigned long tmo)
{
       int ret;

       ret = wait_for_completion_interruptible_timeout(&adev->virt.dump_cpl, tmo);

       if (ret == 0) { /* time out and dump tool still not finish its dump*/
               pr_err("autodump: timeout before dump finished, move on to gpu recovery\n");
               return -ETIMEDOUT;
       } else if (ret < 0) {
               pr_err("autodump: get interrupted during dump waiting\n");
               return ret;
       }
       return 0;
}

void amdgpu_virt_init_setting(struct amdgpu_device *adev)
{
	/* enable virtual display */
	if (adev->mode_info.num_crtc == 0)
		adev->mode_info.num_crtc = 1;
	adev->enable_virtual_display = true;
	adev->ddev->driver->driver_features &= ~DRIVER_ATOMIC;
	adev->cg_flags = 0;
	adev->pg_flags = 0;
}

void amdgpu_virt_kiq_reg_write_reg_wait(struct amdgpu_device *adev,
					uint32_t reg0, uint32_t reg1,
					uint32_t ref, uint32_t mask)
{
	struct amdgpu_kiq *kiq = &adev->gfx.kiq;
	struct amdgpu_ring *ring = &kiq->ring;
	signed long r, cnt = 0;
	unsigned long flags;
	uint32_t seq;

	spin_lock_irqsave(&kiq->ring_lock, flags);
	amdgpu_ring_alloc(ring, 32);
	amdgpu_ring_emit_reg_write_reg_wait(ring, reg0, reg1,
					    ref, mask);
	r = amdgpu_fence_emit_polling(ring, &seq, MAX_KIQ_REG_WAIT);
	if (r)
		goto failed_undo;

	amdgpu_ring_commit(ring);
	spin_unlock_irqrestore(&kiq->ring_lock, flags);

	r = amdgpu_fence_wait_polling(ring, seq, MAX_KIQ_REG_WAIT);

	/* don't wait anymore for IRQ context */
	if (r < 1 && in_interrupt())
		goto failed_kiq;

	might_sleep();
	while (r < 1 && cnt++ < MAX_KIQ_REG_TRY) {

		msleep(MAX_KIQ_REG_BAILOUT_INTERVAL);
		r = amdgpu_fence_wait_polling(ring, seq, MAX_KIQ_REG_WAIT);
	}

	if (cnt > MAX_KIQ_REG_TRY)
		goto failed_kiq;

	return;

failed_undo:
	amdgpu_ring_undo(ring);
	spin_unlock_irqrestore(&kiq->ring_lock, flags);
failed_kiq:
	pr_err("failed to write reg %x wait reg %x\n", reg0, reg1);
}

/**
 * amdgpu_virt_request_full_gpu() - request full gpu access
 * @amdgpu:	amdgpu device.
 * @init:	is driver init time.
 * When start to init/fini driver, first need to request full gpu access.
 * Return: Zero if request success, otherwise will return error.
 */
int amdgpu_virt_request_full_gpu(struct amdgpu_device *adev, bool init)
{
	struct amdgpu_virt *virt = &adev->virt;
	int r;

	if (virt->ops && virt->ops->req_full_gpu) {
		r = virt->ops->req_full_gpu(adev, init);
		if (r)
			return r;

		adev->virt.caps &= ~AMDGPU_SRIOV_CAPS_RUNTIME;
	}

	return 0;
}

/**
 * amdgpu_virt_release_full_gpu() - release full gpu access
 * @amdgpu:	amdgpu device.
 * @init:	is driver init time.
 * When finishing driver init/fini, need to release full gpu access.
 * Return: Zero if release success, otherwise will returen error.
 */
int amdgpu_virt_release_full_gpu(struct amdgpu_device *adev, bool init)
{
	struct amdgpu_virt *virt = &adev->virt;
	int r;

	if (virt->ops && virt->ops->rel_full_gpu) {
		r = virt->ops->rel_full_gpu(adev, init);
		if (r)
			return r;

		adev->virt.caps |= AMDGPU_SRIOV_CAPS_RUNTIME;
	}
	return 0;
}

/**
 * amdgpu_virt_reset_gpu() - reset gpu
 * @amdgpu:	amdgpu device.
 * Send reset command to GPU hypervisor to reset GPU that VM is using
 * Return: Zero if reset success, otherwise will return error.
 */
int amdgpu_virt_reset_gpu(struct amdgpu_device *adev)
{
	struct amdgpu_virt *virt = &adev->virt;
	int r;

	if (virt->ops && virt->ops->reset_gpu) {
		r = virt->ops->reset_gpu(adev);
		if (r)
			return r;

		adev->virt.caps &= ~AMDGPU_SRIOV_CAPS_RUNTIME;
	}

	return 0;
}

void amdgpu_virt_request_init_data(struct amdgpu_device *adev)
{
	struct amdgpu_virt *virt = &adev->virt;

	if (virt->ops && virt->ops->req_init_data)
		virt->ops->req_init_data(adev);

	if (adev->virt.req_init_data_ver > 0)
		DRM_INFO("host supports REQ_INIT_DATA handshake\n");
	else
		DRM_WARN("host doesn't support REQ_INIT_DATA handshake\n");
}

/**
 * amdgpu_virt_wait_reset() - wait for reset gpu completed
 * @amdgpu:	amdgpu device.
 * Wait for GPU reset completed.
 * Return: Zero if reset success, otherwise will return error.
 */
int amdgpu_virt_wait_reset(struct amdgpu_device *adev)
{
	struct amdgpu_virt *virt = &adev->virt;

	if (!virt->ops || !virt->ops->wait_reset)
		return -EINVAL;

	return virt->ops->wait_reset(adev);
}

/**
 * amdgpu_virt_alloc_mm_table() - alloc memory for mm table
 * @amdgpu:	amdgpu device.
 * MM table is used by UVD and VCE for its initialization
 * Return: Zero if allocate success.
 */
int amdgpu_virt_alloc_mm_table(struct amdgpu_device *adev)
{
	int r;

	if (!amdgpu_sriov_vf(adev) || adev->virt.mm_table.gpu_addr)
		return 0;

	r = amdgpu_bo_create_kernel(adev, PAGE_SIZE, PAGE_SIZE,
				    AMDGPU_GEM_DOMAIN_VRAM,
				    &adev->virt.mm_table.bo,
				    &adev->virt.mm_table.gpu_addr,
				    (void *)&adev->virt.mm_table.cpu_addr);
	if (r) {
		DRM_ERROR("failed to alloc mm table and error = %d.\n", r);
		return r;
	}

	memset((void *)adev->virt.mm_table.cpu_addr, 0, PAGE_SIZE);
	DRM_INFO("MM table gpu addr = 0x%llx, cpu addr = %p.\n",
		 adev->virt.mm_table.gpu_addr,
		 adev->virt.mm_table.cpu_addr);
	return 0;
}

/**
 * amdgpu_virt_free_mm_table() - free mm table memory
 * @amdgpu:	amdgpu device.
 * Free MM table memory
 */
void amdgpu_virt_free_mm_table(struct amdgpu_device *adev)
{
	if (!amdgpu_sriov_vf(adev) || !adev->virt.mm_table.gpu_addr)
		return;

	amdgpu_bo_free_kernel(&adev->virt.mm_table.bo,
			      &adev->virt.mm_table.gpu_addr,
			      (void *)&adev->virt.mm_table.cpu_addr);
	adev->virt.mm_table.gpu_addr = 0;
}


int amdgpu_virt_fw_reserve_get_checksum(void *obj,
					unsigned long obj_size,
					unsigned int key,
					unsigned int chksum)
{
	unsigned int ret = key;
	unsigned long i = 0;
	unsigned char *pos;

	pos = (char *)obj;
	/* calculate checksum */
	for (i = 0; i < obj_size; ++i)
		ret += *(pos + i);
	/* minus the chksum itself */
	pos = (char *)&chksum;
	for (i = 0; i < sizeof(chksum); ++i)
		ret -= *(pos + i);
	return ret;
}

void amdgpu_virt_init_data_exchange(struct amdgpu_device *adev)
{
	uint32_t pf2vf_size = 0;
	uint32_t checksum = 0;
	uint32_t checkval;
	char *str;

	adev->virt.fw_reserve.p_pf2vf = NULL;
	adev->virt.fw_reserve.p_vf2pf = NULL;

	if (adev->fw_vram_usage.va != NULL) {
		adev->virt.fw_reserve.p_pf2vf =
			(struct amd_sriov_msg_pf2vf_info_header *)(
			adev->fw_vram_usage.va + AMDGIM_DATAEXCHANGE_OFFSET);
		AMDGPU_FW_VRAM_PF2VF_READ(adev, header.size, &pf2vf_size);
		AMDGPU_FW_VRAM_PF2VF_READ(adev, checksum, &checksum);
		AMDGPU_FW_VRAM_PF2VF_READ(adev, feature_flags, &adev->virt.gim_feature);

		/* pf2vf message must be in 4K */
		if (pf2vf_size > 0 && pf2vf_size < 4096) {
			checkval = amdgpu_virt_fw_reserve_get_checksum(
				adev->virt.fw_reserve.p_pf2vf, pf2vf_size,
				adev->virt.fw_reserve.checksum_key, checksum);
			if (checkval == checksum) {
				adev->virt.fw_reserve.p_vf2pf =
					((void *)adev->virt.fw_reserve.p_pf2vf +
					pf2vf_size);
				memset((void *)adev->virt.fw_reserve.p_vf2pf, 0,
					sizeof(amdgim_vf2pf_info));
				AMDGPU_FW_VRAM_VF2PF_WRITE(adev, header.version,
					AMDGPU_FW_VRAM_VF2PF_VER);
				AMDGPU_FW_VRAM_VF2PF_WRITE(adev, header.size,
					sizeof(amdgim_vf2pf_info));
				AMDGPU_FW_VRAM_VF2PF_READ(adev, driver_version,
					&str);
#ifdef MODULE
				if (THIS_MODULE->version != NULL)
					strcpy(str, THIS_MODULE->version);
				else
#endif
					strcpy(str, "N/A");
				AMDGPU_FW_VRAM_VF2PF_WRITE(adev, driver_cert,
					0);
				AMDGPU_FW_VRAM_VF2PF_WRITE(adev, checksum,
					amdgpu_virt_fw_reserve_get_checksum(
					adev->virt.fw_reserve.p_vf2pf,
					pf2vf_size,
					adev->virt.fw_reserve.checksum_key, 0));
			}
		}
	}
}

void amdgpu_detect_virtualization(struct amdgpu_device *adev)
{
	uint32_t reg;

	switch (adev->asic_type) {
	case CHIP_TONGA:
	case CHIP_FIJI:
		reg = RREG32(mmBIF_IOV_FUNC_IDENTIFIER);
		break;
	case CHIP_VEGA10:
	case CHIP_VEGA20:
	case CHIP_NAVI10:
	case CHIP_NAVI12:
	case CHIP_ARCTURUS:
		reg = RREG32(mmRCC_IOV_FUNC_IDENTIFIER);
		break;
	default: /* other chip doesn't support SRIOV */
		reg = 0;
		break;
	}

	if (reg & 1)
		adev->virt.caps |= AMDGPU_SRIOV_CAPS_IS_VF;

	if (reg & 0x80000000)
		adev->virt.caps |= AMDGPU_SRIOV_CAPS_ENABLE_IOV;

	if (!reg) {
		if (is_virtual_machine())	/* passthrough mode exclus sriov mod */
			adev->virt.caps |= AMDGPU_PASSTHROUGH_MODE;
	}
}
