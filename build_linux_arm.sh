#!/bin/bash

export PATH=/home/yangjianying/Android/Sdk/cmake/3.6.4111459/bin:$PATH
export ANDROID_NDK_HOME=/home/yangjianying/Android/Sdk/ndk-bundle

ASSIMP_DIR=.
OUTPUT_DIR=out_linux_arm

rm -rf $OUTPUT_DIR
mkdir $OUTPUT_DIR
cd $OUTPUT_DIR

# cmake ANDROID_STL=c++_shared is for old code !!!

if [[ “$@“ =~ "-d" ]];then
        echo "----------------------------cmake debug----------------------------"
cmake -DDEBUG=ON \
		-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
      -DANDROID_NDK=$ANDROID_NDK_HOME \
      -DANDROID_ABI=armeabi-v7a \
      -DANDROID_TOOLCHAIN=clang \
      -DCMAKE_INSTALL_PREFIX=install \
      -DANDROID_NATIVE_API_LEVEL=28 \
      -DANDROID_FORCE_ARM_BUILD=TRUE \
      -DANDROID_STL=c++_static \
      -DANDROID_PLATFORM=android-28 \
	 	../${ASSIMP_DIR}
else      
        echo "----------------------------cmake release----------------------------"
cmake -DDEBUG=NO \
		-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
      -DANDROID_NDK=$ANDROID_NDK_HOME \
      -DANDROID_ABI=armeabi-v7a \
      -DANDROID_TOOLCHAIN=clang \
      -DCMAKE_INSTALL_PREFIX=install \
      -DANDROID_NATIVE_API_LEVEL=28 \
      -DANDROID_FORCE_ARM_BUILD=TRUE \
      -DANDROID_STL=c++_static \
      -DANDROID_PLATFORM=android-28 \
	 	../${ASSIMP_DIR}
fi
	  
make 
 
cd ..

echo done!

