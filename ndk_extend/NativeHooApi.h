#ifndef __NATIVE_HOO_API_H___
#define __NATIVE_HOO_API_H___

#include <android/native_window.h>  // ANativeWindow, ANativeWindow_Buffer

#ifdef __cplusplus
extern "C" {
#endif

void ANativeProcessState_startThreadPool();

struct ANativeHooSurface;
typedef struct ANativeHooSurface ANativeHooSurface;

int ANativeHooSurface_create(const char *name, uint32_t width, uint32_t height, uint32_t format, uint32_t flags,
	ANativeHooSurface **out_surface);
int ANativeHooSurface_destroy(ANativeHooSurface *surface);
int ANativeHooSurface_getWidth(ANativeHooSurface *surface, uint32_t *width);
int ANativeHooSurface_getHeight(ANativeHooSurface *surface, uint32_t *height);
int ANativeHooSurface_getFormat(ANativeHooSurface *surface, uint32_t *format);
int ANativeHooSurface_getWindow(ANativeHooSurface* surface, /*out*/ANativeWindow** window);

struct ANativeHooDisplay;
typedef struct ANativeHooDisplay ANativeHooDisplay;
int ANativeHooDisplay_create(const char *name, uint32_t width, uint32_t height, ANativeWindow *window, ANativeHooDisplay **out_display);
int ANativeHooDisplay_destroy(ANativeHooDisplay *display);

#ifdef __cplusplus
};
#endif


#endif

