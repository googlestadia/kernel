#ifndef AMDGPU_BACKPORT_KCL_AMDGPU_H
#define AMDGPU_BACKPORT_KCL_AMDGPU_H

#include <drm/drm_vblank.h>
#include <amdgpu.h>

#if defined(HAVE_VGA_USE_UNSIGNED_INT_PIPE)
static inline u32 kcl_amdgpu_get_vblank_counter_kms(struct drm_device *dev, unsigned int crtc)
#else
static inline u32 kcl_amdgpu_get_vblank_counter_kms(struct drm_device *dev, int crtc)
#endif
{
	return amdgpu_get_vblank_counter_kms(dev, crtc);
}

#if defined(HAVE_VGA_USE_UNSIGNED_INT_PIPE)
static inline int kcl_amdgpu_enable_vblank_kms(struct drm_device *dev, unsigned int crtc)
#else
static inline int kcl_amdgpu_enable_vblank_kms(struct drm_device *dev, int crtc)
#endif
{
	return amdgpu_enable_vblank_kms(dev, crtc);
}

#if defined(HAVE_VGA_USE_UNSIGNED_INT_PIPE)
static inline void kcl_amdgpu_disable_vblank_kms(struct drm_device *dev, unsigned int crtc)
#else
static inline void kcl_amdgpu_disable_vblank_kms(struct drm_device *dev, int crtc)
#endif
{
	return amdgpu_disable_vblank_kms(dev, crtc);
}

#if defined(HAVE_GET_SCANOUT_POSITION_RETURN_BOOL)
static inline bool kcl_amdgpu_get_crtc_scanout_position(struct drm_device *dev, unsigned int pipe,
				 bool in_vblank_irq, int *vpos, int *hpos,
				 ktime_t *stime, ktime_t *etime,
				 const struct drm_display_mode *mode)
{
	return !!amdgpu_display_get_crtc_scanoutpos(dev, pipe, in_vblank_irq, vpos, hpos, stime, etime, mode);
}
#elif defined(HAVE_VGA_USE_UNSIGNED_INT_PIPE)
static inline int kcl_amdgpu_get_crtc_scanout_position(struct drm_device *dev, unsigned int crtc,
				   unsigned int flags, int *vpos, int *hpos,
				   ktime_t *stime, ktime_t *etime,
				   const struct drm_display_mode *mode)
{
	return amdgpu_display_get_crtc_scanoutpos(dev, crtc, flags, vpos, hpos, stime, etime, mode);
}
#elif defined(HAVE_GET_SCANOUT_POSITION_HAS_DRM_DISPLAY_MODE_ARG)
static inline int kcl_amdgpu_get_crtc_scanout_position(struct drm_device *dev, int crtc,
					   unsigned int flags,
					   int *vpos, int *hpos,
					   ktime_t *stime, ktime_t *etime,
					   const struct drm_display_mode *mode)
{
	return amdgpu_display_get_crtc_scanoutpos(dev, crtc, flags, vpos, hpos, stime, etime, mode);
}
#elif defined(HAVE_GET_SCANOUT_POSITION_HAS_TIMESTAMP_ARG)
static inline int kcl_amdgpu_get_crtc_scanout_position(struct drm_device *dev, int crtc,
					   int *vpos, int *hpos, ktime_t *stime,
					   ktime_t *etime)
{
	return amdgpu_display_get_crtc_scanoutpos(dev, crtc, 0, vpos, hpos, stime, etime, NULL);
}
#else
static inline int kcl_amdgpu_get_crtc_scanout_position(struct drm_device *dev, int crtc,
					   int *vpos, int *hpos)
{
	return amdgpu_display_get_crtc_scanoutpos(dev, crtc, 0, vpos, hpos, NULL, NULL, NULL);
}
#endif

#endif /* AMDGPU_BACKPORT_KCL_AMDGPU_H */
