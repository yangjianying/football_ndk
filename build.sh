#!/bin/bash

export PATH=/home/yangjianying/Android/Sdk/cmake/3.6.4111459/bin:$PATH
export ANDROID_NDK_HOME=/home/yangjianying/Android/Sdk/ndk-bundle

rm -rf CMakeCache.txt
rm -rf CMakeFiles
rm -rf cmake_install.cmake
rm -rf Makefile
rm -rf CTestTestfile.cmake

# cmake ANDROID_STL=c++_shared is for old code !!!

if [[ “$@“ =~ "-d" ]];then
        echo "----------------------------cmake debug----------------------------"
cmake -DDEBUG=ON \
		-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
      -DANDROID_NDK=$ANDROID_NDK_HOME \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_TOOLCHAIN=clang \
      -DCMAKE_INSTALL_PREFIX=install \
      -DANDROID_NATIVE_API_LEVEL=26 \
      -DANDROID_FORCE_ARM_BUILD=TRUE \
      -DANDROID_STL=c++_static \
      -DANDROID_PLATFORM=android-29 \
	  .   
else      
        echo "----------------------------cmake release----------------------------"
cmake -DDEBUG=NO \
		-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
      -DANDROID_NDK=$ANDROID_NDK_HOME \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_TOOLCHAIN=clang \
      -DCMAKE_INSTALL_PREFIX=install \
      -DANDROID_NATIVE_API_LEVEL=26 \
      -DANDROID_FORCE_ARM_BUILD=TRUE \
      -DANDROID_STL=c++_static \
      -DANDROID_PLATFORM=android-29 \
	  .  
fi
	  
make 
 
rm -rf CMakeCache.txt
rm -rf CMakeFiles
rm -rf cmake_install.cmake
rm -rf Makefile
rm -rf CTestTestfile.cmake

