
/** auto generated, do not edit !!! **/
/** auto generated, do not edit !!! **/
/** auto generated, do not edit !!! **/

#ifndef __NATIVEFOOTBALLRECEIVER_LOADER_H__
#define __NATIVEFOOTBALLRECEIVER_LOADER_H__


#include "NativeFootballReceiver_extra.h"


#define NativeFootballReceiver_NO_PROTOTYPES
#include "NativeFootballReceiver.h"

/* here define a function pointer variable the same as the 'a' , then use this a as the function 
 * else we should use another macro to re-direct the normal 'a' call to NATIVEFOOTBALLRECEIVER_ptr(a) */
 
#define NATIVEFOOTBALLRECEIVER_ptr(a) a

#define NATIVEFOOTBALLRECEIVER_PF(a) NATIVEFOOTBALLRECEIVER_PF_##a##_1


#define NATIVEFOOTBALLRECEIVER_DECLARE(func___name, r, ...); \
	typedef r (* NATIVEFOOTBALLRECEIVER_PF(func___name))(__VA_ARGS__);  \
	extern NATIVEFOOTBALLRECEIVER_PF(func___name) NATIVEFOOTBALLRECEIVER_ptr(func___name);

// 1,
#define API_PROTO NATIVEFOOTBALLRECEIVER_DECLARE
#include "NativeFootballReceiver.proto.inc"
#undef API_PROTO

//
namespace nativefootballreceiver {
namespace loader {
	int NativeFootballReceiver_initialize();
	int NativeFootballReceiver_uninitialize();
};
};

#define NativeFootballReceiver_INITIALIZE ::nativefootballreceiver::loader::NativeFootballReceiver_initialize
#define NativeFootballReceiver_UNINITIALIZE ::nativefootballreceiver::loader::NativeFootballReceiver_uninitialize


#endif

