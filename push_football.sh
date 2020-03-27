#!/bin/sh

sdm6150_rootOut=.

adb push $sdm6150_rootOut/out_linux_arm/libfootball_shared.so /system/lib/ 
adb push $sdm6150_rootOut/out_linux_arm64/libfootball_shared.so /system/lib64/
adb push $sdm6150_rootOut/out_linux_arm64/footballd /system/bin/

adb shell chmod 777 /system/bin/footballd

echo push done!
