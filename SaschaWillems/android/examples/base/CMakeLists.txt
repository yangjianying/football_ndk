file(GLOB BASE_SRC "../../../base/*.cpp" "../../../external/imgui/*.cpp")

add_library(base_static STATIC ${BASE_SRC})

message(STATUS base/CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR})
message(STATUS base/SRC_DIR_SaschaWillems = ${SRC_DIR_SaschaWillems})

#add_library(libassimp STATIC IMPORTED)
#set_target_properties(libassimp PROPERTIES IMPORTED_LOCATION ${SRC_DIR_SaschaWillems}/libs/assimp/${ANDROID_ABI}/libassimp.a)
#
#add_library(libIrrXML STATIC IMPORTED)
#set_target_properties(libIrrXML PROPERTIES IMPORTED_LOCATION ${SRC_DIR_SaschaWillems}/libs/assimp/${ANDROID_ABI}/libIrrXML.a)

message(STATUS base/BASE_DIR = ${BASE_DIR})

target_include_directories(base_static PRIVATE 
	${REPO_ROOT_DIR}
	.
	${SRC_dir_app_standalone}
	${BASE_DIR}
	../../../external/glm
	../../../external/gli
	../../../external/imgui
	../../../external/assimp
	${ANDROID_NDK}/sources/android/native_app_glue
	${ANDROID_NDK}/sources/third_party/shaderc/include
)
target_link_libraries(base_static
    android
    log
    z
#    libassimp    
#    libIrrXML  
)
