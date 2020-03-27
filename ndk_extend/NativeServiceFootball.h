
// auto generated , do not edit !!!
#ifndef __NATIVESERVICEFOOTBALL__H___
#define __NATIVESERVICEFOOTBALL__H___

// interface name must be the same as the filename !!!
class NativeServiceFootball {
public:
	NativeServiceFootball() {}
	virtual ~NativeServiceFootball() {} ;

	// add interface function below !!!
	// 1
#undef INTERFACE
#undef INTERFACE_r
#undef BINDER_FUNC_DECLARE
#define BINDER_FUNC_DECLARE(code_, func___name, r, r_default__, real_, ...);	\
	enum {CODE_##func___name = code_};							\
	virtual r func___name(__VA_ARGS__) = 0;
#define INTERFACE BINDER_FUNC_DECLARE
#define INTERFACE_r BINDER_FUNC_DECLARE
#include "NativeServiceFootball.bidl"
#undef INTERFACE
#undef INTERFACE_r
#undef BINDER_FUNC_DECLARE


};

#ifdef __cplusplus
extern "C" {
#endif

struct ANativeServiceFootball;
typedef struct ANativeServiceFootball ANativeServiceFootball;

struct ANativeServiceFootballProxy;
typedef struct ANativeServiceFootballProxy ANativeServiceFootballProxy;

typedef void (*PFNativeServiceFootball_binder_DeathRecipient)(void *, ANativeServiceFootballProxy *);

// this macro must be {your_interface_filename}_NO_PROTOTYPES
// for example {your_interface_filename} = MyServiceName ,
//     ie your have this file named "MyServiceName.h" and "MyServiceName.proto.inc"
// then macro is MyServiceName_NO_PROTOTYPES

#ifndef NativeServiceFootball_NO_PROTOTYPES
int ANativeServiceFootball_create(NativeServiceFootball *impl, ANativeServiceFootball **out_service);
int ANativeServiceFootball_getBinder(ANativeServiceFootball *service, void **binder);
int ANativeServiceFootball_addService(ANativeServiceFootball *service, const char *name_);
int ANativeServiceFootball_removeService(ANativeServiceFootball *service);
int ANativeServiceFootball_destroy(ANativeServiceFootball *service);

int ANativeServiceFootballProxy_create(const char *name_, ANativeServiceFootballProxy **out_proxy);
int ANativeServiceFootballProxy_create_from_binder(void *ibinder, ANativeServiceFootballProxy **out_proxy);
NativeServiceFootball* ANativeServiceFootballProxy_get(ANativeServiceFootballProxy *proxy);
int ANativeServiceFootballProxy_setDeathRecipient(ANativeServiceFootballProxy *proxy, PFNativeServiceFootball_binder_DeathRecipient pf, void *ctx);
int ANativeServiceFootballProxy_refInc(ANativeServiceFootballProxy *proxy);
int ANativeServiceFootballProxy_refDec(ANativeServiceFootballProxy *proxy);
bool ANativeServiceFootballProxy_isValid(ANativeServiceFootballProxy *proxy);
int ANativeServiceFootballProxy_destroy(ANativeServiceFootballProxy *proxy);
#endif


#ifdef __cplusplus
};
#endif

#endif


