
/** auto generated, do not edit !!! **/
/** auto generated, do not edit !!! **/
/** auto generated, do not edit !!! **/

#ifndef __NATIVEHOOAPI_LOADER_H__
#define __NATIVEHOOAPI_LOADER_H__


#include "NativeHooApi_extra.h"


#define NativeHooApi_NO_PROTOTYPES
#include "NativeHooApi.h"

/* here define a function pointer variable the same as the 'a' , then use this a as the function 
 * else we should use another macro to re-direct the normal 'a' call to NATIVEHOOAPI_ptr(a) */
 
#define NATIVEHOOAPI_ptr(a) a

#define NATIVEHOOAPI_PF(a) NATIVEHOOAPI_PF_##a##_1


#define NATIVEHOOAPI_DECLARE(func___name, r, ...); \
	typedef r (* NATIVEHOOAPI_PF(func___name))(__VA_ARGS__);  \
	extern NATIVEHOOAPI_PF(func___name) NATIVEHOOAPI_ptr(func___name);

// 1,
#define API_PROTO NATIVEHOOAPI_DECLARE
#include "NativeHooApi.proto.inc"
#undef API_PROTO

//
namespace nativehooapi {
namespace loader {
	int NativeHooApi_initialize();
	int NativeHooApi_uninitialize();
};
};

#define NativeHooApi_INITIALIZE ::nativehooapi::loader::NativeHooApi_initialize
#define NativeHooApi_UNINITIALIZE ::nativehooapi::loader::NativeHooApi_uninitialize


#endif

