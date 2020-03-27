#ifndef __FOOTSESSION_GLES_IMPL_1_H__
#define __FOOTSESSION_GLES_IMPL_1_H__

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

#include "pp/FootballPP.h"
#include "utils/ANativeWindowUtils.h"


namespace gles {
namespace session {
namespace impl {

class RenderingHandler;

class FootSessionGlesImpl1: public ::football::FootballPP::FootballSession
	, public football::HardwareBufferReader::CB
	, public football::ImageReaderImageListenerWrapper
{
public:
	FootSessionGlesImpl1(football::SessionInfo &session);
	virtual ~FootSessionGlesImpl1();
	bool isValid();
	virtual int setSessionParameter(football::SessionParameter *parameter) override ;
	virtual int getSessionParameter(football::SessionParameter *parameter) override ;
	virtual void print() override ;

	// impl public football::HardwareBufferReader::CB
	virtual int on_process_frame(AHardwareBuffer *hardware_buffer) override;

	// impl football::ImageReaderImageListenerWrapper
	virtual void onImageAvailableCallback(AImageReader *reader) override ;

	football::SessionInfo & mSessionInfo;
	football::SessionParameter mSessionParameter;
	int mId = -1;
	AImageReader *mReader = nullptr;

	RenderingHandler *mRenderingHandler = nullptr;
	football::HardwareBufferReader *mHardwareBufferReader = nullptr;
	std::mutex mRenderingHandler_lock;

};

};
};
};


#endif

