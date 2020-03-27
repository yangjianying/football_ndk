#ifndef __NATIVE_HOO_API_H___
#define __NATIVE_HOO_API_H___

#include <android/native_window.h>  // ANativeWindow, ANativeWindow_Buffer

#ifdef __cplusplus
extern "C" {
#endif

enum {
	ANativeHoo_ISurfaceComposerClient_eHidden = 0x00000004,
	ANativeHoo_ISurfaceComposerClient_eOpaque = 0x00000400,
};

struct ANativeHooSurface;
typedef struct ANativeHooSurface ANativeHooSurface;

struct ANativeHooDisplay;
typedef struct ANativeHooDisplay ANativeHooDisplay;

#ifndef NativeHooApi_NO_PROTOTYPES

void ANativeProcessState_startThreadPool();
void ANativeIPCThreadState_joinThreadPool();

int ANativeHooSurface_create(const char *name, uint32_t width, uint32_t height, uint32_t format, uint32_t flags,
	ANativeHooSurface **out_surface);
int ANativeHooSurface_destroy(ANativeHooSurface *surface);
int ANativeHooSurface_show(ANativeHooSurface *surface);
int ANativeHooSurface_hide(ANativeHooSurface *surface);
int ANativeHooSurface_setPos(ANativeHooSurface *surface, int x, int y);
int ANativeHooSurface_getWidth(ANativeHooSurface *surface, uint32_t *width);
int ANativeHooSurface_getHeight(ANativeHooSurface *surface, uint32_t *height);
int ANativeHooSurface_getFormat(ANativeHooSurface *surface, uint32_t *format);
int ANativeHooSurface_getWindow(ANativeHooSurface* surface, /*out*/ANativeWindow** window);

int ANativeHooDisplay_create(const char *name, uint32_t width, uint32_t height, ANativeWindow *window, ANativeHooDisplay **out_display);
int ANativeHooDisplay_destroy(ANativeHooDisplay *display);

int ANativeHoo_CCT_autocontrol(int flags);

#endif

#ifdef __cplusplus
};
#endif


#endif

