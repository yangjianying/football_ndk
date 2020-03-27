cmake_minimum_required(VERSION 3.4.1)

# specified in the wrapper shell script !
#include(/cross/github/jni/android-ndk-r14b/build/cmake/android.toolchain.cmake)

# set targetPlatform, will be passed in from gradle when this sample is completed
# openGL Supportability
# platform         status
#   (0 12)           ES2/ES3 not supported
#   [12, 18)         ES2 only; for ES3, app do dynamic load/detection
#                    this applies to the situations that:
#                        - minimum API is set to less than 18. In this case
#                          there is no ES3 header/lib support inside NDK
#                        - the built APK might be running on newer API phones
#                    with dynamic loading of ES3, the same APK would still be able
#                    to use ES3. Otherwise, app would stuck with ES2 even phone is
#                    is newer than the minimum API level (for example, Android-27 etc).
#
#   [18, 24)         ES2 & ES3
#                    If app is built to only support API-18 or later,
#                    set minimum api level to 18 is good enough, NDK supprts ES3
#                    with the right header and lib files. No need to use ES3 dynamic
#                    detection.
#   [24, infinite)   ES2 & ES3 & Vulkan

if(ANDROID)
    message(STATUS "ANDROID")
elseif(APPLE)
    message(STATUS "APPLE")
elseif(WIN32)
    message(STATUS "WIN32")
elseif(UNIX)
    message(STATUS "UNIX")
else()
    message(FATAL_ERROR "OTHER")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")


if (${ANDROID_PLATFORM_LEVEL} LESS 12)
  message(FATAL_ERROR "OpenGL 2 is not supported before API level 11 \
                      (currently using ${ANDROID_PLATFORM_LEVEL}).")
  return()
elseif (${ANDROID_PLATFORM_LEVEL} LESS 18)
  message(STATUS "GLESv2")
  add_definitions("-DDYNAMIC_ES3")
  set(GL3STUB_SRC gl3stub.c)
  set(OPENGL_LIB GLESv2)
else ()
  message(STATUS "GLESv3")
  set(OPENGL_LIB GLESv3)
endif (${ANDROID_PLATFORM_LEVEL} LESS 12)

if (${ANDROID_PLATFORM_LEVEL} GREATER 28)
  set(BINDER_NDK_LIB binder_ndk)
else ()
  set(BINDER_NDK_LIB android)
endif (${ANDROID_PLATFORM_LEVEL} GREATER 28)

message(STATUS ANDROID_PLATFORM_LEVEL = ${ANDROID_PLATFORM_LEVEL})
message(STATUS CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR})
message(STATUS __ANDROID_API__ = ${__ANDROID_API__})
message(STATUS BINDER_NDK_LIB = ${BINDER_NDK_LIB})

set(SRC_DIR ${CMAKE_SOURCE_DIR})
set(SRC_dir_app_standalone ${SRC_DIR}/standalone)

get_filename_component(REPO_ROOT_DIR ${CMAKE_SOURCE_DIR} ABSOLUTE)

message(STATUS REPO_ROOT_DIR = ${REPO_ROOT_DIR})
set(SRC_DIR_SaschaWillems ${REPO_ROOT_DIR}/SaschaWillems)

set(LUA_DIR ${REPO_ROOT_DIR}/external/lua-5.3.5)


if (${ANDROID_ABI} STREQUAL "arm64-v8a")
set(LUA_BUILD_DIR ${LUA_DIR}/lua-build-arm64v8a)
set(READLINE_INSTALL_DIR ${LUA_DIR}/readline/readline-8.0_arm64/install_)
set(NCURSES_INSTALL_DIR ${LUA_DIR}/ncurses/ncurses-6.1_arm64/install_)
endif()
if (${ANDROID_ABI} STREQUAL "armeabi-v7a")
set(LUA_BUILD_DIR ${LUA_DIR}/lua-build-armeabiv7a)
set(READLINE_INSTALL_DIR ${LUA_DIR}/readline/readline-8.0_arm/install_)
set(NCURSES_INSTALL_DIR ${LUA_DIR}/ncurses/ncurses-6.1_arm/install_)
endif()

message(STATUS READLINE_INSTALL_DIR = ${READLINE_INSTALL_DIR})


# -std=c++14

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
       -std=c++17 \
       -fexceptions \
       -fcxx-exceptions \
	-DVK_USE_PLATFORM_ANDROID_KHR -DVK_NO_PROTOTYPES")

# -Wunreachable-code
# -Werror
# -Wno-unused-variable
# -Wno-delete-non-virtual-dtor

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
	-Wall \
	-Werror \
	-Wthread-safety \
	-Wunused \
	-Wunreachable-code \
	-Wno-unused-parameter \
	-Wno-unused-function \
	-Wno-unused-variable \
	-Wno-unreachable-code \
	-Wno-unused-private-field \
	")

if (${ANDROID_ABI} STREQUAL "armeabi-v7a")
   set(CMAKE_CXX_FLAGS} "${CMAKE_CXX_FLAGS} \
       -mhard-float
       -D_NDK_MATH_NO_SOFTFP=1
       -mfloat-abi=hard")
   set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} \
       -Wl,--no-warn-mismatch")
endif()

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u test_football_main")

if (${ANDROID_PLATFORM_LEVEL} GREATER 28)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSING_binder_ndk")
endif (${ANDROID_PLATFORM_LEVEL} GREATER 28)

#add_definitions(-fvisibility=hidden -fvisibility-inlines-hidden)

add_definitions(-fvisibility=default -fvisibility-inlines-hidden)

#///////////////////////////////////////////////////////////////////////
# build native_app_glue as a static lib


add_library(native-app-glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)


#///////////////////////////////////////////////////////////////////////
add_library(libshaderc STATIC IMPORTED)
set_target_properties(libshaderc PROPERTIES
        IMPORTED_LOCATION ${ANDROID_NDK}/sources/third_party/shaderc/libs/c++_static/${ANDROID_ABI}/libshaderc.a)

#///////////////////////////////////////////////////////////////////////
# (source_dir binary_dir)
add_subdirectory(SaschaWillems/android/examples/base SaschaWillems/android/examples/base)

add_subdirectory(${SRC_DIR_SaschaWillems}/android/examples/computeshader_foot ${SRC_DIR_SaschaWillems}/android/examples/computeshader_foot)
add_subdirectory(${SRC_DIR_SaschaWillems}/android/examples/vulkanscene ${SRC_DIR_SaschaWillems}/android/examples/vulkanscene)
add_subdirectory(${SRC_DIR_SaschaWillems}/android/examples/computeparticles ${SRC_DIR_SaschaWillems}/android/examples/computeparticles)
add_subdirectory(${SRC_DIR_SaschaWillems}/android/examples/computeraytracing ${SRC_DIR_SaschaWillems}/android/examples/computeraytracing)

add_subdirectory(${SRC_dir_app_standalone}/SaschaWillems/computedemo1 ${SRC_dir_app_standalone}/SaschaWillems/computedemo1)


#///////////////////////////////////////////////////////////////////////
add_library(sfhookerapi SHARED
            ndk_extend/NativeHooApiStub.cpp 
            )
target_link_libraries(sfhookerapi
		log
		)

#
add_library(libncurses STATIC IMPORTED)
set_target_properties(libncurses PROPERTIES
        IMPORTED_LOCATION ${NCURSES_INSTALL_DIR}/lib/libncurses.a)

#
add_library(libhistory STATIC IMPORTED)
set_target_properties(libhistory PROPERTIES
        IMPORTED_LOCATION ${READLINE_INSTALL_DIR}/lib/libhistory.a)

add_library(libreadline STATIC IMPORTED)
set_target_properties(libreadline PROPERTIES
        IMPORTED_LOCATION ${READLINE_INSTALL_DIR}/lib/libreadline.a)

#
add_library(libluajit STATIC IMPORTED)
set_target_properties(libluajit PROPERTIES
        IMPORTED_LOCATION ${LUA_BUILD_DIR}/libluajit.a)

set(FOOTBALL_STATIC_SRC
	cmdline/cmdline_v1/MenuV1.cpp
	cmdline/cmdline_v1/linktable.c
	cmdline/cmdline_v1/CmdlineV1.cpp
	cmdline/cmdline_v2/cli.cpp
	cmdline/cmdline_v2/cli_mock_utils.cpp
	cmdline/cmdline_v2/cli_readline.cpp
	cmdline/cmdline_v2/cli_simple.cpp
	cmdline/cmdline_v2/CmdlineV2.cpp
	cmdline/cmdline_lua/CmdlineLuaImpl.cpp
	cmdline/cmdline_v2/CmdlineV3.cpp
	cmdline/CmdLineUtils.cpp
	cmdline/CmdLineFactory.cpp

	utils/debug_handler.cpp
	utils/football_debugger.cpp
	utils/foot_utils.cpp
	utils/MemTrace.cpp
	utils/ANativeWindowUtils.cpp
	utils/StbImage_.cpp

	ndk_extend/NativeHooApi_Loader.cpp
	ndk_extend/NativeFootballReceiver_Loader.cpp
	ndk_extend/NativeServiceFootball_Loader.cpp
	ndk_extend/TestService_Loader.cpp

	pp/impl/FootballPPUtils.cpp
	pp/impl/FootballPPCpu.cpp
	pp/impl/gpu_tonemapper/EGLImageBuffer_KHR.cpp
	pp/impl/gpu_tonemapper/glengine.cpp
	pp/impl/screenrecord/EglWindow.cpp
	pp/impl/screenrecord/Program.cpp
	pp/impl/screenrecord/TextRenderer.cpp
	pp/impl/FootSessionGlesImpl1.cpp
	pp/impl/FootballPPGles.cpp
	pp/impl/FootballPPVk.cpp
	pp/FootballPPFactory.cpp
	pp/FootballPPTester.cpp

	miniled/Mbi6322Fb.cpp
	miniled/TestCmdMiniLed.cpp

	${SRC_dir_app_standalone}/AAssetManagerImpl_.cpp
	${SRC_dir_app_standalone}/native_app_glue.c
	${SRC_dir_app_standalone}/ImageReaderHolder.cpp
	${SRC_dir_app_standalone}/StbImageUtils.cpp
	${SRC_dir_app_standalone}/standalone_main.cpp

	${SRC_dir_app_standalone}/android-vulkan-tutorials/tutorial06_texture_v2/vulkan_wrapper.cpp
	${SRC_dir_app_standalone}/android-vulkan-tutorials/tutorial06_texture_v2/CreateShaderModule_.cpp
	${SRC_dir_app_standalone}/android-vulkan-tutorials/tutorial06_texture_v2/VulkanMain_.cpp
	${SRC_dir_app_standalone}/android-vulkan-tutorials/tutorial06_texture_v2/android_main_vk_.cpp

	${SRC_dir_app_standalone}/gles/GlesMain.cpp
	${SRC_dir_app_standalone}/gles/gles3jni.cpp
	${SRC_dir_app_standalone}/gles/RendererES2.cpp
	${SRC_dir_app_standalone}/gles/RendererES3.cpp
	${SRC_dir_app_standalone}/gles/android_main_gles.cpp

	sys_api/FootballSysApi.cpp

	binder/BinderTest.cpp

	FootballConfig.cpp

	TestCmdline.cpp
	TestCmdline_test.cpp
	)
set(FOOTBALL_STATIC_INC
	.
	utils/
	external/
	external/stb
	${LUA_DIR}/src
	cmdline/
	pp/
	${ANDROID_NDK}/sources/third_party/shaderc/include
	${ANDROID_NDK}/sources/android/native_app_glue
	${SRC_dir_app_standalone}
	${SRC_dir_app_standalone}/SaschaWillems/computedemo1
	)

# binder_ndk should >= 29
set(FOOTBALL_STATIC_LIBS
	android
        log
        m
        z
        ${OPENGL_LIB}
        EGL
        nativewindow
        mediandk
        ${BINDER_NDK_LIB}
        libshaderc
        libluajit
        libhistory
        libreadline
 #       termcap  ## cannot find -ltermcap
 #       tinfo  ## cannot find -ltinfo
        libncurses
        native-app-glue
        base_static
        computeshader_foot_static
 #       vulkanscene_static
 #       computeparticles_static
 #       computeraytracing_static
        computedemo1_static
        )

add_library(football_static STATIC
	${FOOTBALL_STATIC_SRC}
	)
target_include_directories(football_static PRIVATE 
	${FOOTBALL_STATIC_INC}
	)
target_link_libraries(football_static
	${FOOTBALL_STATIC_LIBS}
        )
set_target_properties(football_static PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")


#///////////////////////////////////////////////////////////////////////
# pack static lib as a shared lib, then can be dlopen by sfhookerapi.so

add_library(football_shared SHARED
	football_shared.cpp
	)
target_include_directories(football_shared PRIVATE 
	.
	${ANDROID_NDK}/sources/android/native_app_glue
	)
# ${FOOTBALL_STATIC_LIBS} does not contained in football_static
target_link_libraries(football_shared
        ${FOOTBALL_STATIC_LIBS}
	)
add_dependencies(football_shared football_static)
set_target_properties(football_shared PROPERTIES 
    LINK_FLAGS "-Llib -Wl,--whole-archive,-lfootball_static,--no-whole-archive")


#///////////////////////////////////////////////////////////////////////
add_executable(footballd
	footballd.cpp
	)
target_include_directories(footballd PRIVATE 
	.
	./utils
	)
target_link_libraries(footballd
	football_shared
	)
