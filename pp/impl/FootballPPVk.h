#ifndef __FOOTBALL_PP_VK__H__
#define __FOOTBALL_PP_VK__H__

#include<vector>
#include <iostream>
#include <string>
#include <map>

#include <android/native_window.h>  // ANativeWindow, ANativeWindow_Buffer
#include <android/surface_control.h>
#include <android/hardware_buffer.h>
#include <android/choreographer.h>

#include <media/NdkImage.h>
#include <media/NdkImageReader.h>
#if 0
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaCrypto.h>
#include <media/NdkMediaDataSource.h>
#include <media/NdkMediaDrm.h>
#include <media/NdkMediaError.h>
#include <media/NdkMediaExtractor.h>
#include <media/NdkMediaFormat.h>
#include <media/NdkMediaMuxer.h>
#endif

#include "FootballPP.h"
#include "ANativeWindowUtils.h"

namespace computedemo1 {
namespace impl {
	class FootSessionVkImpl;
};
};


namespace football {


class FootballPPVk: public FootballPP {
public:
	
	FootballPPVk();
	virtual ~FootballPPVk() override;
	virtual int buildSession(FootSession *session, int *session_id) override;
	virtual int closeSession(int session_id) override;
	virtual int setSessionParameter(int session_id, SessionParameter *parameter) override;
	virtual int getSessionParameter(int session_id, SessionParameter *parameter) override;
	virtual std::vector<int> getSessionIds() override;
	virtual int getSession(int session_id, FootSession *session) override;
	virtual void print(int session_id) override;

	std::vector<int> mSessionIds;
	std::map<int, computedemo1::impl::FootSessionVkImpl*> mSessions;

	static int s_SessionId_generator;

	static void test1(int numCycle, int num_of_frames);
	static void test2(int numCycle, int num_of_frames);

};

};



#endif

