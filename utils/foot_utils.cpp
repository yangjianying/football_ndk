#include <unistd.h>

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <math.h>

#include <sys/stat.h>

#ifdef WIN32
#include <io.h>
#include <direct.h> 
#else
#include <unistd.h>
#include <sys/stat.h>
#endif
#include <stdint.h>
#include <string>

#include <iostream>// for std::cout, std::endl
#include <fstream>
#include <string> // for std::string
#include <vector>// for std::vector
#include <numeric> // for std::accumulate
#include <list>
#include <iomanip>
#include <thread>

#include "FootballConfig.h"


#include "foot_utils.h"

#include <sys/system_properties.h>

#undef __CLASS__
#define __CLASS__ "foot_utils"

namespace football {
namespace utils {

//int __system_property_get(const char* __name, char* __value);
static int property_get(const char* __name, char* __value, const char *default__) {
    const prop_info *pi = __system_property_find(__name);
    if(pi != 0) {
        return __system_property_read(pi, 0, __value);
    } else {
        strlcpy(__value, default__, PROP_VALUE_MAX);
        return 0;
    }
}
bool ProcessBootAnimCompleted() {
	  bool bootanim_exit = false;
	
	  /* All other checks namely "init.svc.bootanim" or
	  * HWC_GEOMETRY_CHANGED fail in correctly identifying the
	  * exact bootup transition to homescreen
	  */
	  char property[PROP_VALUE_MAX];
	  bool isEncrypted = false;
	  bool main_class_services_started = false;
	  property_get("ro.crypto.state", property, "unencrypted");
	  if (!strcmp(property, "encrypted")) {
		property_get("ro.crypto.type", property, "block");
		if (!strcmp(property, "block")) {
		  isEncrypted = true;
		  property_get("vold.decrypt", property, "");
		  if (!strcmp(property, "trigger_restart_framework")) {
			main_class_services_started = true;
		  }
		}
	  }
	
	  property_get("service.bootanim.exit", property, "0");
	  if (!strcmp(property, "1")) {
		bootanim_exit = true;
	  }
	
	  if ((!isEncrypted || (isEncrypted && main_class_services_started)) &&
		  bootanim_exit) {
		  DLOGD( "boot_animation_completed_ \r\n");
#if 0
#ifdef FEATURE_cycling_color_function  // frankie, add
		if (sTestITestColorService != NULL) {
			sTestITestColorService->ProcessBootAnimCompleted();
		}
#endif
		DLOGI("Applying default mode for display %d", sdm_id_);
		boot_animation_completed_ = true;
		// Applying default mode after bootanimation is finished And
		// If Data is Encrypted, it is ready for access.
		if (display_intf_) {
		  display_intf_->ApplyDefaultDisplayMode();
		  RestoreColorTransform();
		}
#endif
	  	return true;
	  }

	DLOGD( "boot_animation_completed_ not true. \r\n");
	return false;
}
void foot_utils_test_system_properties_h() {
	ProcessBootAnimCompleted();
}


// file related
static inline bool exists_test0 (const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}
static inline bool exists_test1 (const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}
static inline bool exists_test2 (const std::string& name) {
    return ( access( name.c_str(), F_OK ) != -1 );
}
static inline bool exists_test3 (const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}
/*static*/ bool CheckFile::exist(const std::string& name) {
	return exists_test3(name);
}


};
};


