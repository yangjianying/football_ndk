#include <unistd.h>

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <math.h>

#ifdef WIN32
#include <io.h>
#include <direct.h> 
#else
#include <unistd.h>
#include <sys/stat.h>
#endif
#include <stdint.h>
#include <string>

#if 0  // 

#include <EGL/egl.h>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>

#include <GLES2/gl2.h>

// Include the latest possible header file( GL version header )
#if __ANDROID_API__ >= 24
#include <GLES3/gl32.h>
#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2ext.h>
#include <GLES3/gl3ext.h>

#endif

#include <android/log.h>
#include <android_native_app_glue.h>

#include "FootballConfig.h"

#include "MemTrace.h"
#include "StbImage_.h"

#include "FootballPPTester.h"

#include "ndk_extend/NativeHooApi_Loader.h"

#define LOG_TAG "FootballPPVk"
#include "android_logcat_.h"

#include "ppvkimpl/FootSessionVkImpl.h"

#include "FootballPPVk.h"

#undef __CLASS__
#define __CLASS__ "FootballPPVk"

#undef UNKNOWN_ERROR
#undef NO_ERROR
#define UNKNOWN_ERROR (-1)
#define NO_ERROR (0)

extern void standalone_main_run_all();

using namespace computedemo1::impl;

namespace football {

FootballPPVk::FootballPPVk() {
	DLOGD( "%s,%d \r\n", __func__, __LINE__);
}
FootballPPVk::~FootballPPVk() {
	DLOGD( "%s,%d \r\n", __func__, __LINE__);
	// should close all sessions
}
int FootballPPVk::buildSession(int session_type, SessionInfo &session, int *session_id) {
	DLOGD( "FootballPPVk::%s,  session_type: %d \r\n", __func__, session_type);

	FootSessionVkImpl * session_ = new FootSessionVkImpl(session_type, session);
	if (!session_->isValid()) {
		delete session_;
		DLOGD( "%s,%d not valid\r\n", __func__, __LINE__);
		return -1;
	}
	session_->mId = addSession(session_);
	*session_id = session_->mId;
	DLOGD( "%s id: %d \r\n", __func__, session_->mId);
	return 0;
}


/*static*/ void FootballPPVk::test1(int numCycle, int num_of_frames) {

	for(int c=0;c<numCycle;c++) {

	struct android_app __android_app = {0};
	__android_app.window = nullptr;

	int ret = 0;
	ANativeHooSurface *hooSurface = nullptr;
	ret = ANativeHooSurface_create("", 1080, 1920, AIMAGE_FORMAT_RGBA_8888, 0, &hooSurface);
	if (ret == 0 && hooSurface != nullptr) {
		DLOGD( "create surface ok . \r\n");
	
		ANativeWindow *inputWindow = nullptr;
		ANativeHooSurface_getWindow(hooSurface, &inputWindow);
		if (inputWindow != nullptr) {
			__android_app.window = inputWindow;

		#if 0
			// *** here test failed , when called many times , the system hangup !!!
			InitVulkan(&__android_app);
			
			long frame_num = 0;
			while(frame_num < num_of_frames) {
				if(IsVulkanReady()) {
					frame_num++;
					VulkanDrawFrame();
				}
				//usleep(16*1000);
			}
			
			DeleteVulkan();

		#else
			fill_ANativeWindow_with_color(inputWindow, 0xffff0000);
		#endif
		}

		ANativeHooSurface_destroy(hooSurface);
	}

	}

}


/*static*/ void FootballPPVk::test2(int numCycle, int num_of_frames) {
	standalone_main_run_all();
}


};

