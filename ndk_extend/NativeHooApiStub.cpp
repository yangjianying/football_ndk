#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<unistd.h>
#include <sched.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>

#include <sys/resource.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<sys/ioctl.h>
#include <sys/mman.h>
#include <sys/wait.h>  // wait()

#include <android/native_window.h>

#include "NativeHooApi.h"

#ifdef __cplusplus
extern "C" {
#endif

void ANativeProcessState_startThreadPool() { }

struct ANativeHooSurface {
	int dummy;
};

int ANativeHooSurface_create(const char *name, uint32_t width, uint32_t height, uint32_t format, uint32_t flags,
	ANativeHooSurface **out_surface) { return 0; }
int ANativeHooSurface_destroy(ANativeHooSurface *surface) { return 0; }
int ANativeHooSurface_getWidth(ANativeHooSurface *surface, uint32_t *width) { return 0; }
int ANativeHooSurface_getHeight(ANativeHooSurface *surface, uint32_t *height) { return 0; }
int ANativeHooSurface_getFormat(ANativeHooSurface *surface, uint32_t *format) { return 0; }
int ANativeHooSurface_getWindow(ANativeHooSurface* surface, /*out*/ANativeWindow** window) { return 0; }


struct ANativeHooDisplay {
	int dummy;
};

int ANativeHooDisplay_create(
	const char *name, uint32_t width, uint32_t height, ANativeWindow *window, ANativeHooDisplay **out_display) {
	return 0;
}
int ANativeHooDisplay_destroy(ANativeHooDisplay *display) { return 0;}

#ifdef __cplusplus
};
#endif


