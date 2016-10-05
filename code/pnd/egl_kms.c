#include "egl_kms.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

static struct {
	int fd;
	struct gbm_device *dev;
	struct gbm_surface *surface;
} gbm;

struct drm_ops {
	int (*import)(int fd, uint32_t handle);
};

static struct {
	int fd;
	drmModeModeInfo *mode;
	uint32_t crtc_id;
	uint32_t connector_id;

	const struct drm_ops *ops;
} drm;

struct drm_fb {
	struct gbm_bo *bo;
	uint32_t fb_id;
};

static struct {
	struct gbm_bo *bo;
} state;

static int init_drm(const char *path)
{
	static const char *modules[] = {
		"i915", "radeon", "nouveau", "vmwgfx", "omapdrm", "exynos", "msm", "imx-drm"
	};
	drmModeRes *resources;
	drmModeConnector *connector = NULL;
	drmModeEncoder *encoder = NULL;
	drmVersion *version;
	int i, area;

	if (!path) {
		for (i = 0; i < ARRAY_SIZE(modules); i++) {
			printf("trying to load module %s...", modules[i]);
			drm.fd = drmOpen(modules[i], NULL);
			if (drm.fd < 0) {
				printf("failed.\n");
			} else {
				printf("success.\n");
				break;
			}
		}
	} else {
		printf("opening %s for drm\n", path);
		drm.fd = open(path, O_RDWR);
	}

	if (drm.fd < 0) {
		printf("could not open drm device\n");
		return -1;
	}

	/* query driver version and setup driver-specific hooks */
	version = drmGetVersion(drm.fd);
	if (!version) {
		printf("could not query drm driver version\n");
		return -1;
	}

	drmFreeVersion(version);

	resources = drmModeGetResources(drm.fd);
	if (!resources) {
		printf("drmModeGetResources failed: %s\n", strerror(errno));
		return -1;
	}

	/* find a connected connector: */
	for (i = 0; i < resources->count_connectors; i++) {
		connector = drmModeGetConnector(drm.fd, resources->connectors[i]);
		if (connector->connection == DRM_MODE_CONNECTED) {
			/* it's connected, let's use this! */
			break;
		}
		drmModeFreeConnector(connector);
		connector = NULL;
	}

	if (!connector) {
		/* we could be fancy and listen for hotplug events and wait for
		 * a connector..
		 */
		printf("no connected connector!\n");
		return -1;
	}

	/* find highest resolution mode: */
	for (i = 0, area = 0; i < connector->count_modes; i++) {
		drmModeModeInfo *current_mode = &connector->modes[i];
		int current_area = current_mode->hdisplay * current_mode->vdisplay;
		if (current_area > area) {
			drm.mode = current_mode;
			area = current_area;
		}
	}

	if (!drm.mode) {
		printf("could not find mode!\n");
		return -1;
	}

	/* find encoder: */
	for (i = 0; i < resources->count_encoders; i++) {
		encoder = drmModeGetEncoder(drm.fd, resources->encoders[i]);
		if (encoder->encoder_id == connector->encoder_id)
			break;
		drmModeFreeEncoder(encoder);
		encoder = NULL;
	}

	if (!encoder) {
		printf("no encoder!\n");
		return -1;
	}

	drm.crtc_id = encoder->crtc_id;
	drm.connector_id = connector->connector_id;

	return 0;
}

static int init_gbm(const char *path)
{
	if (path) {
		printf("opening %s for gbm\n", path);

		gbm.fd = open(path, O_RDWR);
		if (gbm.fd < 0) {
			fprintf(stderr, "could not open gbm device\n");
			return -1;
		}
	} else {
		gbm.fd = drm.fd;
	}

	gbm.dev = gbm_create_device(gbm.fd);

	gbm.surface = gbm_surface_create(gbm.dev,
			drm.mode->hdisplay, drm.mode->vdisplay,
			GBM_FORMAT_XRGB8888,
			GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
	if (!gbm.surface) {
		printf("failed to create gbm surface\n");
		return -1;
	}

	return 0;
}

static void
drm_fb_destroy_callback(struct gbm_bo *bo, void *data)
{
	struct drm_fb *fb = data;
	struct gbm_device *gbm = gbm_bo_get_device(bo);

	if (fb->fb_id)
		drmModeRmFB(drm.fd, fb->fb_id);

	free(fb);
}

static struct drm_fb * drm_fb_get_from_bo(struct gbm_bo *bo)
{
	struct drm_fb *fb = gbm_bo_get_user_data(bo);
	uint32_t width, height, stride, handle;
	int ret;

	if (fb)
		return fb;

	fb = calloc(1, sizeof *fb);
	fb->bo = bo;

	width = gbm_bo_get_width(bo);
	height = gbm_bo_get_height(bo);
	stride = gbm_bo_get_stride(bo);
	handle = gbm_bo_get_handle(bo).u32;

	if (drm.fd != gbm.fd) {
		int fd;

		ret = drmPrimeHandleToFD(gbm.fd, handle, 0, &fd);
		if (ret) {
			printf("failed to export bo: %m\n");
			free(fb);
			return NULL;
		}

		ret = drmPrimeFDToHandle(drm.fd, fd, &handle);
		if (ret) {
			printf("failed to import bo: %m\n");
			close(fd);
			free(fb);
			return NULL;
		}

		close(fd);

		if (drm.ops && drm.ops->import) {
			ret = drm.ops->import(drm.fd, handle);
			if (ret < 0) {
				printf("failed to import handle: %s\n",
				       strerror(-ret));
				free(fb);
				return NULL;
			}
		}
	}

	ret = drmModeAddFB(drm.fd, width, height, 24, 32, stride, handle, &fb->fb_id);
	if (ret) {
		printf("failed to create fb: %s\n", strerror(errno));
		free(fb);
		return NULL;
	}

	gbm_bo_set_user_data(bo, fb, drm_fb_destroy_callback);

	return fb;
}

static void page_flip_handler(int fd, unsigned int frame,
		  unsigned int sec, unsigned int usec, void *data)
{
	int *waiting_for_flip = data;
	*waiting_for_flip = 0;
}

int kms_setup(const char *drm_device, const char *gbm_device, void **native_display)
{
	struct drm_fb *fb;
	int ret;

	if (drm_device && gbm_device) {
		/* if both devices are the same, open it once only */
		if (strcmp(drm_device, gbm_device) != 0)
			gbm_device = drm_device;
	}

	ret = init_drm(drm_device);
	if (ret) {
		printf("failed to initialize DRM\n");
		return ret;
	}

	ret = init_gbm(gbm_device);
	if (ret) {
		printf("failed to initialize GBM\n");
		return ret;
	}

	state.bo = gbm_surface_lock_front_buffer(gbm.surface);
	fb = drm_fb_get_from_bo(state.bo);

	/* set mode: */
	ret = drmModeSetCrtc(drm.fd, drm.crtc_id, fb->fb_id, 0, 0,
			&drm.connector_id, 1, drm.mode);
	if (ret) {
		printf("failed to set mode: %s\n", strerror(errno));
		return ret;
	}
	*native_display = gbm.dev;
	return 0;
}

int kms_post_swap(void)
{
	fd_set fds;
	struct gbm_bo *next_bo;
	int waiting_for_flip = 1;
	int ret;
	struct drm_fb *fb;
	drmEventContext evctx = {
		.version = DRM_EVENT_CONTEXT_VERSION,
		.page_flip_handler = page_flip_handler,
	};

	next_bo = gbm_surface_lock_front_buffer(gbm.surface);
	fb = drm_fb_get_from_bo(next_bo);

	ret = drmModePageFlip(drm.fd, drm.crtc_id, fb->fb_id,
			DRM_MODE_PAGE_FLIP_EVENT, &waiting_for_flip);
	if (ret) {
		printf("failed to queue page flip: %s\n", strerror(errno));
		return -1;
	}

	while (waiting_for_flip) {
		FD_ZERO(&fds);
		FD_SET(0, &fds);
		FD_SET(drm.fd, &fds);

		ret = select(drm.fd + 1, &fds, NULL, NULL, NULL);
		if (ret < 0) {
			printf("select err: %s\n", strerror(errno));
			return ret;
		} else if (ret == 0) {
			printf("select timeout!\n");
			return -1;
		} else if (FD_ISSET(0, &fds)) {
			printf("user interrupted!\n");
			break;
		}
		drmHandleEvent(drm.fd, &evctx);
	}

	/* release last buffer to render on again: */
	gbm_surface_release_buffer(gbm.surface, state.bo);
	state.bo = next_bo;

	return ret;
}

