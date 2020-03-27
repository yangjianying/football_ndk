
// auto generated , do not edit !!!
#ifndef __NATIVEFOOTBALLRECEIVER__H___
#define __NATIVEFOOTBALLRECEIVER__H___

// interface name must be the same as the filename !!!
class NativeFootballReceiver {
public:
	NativeFootballReceiver() {}
	virtual ~NativeFootballReceiver() {} ;

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
#include "NativeFootballReceiver.bidl"
#undef INTERFACE
#undef INTERFACE_r
#undef BINDER_FUNC_DECLARE


};

#ifdef __cplusplus
extern "C" {
#endif

struct ANativeFootballReceiver;
typedef struct ANativeFootballReceiver ANativeFootballReceiver;

struct ANativeFootballReceiverProxy;
typedef struct ANativeFootballReceiverProxy ANativeFootballReceiverProxy;

typedef void (*PFNativeFootballReceiver_binder_DeathRecipient)(void *, ANativeFootballReceiverProxy *);

// this macro must be {your_interface_filename}_NO_PROTOTYPES
// for example {your_interface_filename} = MyServiceName ,
//     ie your have this file named "MyServiceName.h" and "MyServiceName.proto.inc"
// then macro is MyServiceName_NO_PROTOTYPES

#ifndef NativeFootballReceiver_NO_PROTOTYPES
int ANativeFootballReceiver_create(NativeFootballReceiver *impl, ANativeFootballReceiver **out_service);
int ANativeFootballReceiver_getBinder(ANativeFootballReceiver *service, void **binder);
int ANativeFootballReceiver_addService(ANativeFootballReceiver *service, const char *name_);
int ANativeFootballReceiver_removeService(ANativeFootballReceiver *service);
int ANativeFootballReceiver_destroy(ANativeFootballReceiver *service);

int ANativeFootballReceiverProxy_create(const char *name_, ANativeFootballReceiverProxy **out_proxy);
int ANativeFootballReceiverProxy_create_from_binder(void *ibinder, ANativeFootballReceiverProxy **out_proxy);
NativeFootballReceiver* ANativeFootballReceiverProxy_get(ANativeFootballReceiverProxy *proxy);
int ANativeFootballReceiverProxy_setDeathRecipient(ANativeFootballReceiverProxy *proxy, PFNativeFootballReceiver_binder_DeathRecipient pf, void *ctx);
int ANativeFootballReceiverProxy_refInc(ANativeFootballReceiverProxy *proxy);
int ANativeFootballReceiverProxy_refDec(ANativeFootballReceiverProxy *proxy);
bool ANativeFootballReceiverProxy_isValid(ANativeFootballReceiverProxy *proxy);
int ANativeFootballReceiverProxy_destroy(ANativeFootballReceiverProxy *proxy);
#endif


#ifdef __cplusplus
};
#endif

#endif


