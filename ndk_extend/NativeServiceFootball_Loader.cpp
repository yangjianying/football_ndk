
/** auto generated, do not edit !!! **/
/** auto generated, do not edit !!! **/
/** auto generated, do not edit !!! **/
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <stddef.h>
#include <dlfcn.h>

#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/wait.h>  // wait()

#include "FootballConfig.h"
#undef __CLASS__
#define __CLASS__ "NativeServiceFootball"

#define NATIVESERVICEFOOTBALL_LOADER_SORUCE
#include "NativeServiceFootball_Loader.h"

#define NATIVESERVICEFOOTBALL_symbol(a) #a

#define API_LOAD(func___name, r, ...); \
	NATIVESERVICEFOOTBALL_ptr(func___name) = \
		(NATIVESERVICEFOOTBALL_PF(func___name)) \
			dlsym(__api_lib, NATIVESERVICEFOOTBALL_symbol(func___name)); \
	if (NATIVESERVICEFOOTBALL_ptr(func___name) == nullptr) { \
		DLOGD( "%s dlsym failed! \r\n", NATIVESERVICEFOOTBALL_symbol(func___name)); \
	} else { \
		/*DLOGD( "%s dlsym ok! \r\n", NATIVESERVICEFOOTBALL_symbol(func___name));*/ \
	}

#define API_IMPL(func___name, r, ...); \
	NATIVESERVICEFOOTBALL_PF(func___name) NATIVESERVICEFOOTBALL_ptr(func___name) = nullptr;


// 2
#define API_PROTO API_IMPL
#include "NativeServiceFootball.proto.inc"
#undef API_PROTO

namespace nativeservicefootball {
namespace loader {
	static void *__api_lib = NULL;
	int NativeServiceFootball_initialize() {
		if (__api_lib != nullptr) {
			/*DLOGD( "libsfhookerapi.so already loaded ! \r\n");*/
			return 0;		
		}
		__api_lib = dlopen("libsfhookerapi.so", RTLD_NOW);
		if (__api_lib == nullptr) { 
			const char* error = dlerror();
			DLOGD( "%s dlopen libsfhookerapi.so  error:%s \r\n", __func__, error);
			abort();
			return -1;
		}
		/*DLOGD( "libsfhookerapi.so load ok ! \r\n");*/
		
		// 3
		#define API_PROTO API_LOAD
		#include "NativeServiceFootball.proto.inc"
		#undef API_PROTO
		
		return 0;
	}
	int NativeServiceFootball_uninitialize() {
		if (__api_lib == nullptr) {
			DLOGD( "libsfhookerapi.so NOT loaded ! \r\n");
			return -1;
		}
		dlclose(__api_lib);
		__api_lib = nullptr;
		/*DLOGD( "libsfhookerapi.so close ok ! \r\n");*/
		return 0;
	}
};
};




