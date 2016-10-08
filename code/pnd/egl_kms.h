#ifndef __EGL_KMS_H__
#define __EGL_KMS_H__

#include <EGL/egl.h>
int kms_setup(const char *drm_device, const char *gbm_device, NativeDisplayType *native_display, EGLNativeWindowType *native_window);
int kms_post_swap(void);
int kms_teardown(void);

#endif
