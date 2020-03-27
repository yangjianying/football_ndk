

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


namespace computedemo1 {
namespace impl {

class RenderingHandler;
class HardwareBufferReader;

class FootSessionVkImpl: 
	public ::football::FootballPP::FootballSession
	, public football::HardwareBufferReader::CB
	, public football::ImageReaderImageListenerWrapper 
	{
public:
	FootSessionVkImpl(int type_, ::football::SessionInfo &session);
	virtual ~FootSessionVkImpl();
	bool isValid();
	
	virtual int setSessionParameter(football::SessionParameter *parameter) override ;
	virtual int getSessionParameter(football::SessionParameter *parameter) override ;
	virtual void print() override ;

	// impl public football::HardwareBufferReader::CB
	virtual int on_process_frame(AHardwareBuffer *hardware_buffer) override;

	// iml public football::ImageReaderImageListenerWrapper 
	virtual void onImageAvailableCallback(AImageReader *reader) override ;

	int mType_ = 0;
	football::SessionInfo & mSessionInfo;
	football::SessionParameter mSessionParameter;
	int mId = -1;
	AImageReader *mReader = nullptr;
	football::HardwareBufferReader *mHardwareBufferReader = nullptr;

	RenderingHandler *mRenderingHandler = nullptr;
	std::mutex mRenderingHandler_lock;

};

};
};

