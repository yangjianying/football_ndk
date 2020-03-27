
// auto generated , do not edit !!!

API_PROTO(ANativeFootballReceiver_create, int, NativeFootballReceiver *impl, ANativeFootballReceiver **out_service);
API_PROTO(ANativeFootballReceiver_getBinder, int, ANativeFootballReceiver *service, void **binder);
API_PROTO(ANativeFootballReceiver_addService, int, ANativeFootballReceiver *service, const char *name_);
API_PROTO(ANativeFootballReceiver_removeService, int, ANativeFootballReceiver *service);
API_PROTO(ANativeFootballReceiver_destroy, int, ANativeFootballReceiver *service);

API_PROTO(ANativeFootballReceiverProxy_create, int, const char *name_, ANativeFootballReceiverProxy **out_proxy);
API_PROTO(ANativeFootballReceiverProxy_create_from_binder, int, void *ibinder, ANativeFootballReceiverProxy **out_proxy);
API_PROTO(ANativeFootballReceiverProxy_get, NativeFootballReceiver*, ANativeFootballReceiverProxy *proxy);
API_PROTO(ANativeFootballReceiverProxy_setDeathRecipient, int, ANativeFootballReceiverProxy *proxy, PFNativeFootballReceiver_binder_DeathRecipient pf, void *ctx);
API_PROTO(ANativeFootballReceiverProxy_refInc, int, ANativeFootballReceiverProxy *proxy);
API_PROTO(ANativeFootballReceiverProxy_refDec, int, ANativeFootballReceiverProxy *proxy);
API_PROTO(ANativeFootballReceiverProxy_isValid, bool, ANativeFootballReceiverProxy *proxy);
API_PROTO(ANativeFootballReceiverProxy_destroy, int, ANativeFootballReceiverProxy *proxy);


