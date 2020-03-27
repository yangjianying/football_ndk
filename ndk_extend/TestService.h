
// auto generated , do not edit !!!
#ifndef __TESTSERVICE__H___
#define __TESTSERVICE__H___

// interface name must be the same as the filename !!!
class TestService {
public:
	TestService() {}
	virtual ~TestService() {} ;

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
#include "TestService.bidl"
#undef INTERFACE
#undef INTERFACE_r
#undef BINDER_FUNC_DECLARE


};

#ifdef __cplusplus
extern "C" {
#endif

struct ATestService;
typedef struct ATestService ATestService;

struct ATestServiceProxy;
typedef struct ATestServiceProxy ATestServiceProxy;

typedef void (*PFTestService_binder_DeathRecipient)(void *, ATestServiceProxy *);

// this macro must be {your_interface_filename}_NO_PROTOTYPES
// for example {your_interface_filename} = MyServiceName ,
//     ie your have this file named "MyServiceName.h" and "MyServiceName.proto.inc"
// then macro is MyServiceName_NO_PROTOTYPES

#ifndef TestService_NO_PROTOTYPES
int ATestService_create(TestService *impl, ATestService **out_service);
int ATestService_getBinder(ATestService *service, void **binder);
int ATestService_addService(ATestService *service, const char *name_);
int ATestService_removeService(ATestService *service);
int ATestService_destroy(ATestService *service);

int ATestServiceProxy_create(const char *name_, ATestServiceProxy **out_proxy);
int ATestServiceProxy_create_from_binder(void *ibinder, ATestServiceProxy **out_proxy);
TestService* ATestServiceProxy_get(ATestServiceProxy *proxy);
int ATestServiceProxy_setDeathRecipient(ATestServiceProxy *proxy, PFTestService_binder_DeathRecipient pf, void *ctx);
int ATestServiceProxy_refInc(ATestServiceProxy *proxy);
int ATestServiceProxy_refDec(ATestServiceProxy *proxy);
bool ATestServiceProxy_isValid(ATestServiceProxy *proxy);
int ATestServiceProxy_destroy(ATestServiceProxy *proxy);
#endif


#ifdef __cplusplus
};
#endif

#endif


