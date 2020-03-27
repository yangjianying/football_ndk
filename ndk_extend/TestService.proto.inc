
// auto generated , do not edit !!!

API_PROTO(ATestService_create, int, TestService *impl, ATestService **out_service);
API_PROTO(ATestService_getBinder, int, ATestService *service, void **binder);
API_PROTO(ATestService_addService, int, ATestService *service, const char *name_);
API_PROTO(ATestService_removeService, int, ATestService *service);
API_PROTO(ATestService_destroy, int, ATestService *service);

API_PROTO(ATestServiceProxy_create, int, const char *name_, ATestServiceProxy **out_proxy);
API_PROTO(ATestServiceProxy_create_from_binder, int, void *ibinder, ATestServiceProxy **out_proxy);
API_PROTO(ATestServiceProxy_get, TestService*, ATestServiceProxy *proxy);
API_PROTO(ATestServiceProxy_setDeathRecipient, int, ATestServiceProxy *proxy, PFTestService_binder_DeathRecipient pf, void *ctx);
API_PROTO(ATestServiceProxy_refInc, int, ATestServiceProxy *proxy);
API_PROTO(ATestServiceProxy_refDec, int, ATestServiceProxy *proxy);
API_PROTO(ATestServiceProxy_isValid, bool, ATestServiceProxy *proxy);
API_PROTO(ATestServiceProxy_destroy, int, ATestServiceProxy *proxy);


