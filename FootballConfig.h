#ifndef __FOOTBALL_CONFIG_H__
#define __FOOTBALL_CONFIG_H__


#define MAKE_VERSION_(a,b,c) (a<<16 | b<<8 | c)
#define VERSION_MAJOR(v) (((v)>>16)&0xff)
#define VERSION_MINOR(v) (((v)>>8)&0xff)
#define VERSION_develop(v) (((v)>>0)&0xff)


/*****************************************************************************/


#define FOOTBALL_VERSION MAKE_VERSION_(1, 1, 1)
/*


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


#define FootballPPTester_FileManager_DEFAULT_PATH "/sdcard/Pictures"

#define FootballPPTester_special_SURFACE_NAME "#test_surface"

#define FootballPPTester_special_DISPLAY_NAME "#test_display"

#define AAssetManagerImpl_DEFAULT_PATH "/sdcard/data/assets/"





#endif


