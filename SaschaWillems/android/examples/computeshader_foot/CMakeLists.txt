cmake_minimum_required(VERSION 3.4.1 FATAL_ERROR)

set(NAME computeshader_foot)

set(EXAMPLE_SRC_DIR ../../../examples/${NAME})
set(BASE_DIR ../../../base)
set(EXTERNAL_DIR ../../../external)

file(GLOB EXAMPLE_SRC "${EXAMPLE_SRC_DIR}/*.cpp")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

message(STATUS EXAMPLE_SRC = ${EXAMPLE_SRC})
add_library(computeshader_foot_static STATIC ${EXAMPLE_SRC})

target_include_directories(computeshader_foot_static PRIVATE 
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

target_link_libraries(computeshader_foot_static
	native-app-glue
	base_static
    android
    log
    z
)	

