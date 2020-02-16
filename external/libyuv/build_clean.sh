#!/bin/bash

export ANDROID_NDK_HOME=/home/yangjianying/Android/Sdk/ndk-bundle
export PATH=${ANDROID_NDK_HOME}:$PATH
#export LIBYUV_DISABLE_JPEG=yes
export LIBYUV_DISABLE_JPEG="yes"
export APP_ALLOW_MISSING_DEPS=true
ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk APP_PLATFORM=android-16 clean

