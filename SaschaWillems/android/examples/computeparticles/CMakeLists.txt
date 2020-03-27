cmake_minimum_required(VERSION 3.4.1 FATAL_ERROR)

set(NAME computeparticles)

set(SRC_DIR ../../../examples/${NAME})
set(BASE_DIR ../../../base)
set(EXTERNAL_DIR ../../../external)

file(GLOB EXAMPLE_SRC "${SRC_DIR}/*.cpp")


set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

add_library(computeparticles_static STATIC ${EXAMPLE_SRC})

target_include_directories(computeparticles_static PRIVATE 
	${REPO_ROOT_DIR}
	.
	${SRC_dir_app_standalone}
	${BASE_DIR}
	${EXTERNAL_DIR}
	${EXTERNAL_DIR}/glm
	${EXTERNAL_DIR}/gli
	${EXTERNAL_DIR}/imgui
	${EXTERNAL_DIR}/assimp
	${ANDROID_NDK}/sources/android/native_app_glue
	)

target_link_libraries(computeparticles_static
    native-app-glue
    base_static
    android
    log
    z
	)
