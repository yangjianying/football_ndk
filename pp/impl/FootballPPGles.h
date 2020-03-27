#ifndef __FOOTBALL_PP_GLES_H__
#define __FOOTBALL_PP_GLES_H__

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

namespace gles {
namespace session {
namespace impl {

class FootSessionGlesImpl1;

};
};
};


namespace football {


class FootballPPGles: public FootballPP {
public:
	
	FootballPPGles();
	virtual ~FootballPPGles() override;
	virtual int buildSession(int session_type, SessionInfo &session, int *session_id) override;

	static void test1(int numCycle, long wait_ms);

};

class GlesTestRendererWrapper1{
public:
	GlesTestRendererWrapper1(ANativeWindow *window);
	~GlesTestRendererWrapper1();
	void renderFrame(AHardwareBuffer *buffer);
private:
	void *mImpl = nullptr;
};


};


#endif

