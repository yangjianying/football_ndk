
/** auto generated, do not edit !!! **/
/** auto generated, do not edit !!! **/
/** auto generated, do not edit !!! **/

#ifndef __NATIVESERVICEFOOTBALL_LOADER_H__
#define __NATIVESERVICEFOOTBALL_LOADER_H__


#include "NativeServiceFootball_extra.h"


#define NativeServiceFootball_NO_PROTOTYPES
#include "NativeServiceFootball.h"

/* here define a function pointer variable the same as the 'a' , then use this a as the function 
 * else we should use another macro to re-direct the normal 'a' call to NATIVESERVICEFOOTBALL_ptr(a) */
 
#define NATIVESERVICEFOOTBALL_ptr(a) a

#define NATIVESERVICEFOOTBALL_PF(a) NATIVESERVICEFOOTBALL_PF_##a##_1


#define NATIVESERVICEFOOTBALL_DECLARE(func___name, r, ...); \
	typedef r (* NATIVESERVICEFOOTBALL_PF(func___name))(__VA_ARGS__);  \
	extern NATIVESERVICEFOOTBALL_PF(func___name) NATIVESERVICEFOOTBALL_ptr(func___name);

// 1,
#define API_PROTO NATIVESERVICEFOOTBALL_DECLARE
#include "NativeServiceFootball.proto.inc"
#undef API_PROTO

//
namespace nativeservicefootball {
namespace loader {
	int NativeServiceFootball_initialize();
	int NativeServiceFootball_uninitialize();
};
};

#define NativeServiceFootball_INITIALIZE ::nativeservicefootball::loader::NativeServiceFootball_initialize
#define NativeServiceFootball_UNINITIALIZE ::nativeservicefootball::loader::NativeServiceFootball_uninitialize


#endif

