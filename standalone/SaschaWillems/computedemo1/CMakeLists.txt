cmake_minimum_required(VERSION 3.4.1 FATAL_ERROR)

set(NAME computedemo1)

set(EXAMPLE_SRC_DIR .)
set(BASE_DIR ${SRC_DIR_SaschaWillems}/base)
set(EXTERNAL_DIR ${SRC_DIR_SaschaWillems}/external)

file(GLOB EXAMPLE_SRC "${EXAMPLE_SRC_DIR}/*.cpp"
	"${EXAMPLE_SRC_DIR}/ppvkimpl/*.cpp")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

message(STATUS EXAMPLE_SRC = ${EXAMPLE_SRC})
add_library(computedemo1_static STATIC ${EXAMPLE_SRC})

target_include_directories(computedemo1_static PRIVATE 
	${REPO_ROOT_DIR}
	${REPO_ROOT_DIR}/pp
	${REPO_ROOT_DIR}/pp/impl
	.
	${SRC_dir_app_standalone}
	${SRC_dir_app_standalone}/android-vulkan-tutorials/tutorial06_texture_v2
	${BASE_DIR}
	${EXTERNAL_DIR}
	${EXTERNAL_DIR}/glm
	${EXTERNAL_DIR}/gli
	${EXTERNAL_DIR}/imgui
	${EXTERNAL_DIR}/assimp
	${ANDROID_NDK}/sources/android/native_app_glue
	)

target_link_libraries(computedemo1_static
	native-app-glue
	base_static
    android
    log
    z
)	

