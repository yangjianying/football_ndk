
/** auto generated, do not edit !!! **/
/** auto generated, do not edit !!! **/
/** auto generated, do not edit !!! **/

#ifndef __TESTSERVICE_LOADER_H__
#define __TESTSERVICE_LOADER_H__


#include "TestService_extra.h"


#define TestService_NO_PROTOTYPES
#include "TestService.h"

/* here define a function pointer variable the same as the 'a' , then use this a as the function 
 * else we should use another macro to re-direct the normal 'a' call to TESTSERVICE_ptr(a) */
 
#define TESTSERVICE_ptr(a) a

#define TESTSERVICE_PF(a) TESTSERVICE_PF_##a##_1


#define TESTSERVICE_DECLARE(func___name, r, ...); \
	typedef r (* TESTSERVICE_PF(func___name))(__VA_ARGS__);  \
	extern TESTSERVICE_PF(func___name) TESTSERVICE_ptr(func___name);

// 1,
#define API_PROTO TESTSERVICE_DECLARE
#include "TestService.proto.inc"
#undef API_PROTO

//
namespace testservice {
namespace loader {
	int TestService_initialize();
	int TestService_uninitialize();
};
};

#define TestService_INITIALIZE ::testservice::loader::TestService_initialize
#define TestService_UNINITIALIZE ::testservice::loader::TestService_uninitialize


#endif

