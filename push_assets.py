#!/usr/bin/python
#
# Copyright 2018 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Main entry point for all of acloud's unittest."""

from importlib import import_module
import logging
import os
import sys
import time
#import unittest
import os,subprocess
import re

def generate_mbi6322_config_header():
    c_header_filename_ = r"mbi6322_reference_settings.h";
    filename_ = r"DataGridView-for TCL-20190916 .demo";
    if not os.path.exists(filename_):
        print "error: ", filename_, "not exists!"
        exit(-2)
    print "Hello,world"

    header_file = open("" + c_header_filename_, "w");
    header_file_c_macro = "__" + c_header_filename_.replace(".", "_").upper() + "_"
    header_file.write("/* auto generated file, DO NOT edit this file !!! */ \r\n")
    header_file.write("/* auto generated file, DO NOT edit this file !!! */ \r\n")
    header_file.write("/* auto generated file, DO NOT edit this file !!! */ \r\n\r\n")
    header_file.write("#ifndef " + header_file_c_macro + "\r\n")
    header_file.write("#define " + header_file_c_macro + "\r\n")
    header_file.write("\r\n")

    byte_array_name = "mbi6322_initial_settings"

    header_file.write("const uint16_t " + byte_array_name + "[] = {" + "\r\n")

    reg_count = 0
    addr_last = -1
    f = open(filename_)
    line = f.readline()
    while line: 
        line=line.strip('\n')
        line=line.strip('\r')
        #print line
        b1 = line.split('\t');
        #print b1[2], ":", b1[3]
        addr_ = int(b1[2]);
        header_file.write("0x%04x"%(addr_))
        header_file.write(", " + "0x" + b1[3] + ", ")
        header_file.write("    // " + b1[2] + " \r\n")
        
        if addr_last > 0:
            if (addr_ - addr_last) > 1:
                print("addr : ", addr_last + 1, " ~ ", addr_ - 1, " empty !!!")
        addr_last = addr_
        reg_count = reg_count + 1
        #print(line, end = '')
        line = f.readline()
    f.close()

    header_file.write("};\r\n")

    header_file.write("const int " + byte_array_name + "_reg_size = ");
    header_file.write("%d ; \r\n"%(reg_count))

    header_file.write("\r\n")
    header_file.write("#endif")
    header_file.close()

    print "reg_count:", reg_count
    print "Done!"

print "Hello,world!"

def connectDevcie():
    try:
        deviceInfo= subprocess.check_output('adb devices').split("\r\n")
        if deviceInfo[1]=='':
            return False
        else:
            return True
    except Exception,e:
        print "Device Connect Fail:",e
     
def getAndroidVersion():
    try:
        if connectDevcie():   
            sysInfo= subprocess.check_output('adb shell cat /system/build.prop')
            androidVersion=re.findall("version.release=(\d\.\d)*",sysInfo , re.S)[0]
            return  androidVersion
        else:
            return "Connect Fail,Please reconnect Device..."
    except Exception,e:
        print "Get Android Version:",e
 
def getDeviceName(): 
    try:
        if connectDevcie():
            deviceInfo= subprocess.check_output('adb devices -l')
            deviceName=re.findall(r'device product:(.*)\smodel',deviceInfo,re.S)[0]
            return  deviceName
        else:
            return "Connect Fail,Please reconnect Device..."
    except Exception,e:
        print "Get Device Name:",e
 
print "androidVersion:", getDeviceName(),"\n", "deviceName:", getAndroidVersion()


list = [
("standalone/android-vulkan-tutorials/tutorial06_texture_v2/assets", "/sdcard/data/tutorial06_texture/"),
("standalone/SaschaWillems/computedemo1/assets", "/sdcard/data/SaschaWillems/computedemo1/"),
("SaschaWillems/android/examples/computeshader_foot/assets", "/sdcard/data/SaschaWillems/computeshader_foot/"),
("SaschaWillems/android/examples/computeshader/assets", "/sdcard/data/SaschaWillems/computeshader/"),
("SaschaWillems/android/examples/vulkanscene/assets", "/sdcard/data/SaschaWillems/vulkanscene/"),
("SaschaWillems/android/examples/computeparticles/assets", "/sdcard/data/SaschaWillems/computeparticles/"),
("SaschaWillems/android/examples/computeraytracing/assets", "/sdcard/data/SaschaWillems/computeraytracing/"),
("SaschaWillems/android/examples/computecloth/assets", "/sdcard/data/SaschaWillems/computecloth/"),
("SaschaWillems/android/examples/computecullandlod/assets", "/sdcard/data/SaschaWillems/computecullandlod/"),
]

for item__ in list:
    print "push ", item__[0], "\n  into ", item__[1]
    subprocess.check_output('adb shell mkdir -p ' + item__[1])
    subprocess.check_output('adb shell rm -rf ' + item__[1] + "*")
    subprocess.check_output('adb push ' + item__[0] + ' ' + item__[1])


#input()
#os.system("pause");
#time.sleep(1);

#if __name__ == '__main__':
#    main(sys.argv[1:])
