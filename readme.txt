


#///////////////////////////////////////////////////////////////////////
add_library(libshaderc STATIC IMPORTED)
set_target_properties(libshaderc PROPERTIES
        IMPORTED_LOCATION ${ANDROID_NDK}/sources/third_party/shaderc/libs/c++_static/${ANDROID_ABI}/libshaderc.a)
		

-- Build files have been written to: G:/tcl/football/stage_code_20200210/football_20200210/football/out_arm64v8a
ninja: error: 'C:/Users/hp/AppData/Local/Android/Sdk/ndk/20.1.5948944/sources/third_party/shaderc/libs/c++_static/arm64-v8a/libshaderc.a', needed by 'test_football', missing and no known rule to make it



http://www.mobibrw.com/2018/16065
// 编译 : C:\Users\hp\AppData\Local\Android\Sdk\ndk\20.1.5948944\sources\third_party\shaderc

# ${ndk.dir} 为NDK的安装目录
$ cd ${ndk.dir}/sources/third_party/shaderc/
 
# 这一步的编译特别耗时，好消息是只需要执行一次就可以
$ ../../../ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk APP_STL:=c++_shared APP_ABI=all libshaderc_combined
 
# 这一步就非常快了
$ ../../../ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk APP_STL:=c++_static APP_ABI=all libshaderc_combined






