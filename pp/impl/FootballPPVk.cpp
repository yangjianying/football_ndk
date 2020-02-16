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

#include "MemTrace.h"
#include "StbImage_.h"

#include "FootballPPTester.h"

#include "ndk_extend/NativeHooApi.h"

#define LOG_TAG "FootballPPVk"
#include "android_logcat_.h"

#include "ppvkimpl/FootSessionVkImpl.h"

#include "FootballPPVk.h"

#undef UNKNOWN_ERROR
#undef NO_ERROR
#define UNKNOWN_ERROR (-1)
#define NO_ERROR (0)

extern void standalone_main_run_all();

using namespace computedemo1::impl;

namespace football {

/*static*/ int FootballPPVk::s_SessionId_generator = 0;

FootballPPVk::FootballPPVk() {
	fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
}
FootballPPVk::~FootballPPVk() {
	fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	// should close all sessions
}
int FootballPPVk::buildSession(FootSession *session, int *session_id) {
	fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);

	FootSessionVkImpl * session_ = new FootSessionVkImpl(session);
	if (!session_->isValid()) {
		delete session_;
		fprintf(stderr, "%s,%d not valid\r\n", __func__, __LINE__);
		return -1;
	}
	session_->mId = ++s_SessionId_generator;
	mSessionIds.push_back(session_->mId);
	mSessions.insert(std::pair<int, FootSessionVkImpl*>(session_->mId, session_)); // *** Note: if the key is exist , insert will failed !!!
	*session_id = session_->mId;
	fprintf(stderr, "%s id: %d \r\n", __func__, session_->mId);
	return 0;
}
int FootballPPVk::closeSession(int session_id) {
	// should wait this session's processing is finished !
	int found;
	fprintf(stderr, "closeSession id:%d \r\n", session_id);

	{
		found = 0;
		std::map<int, FootSessionVkImpl*>::iterator iter2;
		/*
		for (iter2 = mSessions.begin(); iter2 != mSessions.end(); iter2++) {
			//cout << iter2->first << ": " << iter2->second << endl;
			if (iter2->first == session_id) {
				FootSessionVkImpl* session_ = (FootSessionVkImpl*)iter2->second;
			}
		}
		*/
		iter2 = mSessions.find(session_id);
		if (iter2 != mSessions.end()) {
			found = 1;
			fprintf(stderr, "mSessions %d found \r\n", session_id);
			FootSessionVkImpl* session_ = (FootSessionVkImpl*)iter2->second;
			mSessions.erase(session_id);
			delete session_; // ### delete ###
		}
		
	}

	{
		found = 0;
		std::vector<int>::iterator iter1 = std::find(mSessionIds.begin(),mSessionIds.end(), session_id);
		if(iter1 != mSessionIds.end()) {
			mSessionIds.erase(iter1);
			found = 1;
			fprintf(stderr, "mSessionIds %d found \r\n", session_id);
		}
	}



	return 0;
}
int FootballPPVk::setSessionParameter(int session_id, SessionParameter *parameter) {
	std::map<int, FootSessionVkImpl*>::iterator iter2;
	iter2 = mSessions.find(session_id);
	if (iter2 != mSessions.end()) {
		FootSessionVkImpl* session_ = (FootSessionVkImpl*)iter2->second;
		session_->setSessionParameter(parameter);
	}
	return -1;
}
int FootballPPVk::getSessionParameter(int session_id, SessionParameter *parameter) {
	std::map<int, FootSessionVkImpl*>::iterator iter2;
	iter2 = mSessions.find(session_id);
	if (iter2 != mSessions.end()) {
		FootSessionVkImpl* session_ = (FootSessionVkImpl*)iter2->second;
		session_->getSessionParameter(parameter);
	}
	return -1;
}

std::vector<int> FootballPPVk::getSessionIds() {
	return mSessionIds;
}
int FootballPPVk::getSession(int session_id, FootSession *session) {
	return 0;
}
void FootballPPVk::print(int session_id) {
	std::map<int, FootSessionVkImpl*>::iterator iter2;
	iter2 = mSessions.find(session_id);
	if (iter2 != mSessions.end()) {
		FootSessionVkImpl* session_ = (FootSessionVkImpl*)iter2->second;
		session_->print();
	}
}

/*static*/ void FootballPPVk::test1(int numCycle, int num_of_frames) {

	for(int c=0;c<numCycle;c++) {

	struct android_app __android_app = {0};
	__android_app.window = nullptr;

	int ret = 0;
	ANativeHooSurface *hooSurface = nullptr;
	ret = ANativeHooSurface_create("", 1080, 1920, 0x01, 0, &hooSurface);
	if (ret == 0 && hooSurface != nullptr) {
		fprintf(stderr, "create surface ok . \r\n");
	
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

