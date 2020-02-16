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

message(STATUS ANDROID_PLATFORM_LEVEL = ${ANDROID_PLATFORM_LEVEL})
message(STATUS CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR})
message(STATUS __ANDROID_API__ = ${__ANDROID_API__})

set(SRC_DIR ${CMAKE_SOURCE_DIR})
set(READLINE_DIR ${SRC_DIR}/readline)
set(NCURSES_DIR ${SRC_DIR}/ncurses)

if (${ANDROID_ABI} STREQUAL "arm64-v8a")
set(READLINE_INSTALL_DIR ${READLINE_DIR}/readline-8.0_arm64/install_)
set(NCURSES_INSTALL_DIR ${NCURSES_DIR}/ncurses-6.1_arm64/install_)
endif()
if (${ANDROID_ABI} STREQUAL "armeabi-v7a")
set(READLINE_INSTALL_DIR ${READLINE_DIR}/readline-8.0_arm/install_)
set(NCURSES_INSTALL_DIR ${NCURSES_DIR}/ncurses-6.1_arm/install_)
endif()

message(STATUS READLINE_DIR = ${READLINE_DIR})
message(STATUS READLINE_INSTALL_DIR = ${READLINE_INSTALL_DIR})

get_filename_component(REPO_ROOT_DIR ${CMAKE_SOURCE_DIR} ABSOLUTE)

message(STATUS REPO_ROOT_DIR = ${REPO_ROOT_DIR})

# -Werror \
# -Wno-unused-variable \
# -Wno-delete-non-virtual-dtor \
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
       -std=c++14 \
       -fexceptions \
       -fcxx-exceptions \
       -Wall")


# -Wunreachable-code
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

#///////////////////////////////////////////////////////////////////////
# build native_app_glue as a static lib

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -DVK_USE_PLATFORM_ANDROID_KHR -DVK_NO_PROTOTYPES")

if (${ANDROID_ABI} STREQUAL "armeabi-v7a")
   set(CMAKE_CXX_FLAGS} "${CMAKE_CXX_FLAGS} \
       -mhard-float
       -D_NDK_MATH_NO_SOFTFP=1
       -mfloat-abi=hard")
   set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} \
       -Wl,--no-warn-mismatch")
endif()

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
	-DLUA_USE_LINUX \
	-Wno-implicit-function-declaration")

add_definitions(-Wno-deprecated)


add_library(libhistory STATIC IMPORTED)
set_target_properties(libhistory PROPERTIES
        IMPORTED_LOCATION ${READLINE_INSTALL_DIR}/lib/libhistory.a)

add_library(libreadline STATIC IMPORTED)
set_target_properties(libreadline PROPERTIES
        IMPORTED_LOCATION ${READLINE_INSTALL_DIR}/lib/libreadline.a)

add_library( # Sets the name of the library.
             luajit

             # Sets the library as a shared library.
             STATIC

             # Provides a relative path to your source file(s).
             src/lapi.c
             src/lauxlib.c
             src/lbaselib.c
             src/lbitlib.c
             src/lcode.c
             src/lcorolib.c
             src/lctype.c
             src/ldblib.c
             src/ldebug.c
             src/ldo.c
             src/ldump.c
             src/lfunc.c
             src/lgc.c
             src/linit.c
             src/liolib.c
             src/llex.c
             src/lmathlib.c
             src/lmem.c
             src/loadlib.c
             src/lobject.c
             src/lopcodes.c
             src/loslib.c
             src/lparser.c
             src/lstate.c
             src/lstate.h
             src/lstring.c
             src/lstrlib.c
             src/ltable.c
             src/ltablib.c
             src/ltm.c
             src/lua_ext.c
             src/luac_ext.c
             src/lua.hpp
             src/lundump.c
             src/lutf8lib.c
             src/lvm.c
             src/lzio.c
			 )


target_include_directories(luajit PRIVATE 
	.
	src/
	${READLINE_INSTALL_DIR}/include
	)
target_link_libraries(luajit
            android
            log
            m
            z
            libhistory
            libreadline
            )

#///////////////////////////////////////////////////////////////////////

#
add_library(libncurses STATIC IMPORTED)
set_target_properties(libncurses PROPERTIES
        IMPORTED_LOCATION ${NCURSES_INSTALL_DIR}/lib/libncurses.a)

add_executable( # Sets the name of the library.
             lua
             src/lua.c
	)

target_include_directories(lua PRIVATE 
	.
	src/
	${READLINE_INSTALL_DIR}/include
	)
target_link_libraries(lua
            android
            log
            m
            z
            luajit
            libhistory
            libreadline
            libncurses
            )
 
#///////////////////////////////////////////////////////////////////////

