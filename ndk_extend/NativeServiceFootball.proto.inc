
// auto generated , do not edit !!!

API_PROTO(ANativeServiceFootball_create, int, NativeServiceFootball *impl, ANativeServiceFootball **out_service);
API_PROTO(ANativeServiceFootball_getBinder, int, ANativeServiceFootball *service, void **binder);
API_PROTO(ANativeServiceFootball_addService, int, ANativeServiceFootball *service, const char *name_);
API_PROTO(ANativeServiceFootball_removeService, int, ANativeServiceFootball *service);
API_PROTO(ANativeServiceFootball_destroy, int, ANativeServiceFootball *service);

API_PROTO(ANativeServiceFootballProxy_create, int, const char *name_, ANativeServiceFootballProxy **out_proxy);
API_PROTO(ANativeServiceFootballProxy_create_from_binder, int, void *ibinder, ANativeServiceFootballProxy **out_proxy);
API_PROTO(ANativeServiceFootballProxy_get, NativeServiceFootball*, ANativeServiceFootballProxy *proxy);
API_PROTO(ANativeServiceFootballProxy_setDeathRecipient, int, ANativeServiceFootballProxy *proxy, PFNativeServiceFootball_binder_DeathRecipient pf, void *ctx);
API_PROTO(ANativeServiceFootballProxy_refInc, int, ANativeServiceFootballProxy *proxy);
API_PROTO(ANativeServiceFootballProxy_refDec, int, ANativeServiceFootballProxy *proxy);
API_PROTO(ANativeServiceFootballProxy_isValid, bool, ANativeServiceFootballProxy *proxy);
API_PROTO(ANativeServiceFootballProxy_destroy, int, ANativeServiceFootballProxy *proxy);


