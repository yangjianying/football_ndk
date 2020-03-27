#!/usr/bin/python


from importlib import import_module
import logging
import os
import sys
import time
#import unittest
import os,subprocess
import re
from string import Template

# usage:
# python generate_loader.py FootballSysApi
# 1, check if FootballSysApi.h file is exist
# 2, check if FootballSysApi.proto.inc file is exist
# 3, create FootballSysApiLoader.cpp, FootballSysApi.h files
# 4, FootballSysApi -> FootballSysApi, footballsysapi, FOOTBALLSYSAPI, FootballSysApi_NO_PROTOTYPES
#      FOOTBALLSYSAPI_ptr, FOOTBALLSYSAPI_PF, FOOTBALLSYSAPI_DECLARE
#      

if len(sys.argv) <= 1:
    print "error: should input a header filename and a target library name!"
    exit(-1)
if len(sys.argv) <= 2:
    print "error: should input target library name!"
    exit(-1)
if len(sys.argv) > 3:
    print "error: should only input a header filename and a target library name!"
    exit(-1)
apitarget_lib_name = sys.argv[2]
filename_ = sys.argv[1]
header_filename = filename_ + ".h"
proto_filename = filename_ + ".proto.inc"
if not os.path.exists(header_filename):
    print "error: ", header_filename, "not exists!"
    exit(-2)
if not os.path.exists(proto_filename):
    print "error: ", proto_filename, "not exists!"
    exit(-2)

loader_filename_cpp = filename_ + "_Loader.cpp"
loader_filename_h = filename_ + "_Loader.h"
loader_filename_h_extra = filename_ + "_extra.h"
loader_filename_h_extra_code_snippets_template = Template(r'''
#include "${api_name}_extra.h"
''');
loader_filename_h_extra_code_snippets_content = ""

if os.path.exists(loader_filename_cpp):
    print loader_filename_cpp, " already exist, delete first!"
    os.remove(loader_filename_cpp)
if os.path.exists(loader_filename_h):
    print loader_filename_h, " already exist, delete first!"
    os.remove(loader_filename_h)



print "Hello,world"

apiname = filename_
apiname_upper = filename_.upper();
apiname_lower = filename_.lower();

if os.path.exists(loader_filename_h_extra):
    loader_filename_h_extra_code_snippets_content = loader_filename_h_extra_code_snippets_template.substitute(api_name=apiname)

# api_name
# api_name_upper
# api_name_lower
# api_target_lib_name
# extra_include_code_snippets
template_header = Template(r'''
/** auto generated, do not edit !!! **/
/** auto generated, do not edit !!! **/
/** auto generated, do not edit !!! **/

#ifndef __${api_name_upper}_LOADER_H__
#define __${api_name_upper}_LOADER_H__

${extra_include_code_snippets}

#define ${api_name}_NO_PROTOTYPES
#include "${api_name}.h"

/* here define a function pointer variable the same as the 'a' , then use this a as the function 
 * else we should use another macro to re-direct the normal 'a' call to ${api_name_upper}_ptr(a) */
 
#define ${api_name_upper}_ptr(a) a

#define ${api_name_upper}_PF(a) ${api_name_upper}_PF_##a##_1


#define ${api_name_upper}_DECLARE(func___name, r, ...); \
	typedef r (* ${api_name_upper}_PF(func___name))(__VA_ARGS__);  \
	extern ${api_name_upper}_PF(func___name) ${api_name_upper}_ptr(func___name);

// 1,
#define API_PROTO ${api_name_upper}_DECLARE
#include "${api_name}.proto.inc"
#undef API_PROTO

//
namespace ${api_name_lower} {
namespace loader {
	int ${api_name}_initialize();
	int ${api_name}_uninitialize();
};
};

#define ${api_name}_INITIALIZE ::${api_name_lower}::loader::${api_name}_initialize
#define ${api_name}_UNINITIALIZE ::${api_name_lower}::loader::${api_name}_uninitialize


#endif

''')

template_cpp = Template(r'''
/** auto generated, do not edit !!! **/
/** auto generated, do not edit !!! **/
/** auto generated, do not edit !!! **/
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <stddef.h>
#include <dlfcn.h>

#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/wait.h>  // wait()

#include "FootballConfig.h"
#undef __CLASS__
#define __CLASS__ "${api_name}"

#define ${api_name_upper}_LOADER_SORUCE
#include "${api_name}_Loader.h"

#define ${api_name_upper}_symbol(a) #a

#define API_LOAD(func___name, r, ...); \
	${api_name_upper}_ptr(func___name) = \
		(${api_name_upper}_PF(func___name)) \
			dlsym(__api_lib, ${api_name_upper}_symbol(func___name)); \
	if (${api_name_upper}_ptr(func___name) == nullptr) { \
		DLOGD( "%s dlsym failed! \r\n", ${api_name_upper}_symbol(func___name)); \
	} else { \
		/*DLOGD( "%s dlsym ok! \r\n", ${api_name_upper}_symbol(func___name));*/ \
	}

#define API_IMPL(func___name, r, ...); \
	${api_name_upper}_PF(func___name) ${api_name_upper}_ptr(func___name) = nullptr;


// 2
#define API_PROTO API_IMPL
#include "${api_name}.proto.inc"
#undef API_PROTO

namespace ${api_name_lower} {
namespace loader {
	static void *__api_lib = NULL;
	int ${api_name}_initialize() {
		if (__api_lib != nullptr) {
			/*DLOGD( "${api_target_lib_name} already loaded ! \r\n");*/
			return 0;		
		}
		__api_lib = dlopen("${api_target_lib_name}", RTLD_NOW);
		if (__api_lib == nullptr) { 
			const char* error = dlerror();
			DLOGD( "%s dlopen ${api_target_lib_name}  error:%s \r\n", __func__, error);
			abort();
			return -1;
		}
		/*DLOGD( "${api_target_lib_name} load ok ! \r\n");*/
		
		// 3
		#define API_PROTO API_LOAD
		#include "${api_name}.proto.inc"
		#undef API_PROTO
		
		return 0;
	}
	int ${api_name}_uninitialize() {
		if (__api_lib == nullptr) {
			DLOGD( "${api_target_lib_name} NOT loaded ! \r\n");
			return -1;
		}
		dlclose(__api_lib);
		__api_lib = nullptr;
		/*DLOGD( "${api_target_lib_name} close ok ! \r\n");*/
		return 0;
	}
};
};




''')

# api_name
# api_name_upper
# api_name_lower
# api_target_lib_name
print apiname
print apiname_upper
print apiname_lower
print apitarget_lib_name


header_content = template_header.substitute(\
        api_name=apiname, \
        api_name_upper=apiname_upper, \
        api_name_lower=apiname_lower, \
        api_target_lib_name=apitarget_lib_name, \
        extra_include_code_snippets=loader_filename_h_extra_code_snippets_content)

cpp_content = template_cpp.substitute(\
    api_name=apiname, \
    api_name_upper=apiname_upper, \
    api_name_lower=apiname_lower, \
    api_target_lib_name=apitarget_lib_name)

out_header_file = open("./" + loader_filename_h, "w");
out_header_file.write(header_content)
out_header_file.close();

out_cpp_file = open("./" + loader_filename_cpp, "w");
out_cpp_file.write(cpp_content)
out_cpp_file.close();

print "ok!"

#if __name__ == '__main__':
#    main(sys.argv[1:])

