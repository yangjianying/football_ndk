#ifndef __FOOTBALL_CONFIG_H__
#define __FOOTBALL_CONFIG_H__


#define __FOOTBALL_MAKE_VERSION_(a,b,c) (a<<16 | b<<8 | c)

#define __FOOTBALL_VERSION_MAJOR(v) (((v)>>16)&0xff)
#define __FOOTBALL_VERSION_MINOR(v) (((v)>>8)&0xff)
#define __FOOTBALL_VERSION_develop(v) (((v)>>0)&0xff)


/*****************************************************************************/


#define FOOTBALL_VERSION __FOOTBALL_MAKE_VERSION_(1, 1, 3)
/*
1.1.3
add a binder client for output the debug info.
add two histogram statistic implementation , one using compute shader, the other using graphic pipeline
based on the above histogram statistic, implement the AGC, BHE, and AGC-BHE algorithm. 

1.1.2
add build.sh for linux  ndk
impl another ndk extension api loader,  use "NativeHooApi_Loader.h" for other code parts.
  when new api call should be added, 
  1, just copy the new "NativeHooApi.h" header file into this project, 
  2, then add the new api into "NativeHooApiLoader2_proto.inc", must one api per line. 
  3, add stub implementation in "NativeHooApiStub.cpp"
	TODO: how to generate the "NativeHooApiLoader2_proto.inc" file automatically.  
	TODO: generate "NativeHooApiStub.cpp" file automatically.
fpp
	fpp
	fpp -h
	fpp -p 0
	fpp -p 1
	fpp -p 2
	fpp -c 10
	fpp -p 0 -c 10
	fpp -p 1 -c 10
	fpp -p 2 -c 10
	p
	n
	c

vpp
	vpp -h
	vpp 
	vpp -p 0
	vpp -p 1
	vpp -p 2
	vpp -c 10 						// default 100ms
	vpp -c 10 -t 1000  				// 1000ms timeout !!!
	vpp -p 0 -c 10 -t 1000
	vpp -p 0 -t 1000   				// only 1 times
	vpp -d 							// delete pp tester 
	vpp -p 0 -c 10 -t 1000 -d 		// will delete pp tester every test cycle !!!

	currently , following cmd always test failed !!!
	vpp -p 0 -c 50 -t 2000  // test failed !!!


todo:
  1, impl the binder in ndk_extend module
  2, impl the socket framework 
  3, add client tool for config/control footballd daemon


1.1.1
2020.02.11, first version

cmdline/
	1, v1
	2, v2
	3, lua impl

external/
external/lua-5.3.5
	1, compile readline-8.0 under linux using standalone ndk toolchain,
		for libhistory.a, libhistory.a
	2, compile ncurses-6.1 under linux using standalone ndk toolchain,
		for libncurses.a
	3, compile lua-5.3.5 under windows using build_lua_arm64-v8a.bat/build_lua_armeabi-v7a.bat,
		for libluajit.a
	4, import all *.a generated above into this project.
		*but lua.c's command line still works not very satisfied:
			back space can not delete the char .

miniled/

ndk_extend/

pp/

standalone/	// used for run NativeActivity's native code standalone, not within JVM !
standalone\android-vulkan-tutorials\tutorial06_texture_v2
standalone\gles
standalone\SaschaWillems\computedemo1
SaschaWillems\android\examples\computeshader_foot
SaschaWillems\android\examples\vulkanscene
SaschaWillems\android\examples\computeraytracing


*/

/*****************************************************************************/


#define FootballSysApi_AUTO_RUN_ 0 // 1


#define FootballPPTester_window_width (1080)
#define FootballPPTester_window_height (2340) // (2340)  // 1920
/*************************************************************** warning : 
* if this size is the same as the full screen size, but the surfaceflinger is not modified for Surface-Display loop 
* the display seems not update normally !!!
*/

//#define FootballPPTester_FileManager_DEFAULT_PATH "/sdcard/Pictures"
#define FootballPPTester_FileManager_DEFAULT_PATH "/sdcard/Pictures/LCUP_R5"

#define FootballPPTester_special_SURFACE_NAME "AV0Q4-3AEH8" // 
//#define FootballPPTester_special_SURFACE_NAME "#test_surface"

#define FootballPPTester_special_DISPLAY_NAME "1F04Z-6D111"
//#define FootballPPTester_special_DISPLAY_NAME "#test_display" // 

#define AAssetManagerImpl_DEFAULT_BASEPATH "/sdcard/data/"
//#define AAssetManagerImpl_DEFAULT_BASEPATH "/data/vendor/display/"

#define AAssetManagerImpl_DEFAULT_PATH "/sdcard/data/assets/"


namespace football {
	class FootballConfig {
	public:
		static const char *cycling_service_name;
		static void print_version();
	};
};

#include "utils/debug_handler.h"


#endif


