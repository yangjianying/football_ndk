#ifndef __FOOTBALL_PP_VK__H__
#define __FOOTBALL_PP_VK__H__

#include <vector>
#include <iostream>
#include <string>
#include <map>

#include <android/native_window.h>  // ANativeWindow, ANativeWindow_Buffer
#include <android/surface_control.h>
#include <android/hardware_buffer.h>
#include <android/choreographer.h>

#include <media/NdkImage.h>
#include <media/NdkImageReader.h>

#include "FootballPP.h"
#include "utils/ANativeWindowUtils.h"

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
	virtual int buildSession(int session_type, SessionInfo &session, int *session_id) override;


	static void test1(int numCycle, int num_of_frames);
	static void test2(int numCycle, int num_of_frames);

};

};



#endif

