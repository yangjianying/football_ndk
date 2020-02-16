

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

namespace computedemo1 {
namespace impl {

class RenderingHandler;
class HardwareBufferReader;

class FootSessionVkImpl: public football::ImageReaderImageListenerWrapper {
public:
	FootSessionVkImpl(football::FootSession *session);
	virtual ~FootSessionVkImpl();
	bool isValid();
	int setSessionParameter(football::SessionParameter *parameter);
	int getSessionParameter(football::SessionParameter *parameter);
	void print();

	void processFrame(AHardwareBuffer *buffer);
	virtual void onImageAvailableCallback(AImageReader *reader);

	football::FootSession mFootSession;
	football::SessionParameter mSessionParameter;
	int mId = -1;
	AImageReader *mReader = nullptr;

	RenderingHandler *mRenderingHandler = nullptr;
	HardwareBufferReader *mHardwareBufferReader = nullptr;
	std::mutex mRenderingHandler_lock;

};

};
};

