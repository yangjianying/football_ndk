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

#define LOG_TAG "FootballPPVk"
#include "android_logcat_.h"

#include "NativeHooApi.h"

#include "ExtLibWrapper_c.h"

#define pfunc(x) PF_##x
#define PF(x) (*pfunc(x))

typedef void PF(ANativeProcessState_startThreadPool)();

typedef int PF(ANativeHooSurface_create)(const char *name, uint32_t width, uint32_t height, uint32_t format, uint32_t flags,
	ANativeHooSurface **out_surface);
typedef int PF(ANativeHooSurface_destroy)(ANativeHooSurface *surface);
typedef int PF(ANativeHooSurface_getWidth)(ANativeHooSurface *surface, uint32_t *width);
typedef int PF(ANativeHooSurface_getHeight)(ANativeHooSurface *surface, uint32_t *height);
typedef int PF(ANativeHooSurface_getFormat)(ANativeHooSurface *surface, uint32_t *format);
typedef int PF(ANativeHooSurface_getWindow)(ANativeHooSurface* surface, /*out*/ANativeWindow** window);

typedef int PF(ANativeHooDisplay_create)(const char *name, uint32_t width, uint32_t height, ANativeWindow *window, ANativeHooDisplay **out_display);
typedef int PF(ANativeHooDisplay_destroy)(ANativeHooDisplay *display);


static lib_entry_name_t ndk_extend_lib_symbols[] = {
	LIB_ENTRY("ANativeProcessState_startThreadPool"), // 0
	
	LIB_ENTRY("ANativeHooSurface_create"),  // 1
	LIB_ENTRY("ANativeHooSurface_destroy"),  // 2
	LIB_ENTRY("ANativeHooSurface_getWidth"),  // 3
	LIB_ENTRY("ANativeHooSurface_getHeight"),  // 4
	LIB_ENTRY("ANativeHooSurface_getFormat"),  // 5
	LIB_ENTRY("ANativeHooSurface_getWindow"),  // 6
	
	LIB_ENTRY("ANativeHooDisplay_create"),  // 7
	LIB_ENTRY("ANativeHooDisplay_destroy"),  // 8
	
};
DECLARE_ExtLibWrapper(lib_loader_ndk_extend);
IMPL_ExtLibWrapper(lib_loader_ndk_extend);

namespace android {
namespace ndk {
namespace extend {
	static lib_loader_ndk_extend *ndk_extend_ = nullptr;
	int android_ndk_extend_initialize() {
		if (ndk_extend_ == nullptr) {
			ndk_extend_ = lib_loader_ndk_extend::makeInstance();
			if (ndk_extend_->load("libsfhookerapi.so", 
				ndk_extend_lib_symbols, SIZEOF_(ndk_extend_lib_symbols)) < 0) {
				fprintf(stderr, "fatal error in %s \r\n", __func__);
				abort();
			}
		}
		return 0;
	}
	int android_ndk_extend_uninitialize() {
		if (ndk_extend_ != nullptr) {
			lib_loader_ndk_extend::releaseInstance(ndk_extend_);
			ndk_extend_ = nullptr;
		}
		return 0;
	}
};
};
};

#define NDK_EXTEND_LIBPTR ::android::ndk::extend::ndk_extend_

#ifdef __cplusplus
extern "C" {
#endif

void ANativeProcessState_startThreadPool() {
	CALLLIB_FUNC_VOID(NDK_EXTEND_LIBPTR, 0, pfunc(ANativeProcessState_startThreadPool));
}

struct ANativeHooSurface {
	int dummy;
};

int ANativeHooSurface_create(const char *name, uint32_t width, uint32_t height, uint32_t format, uint32_t flags,
	ANativeHooSurface **out_surface) {
	CALLLIB_FUNC_R(NDK_EXTEND_LIBPTR, 1, pfunc(ANativeHooSurface_create), name, width, height, format, flags, out_surface);
	return -1;
}
int ANativeHooSurface_destroy(ANativeHooSurface *surface) {
	CALLLIB_FUNC_R(NDK_EXTEND_LIBPTR, 2, pfunc(ANativeHooSurface_destroy), surface);
	return -1;
}
int ANativeHooSurface_getWidth(ANativeHooSurface *surface, uint32_t *width) {
	CALLLIB_FUNC_R(NDK_EXTEND_LIBPTR, 3, pfunc(ANativeHooSurface_getWidth), surface, width);
	return -1; 
}
int ANativeHooSurface_getHeight(ANativeHooSurface *surface, uint32_t *height) {
	CALLLIB_FUNC_R(NDK_EXTEND_LIBPTR, 4, pfunc(ANativeHooSurface_getHeight), surface, height);
	return -1; 
}
int ANativeHooSurface_getFormat(ANativeHooSurface *surface, uint32_t *format) {
	CALLLIB_FUNC_R(NDK_EXTEND_LIBPTR, 5, pfunc(ANativeHooSurface_getFormat), surface, format);
	return -1; 
}
int ANativeHooSurface_getWindow(ANativeHooSurface* surface, /*out*/ANativeWindow** window) {
	CALLLIB_FUNC_R(NDK_EXTEND_LIBPTR, 6, pfunc(ANativeHooSurface_getWindow), surface, window);
	return -1; 
}

struct ANativeHooDisplay {
	int dummy;
};

int ANativeHooDisplay_create(
	const char *name, uint32_t width, uint32_t height, ANativeWindow *window, ANativeHooDisplay **out_display) {
	CALLLIB_FUNC_R(NDK_EXTEND_LIBPTR, 7, pfunc(ANativeHooDisplay_create), name, width, height, window, out_display);
	return -1;
}
int ANativeHooDisplay_destroy(ANativeHooDisplay *display) {
	CALLLIB_FUNC_R(NDK_EXTEND_LIBPTR, 8, pfunc(ANativeHooDisplay_destroy), display);
	return -1;
}

#ifdef __cplusplus
};
#endif



