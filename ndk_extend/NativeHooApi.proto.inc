
API_PROTO(ANativeProcessState_startThreadPool, void, void);
API_PROTO(ANativeIPCThreadState_joinThreadPool, void, void);

API_PROTO(ANativeHooSurface_create, int, const char *name, uint32_t width, uint32_t height, uint32_t format, uint32_t flags, ANativeHooSurface **out_surface);
API_PROTO(ANativeHooSurface_destroy, int, ANativeHooSurface *surface);
API_PROTO(ANativeHooSurface_show, int, ANativeHooSurface *surface);
API_PROTO(ANativeHooSurface_hide, int, ANativeHooSurface *surface);
API_PROTO(ANativeHooSurface_setPos, int, ANativeHooSurface *surface, int x, int y);
API_PROTO(ANativeHooSurface_getWidth, int, ANativeHooSurface *surface, uint32_t *width);
API_PROTO(ANativeHooSurface_getHeight, int, ANativeHooSurface *surface, uint32_t *height);
API_PROTO(ANativeHooSurface_getFormat, int, ANativeHooSurface *surface, uint32_t *format);
API_PROTO(ANativeHooSurface_getWindow, int, ANativeHooSurface* surface, /*out*/ANativeWindow** window);
API_PROTO(ANativeHooDisplay_create, int, const char *name, uint32_t width, uint32_t height, ANativeWindow *window, ANativeHooDisplay **out_display);
API_PROTO(ANativeHooDisplay_destroy, int, ANativeHooDisplay *display);
API_PROTO(ANativeHoo_CCT_autocontrol, int, int flags);

