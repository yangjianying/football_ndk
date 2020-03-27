@echo off
cls

REM *NOTE* Change these based on 
SET LUA_SRC_DIR=.
SET OUTPUT_DIR=lua-build-arm64v8a
SET ANDROID_PATH=C:\Users\hp\AppData\Local\Android\Sdk
SET NDK_PATH=C:\Users\hp\AppData\Local\Android\Sdk\ndk\20.1.5948944
SET NDK_TOOLCHAIN=D:\vulkan\SaschaWillems\SaschaWillems\android_standalong_toolchain\android-toolchain-24-llvm-arm64v8a
SET CMAKE_TOOLCHAIN=%NDK_PATH%\build\cmake\android.toolchain.cmake
SET CMAKE_PATH=%ANDROID_PATH%\cmake\3.6.4111459

REM *NOTE* Careful if you don't want rm -rf, I use it for testing purposes.
rmdir /s/q %OUTPUT_DIR%
mkdir %OUTPUT_DIR%

REM pushd doesn't seem to work ):<
cd %OUTPUT_DIR%

if not defined ORIGPATH set ORIGPATH=%PATH%
SET PATH=%CMAKE_PATH%\bin;%ANDROID_PATH%\tools;%ANDROID_PATH%\platform-tools;%ORIGPATH%

cmake ^
      -GNinja ^
      -DCMAKE_TOOLCHAIN_FILE=%CMAKE_TOOLCHAIN% ^
      -DASSIMP_ANDROID_JNIIOSYSTEM=ON ^
      -DANDROID_NDK=%NDK_PATH% ^
      -DCMAKE_MAKE_PROGRAM=%CMAKE_PATH%\bin\ninja.exe ^
      -DCMAKE_BUILD_TYPE=Release ^
      -DANDROID_ABI="arm64-v8a" ^
      -DANDROID_NATIVE_API_LEVEL=26 ^
      -DANDROID_FORCE_ARM_BUILD=TRUE ^
      -DCMAKE_INSTALL_PREFIX=install ^
      -DANDROID_STL=c++_static^
      -DCMAKE_CXX_FLAGS=-Wno-c++11-narrowing ^
      -DANDROID_TOOLCHAIN=clang ^
      -DASSIMP_BUILD_TESTS=OFF ^

      ../%LUA_SRC_DIR%

cmake --build .

REM ninja install

cd ..

REM pause
