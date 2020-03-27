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

#include <android/native_activity.h>

#include "FootballConfig.h"

#include "standalone/native_app_glue.h"
#include "standalone/AAssetManagerImpl_.h"

#include "utils/MemTrace.h"
#include "utils/StbImage_.h"

#include "pp/FootballPPTester.h"

#include "FootballPPUtils.h"

#include "FootballPPGles.h"
#include "FootSessionGlesImpl1.h"

#undef __CLASS__
#define __CLASS__ "FootSessionGlesImpl1"

using namespace football;


namespace gles {
namespace session {
namespace impl {


class Renderer_ {
public:
	static Renderer_ *build(FootSessionGlesImpl1 *vkImpl, struct android_app_ *app);
	struct android_app_ *mApp = nullptr;
	Renderer_(struct android_app_ *app):mApp(app) {}
	virtual ~Renderer_() {
	}
	virtual void processFrame(AHardwareBuffer *buffer) = 0;
};

class Renderer_Impl1: public Renderer_ {
/** test failed when many times called, result the framework to restart !!!
*/
public:
	FootSessionGlesImpl1 *mSessionImpl = nullptr;
	GlesTestRendererWrapper1 *mTestWrapper = nullptr;
	Renderer_Impl1(FootSessionGlesImpl1 *vkImpl, struct android_app_ *app):
		Renderer_(app), mSessionImpl(vkImpl) {
		::android_facade::AAssetManagerImpl_setAssetRootPath("tutorial06_texture/assets/");

		mTestWrapper = new GlesTestRendererWrapper1(app->window);
	}
	virtual ~Renderer_Impl1() {

		delete mTestWrapper;
	}
	virtual void processFrame(AHardwareBuffer *buffer){

		mTestWrapper->renderFrame(buffer);

		{
			ANativeWindow * inputWindow = mSessionImpl->mSessionInfo.backlight_data;
			fill_ANativeWindow_with_color(inputWindow, 0x000000ff);
		}
	}
};


/*static*/ Renderer_ *Renderer_::build(FootSessionGlesImpl1 *sessionImpl, struct android_app_ *app) {
	return new Renderer_Impl1(sessionImpl, app);
	//return new Renderer_Impl2(sessionImpl, app);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AndroidApp_{
public:
	AndroidApp_() {
		mApp.activity = &mNativeActivity;
		mApp.window = nullptr;
	}
	void setWindow(ANativeWindow *window) { mApp.window = window; }
	struct android_app_ *getApp() { return &mApp; }
	ANativeActivity mNativeActivity;
	struct android_app_ mApp;
};

class RenderingHandler: public SyncTaskHandler {
public:
	
	RenderingHandler(FootSessionGlesImpl1 *impl): mSessionImpl(impl)
	{
		SyncTaskHandler::init(impl->mSessionInfo.final_image);
	}
	~RenderingHandler() { 
		SyncTaskHandler::deinit();
	}

	void processFrame(AHardwareBuffer *buffer) {
		process(buffer);
	}

	virtual int onStart(void *ctx_) override {
		ANativeWindow *window = static_cast<ANativeWindow *>(ctx_);
		DLOGD( "RenderingHandler::%s ...\r\n", __func__);
		
		mAndroid_app.setWindow(window);
		mRenderer_ = Renderer_::build(mSessionImpl, mAndroid_app.getApp());

		DLOGD( "RenderingHandler::%s done \r\n", __func__);
		return 0;
	}
	virtual int onParameter(int type_, void *parameter_) override { return 0; }
	virtual int onProcess(void *something) override {
		AHardwareBuffer *buffer = static_cast<AHardwareBuffer *>(something);
		DLOGD( "RenderingHandler::%s ...\r\n", __func__);
		mRenderer_->processFrame(buffer);
		mSessionImpl->mSessionInfo._on_frame_call();
		DLOGD( "RenderingHandler::%s done \r\n", __func__);
		return 0;
	}
	virtual int onStop() override {
		DLOGD( "RenderingHandler::%s ...\r\n", __func__);
		delete mRenderer_;
		mRenderer_ = nullptr;
		DLOGD( "RenderingHandler::%s done \r\n", __func__);
		return 0;
	}

	FootSessionGlesImpl1 *mSessionImpl = nullptr;
	AndroidApp_ mAndroid_app;
	Renderer_ *mRenderer_ = nullptr;

};

FootSessionGlesImpl1::FootSessionGlesImpl1(SessionInfo & session) :
	ImageReaderImageListenerWrapper(),
	mSessionInfo(session) {
	DLOGD( "FootSessionGlesImpl1::%s,%d  tid:%lu \r\n", __func__, __LINE__, pthread_self());

	if (mSessionInfo.final_image == nullptr
		|| mSessionInfo.backlight_data == nullptr) {
		DLOGD( "%s,%d error\r\n", __func__, __LINE__);
		return ;
	}
	int32_t width_ = 0;
	int32_t height_ = 0;
	int32_t format_ = 0;
	int32_t maxImages_ = 0;

	width_ = ANativeWindow_getWidth(mSessionInfo.final_image);
	height_ = ANativeWindow_getHeight(mSessionInfo.final_image);
	format_ = ANativeWindow_getFormat(mSessionInfo.final_image);
	if (mSessionInfo.width != width_) {
		mSessionInfo.width = width_;
	}
	if (mSessionInfo.height != height_) {
		mSessionInfo.height = height_;
	}
	if (mSessionInfo.format != format_) {
		mSessionInfo.format = format_;
		mSessionInfo.format = AIMAGE_FORMAT_RGBA_8888;
	}

	width_ = ANativeWindow_getWidth(mSessionInfo.backlight_data);
	height_ = ANativeWindow_getHeight(mSessionInfo.backlight_data);
	format_ = ANativeWindow_getFormat(mSessionInfo.backlight_data);
	if (mSessionInfo.bl_width != width_) {
		mSessionInfo.bl_width = width_; 
	}
	if (mSessionInfo.bl_height != height_) {
		mSessionInfo.bl_height = height_; 
	}
	if (mSessionInfo.bl_format != format_) {
		mSessionInfo.bl_format = format_; 
	}

	// create rendering thread 
	{
		std::unique_lock<std::mutex> caller_lock(mRenderingHandler_lock);
		mRenderingHandler = new RenderingHandler(this);
	}

{
	uint64_t usage_ = AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER
						| AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
	if (mSessionInfo.inputOwner > 0) {
		if (mSessionInfo.inputOwner == SessionInfo::INPUT_OWNER_VIRTUAL_DISPLAY) {
		}
		else if (mSessionInfo.inputOwner == SessionInfo::INPUT_OWNER_CPU) {
			usage_ = AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN
				| AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
		}
	}

	mHardwareBufferReader = new football::HardwareBufferReader(this, 
		mSessionInfo.width, mSessionInfo.height, mSessionInfo.format, 3, usage_, football::HardwareBufferReader::PROC_HARDWARE_BUFFER);
	mSessionInfo.input_window = mHardwareBufferReader->getANativeWindow();
	mSessionInfo.input_window = mSessionInfo.input_window;
}
	DLOGD( "# session window size: %04d x %04d format:%08x / bl data size:%04d x %04d format:%08x \r\n", 
		mSessionInfo.width, mSessionInfo.height, mSessionInfo.format,
		mSessionInfo.bl_width, mSessionInfo.bl_height, mSessionInfo.bl_format);


}

FootSessionGlesImpl1::~FootSessionGlesImpl1() {
	DLOGD( "FootSessionGlesImpl1::%s,%d  tid:%lu \r\n", __func__, __LINE__, pthread_self());

	{
		if (mHardwareBufferReader != nullptr) {
			delete mHardwareBufferReader;
			mHardwareBufferReader = nullptr;
		}
	}

	// destroy rendering thread
	{
		std::unique_lock<std::mutex> caller_lock(mRenderingHandler_lock);
		if (mRenderingHandler != nullptr) {
			delete mRenderingHandler;
			mRenderingHandler = nullptr;
		}
	}

	if (mReader != nullptr) {
		// if onImageAvailableCallback is ongoing , should wait it to finished !!!
		// destroy reader
		AImageReader_setImageListener(mReader, nullptr);
		AImageReader_delete(mReader);
	}

	DLOGD( "FootSessionGlesImpl1::%s,%d  tid:%lu \r\n", __func__, __LINE__, pthread_self());
}
bool FootSessionGlesImpl1::isValid() {
	return mSessionInfo.input_window != nullptr ? true : false;
}
int FootSessionGlesImpl1::setSessionParameter(SessionParameter *parameter) {
	if (parameter->trigger_request) {
		parameter->trigger_request = 0;

	}

	mSessionParameter = *parameter;
	return 0;
}
int FootSessionGlesImpl1::getSessionParameter(SessionParameter *parameter) {
	*parameter = mSessionParameter;
	return 0;
}

void FootSessionGlesImpl1::print() {
	DLOGD( "%s \r\n", __func__);
}

// impl public football::HardwareBufferReader::CB
int FootSessionGlesImpl1::on_process_frame(AHardwareBuffer *buffer) {
	std::unique_lock<std::mutex> caller_lock(mRenderingHandler_lock);
	if (mRenderingHandler != nullptr) {
		mRenderingHandler->processFrame(buffer);
	}
	return 0;
}

// impl football::ImageReaderImageListenerWrapper
void FootSessionGlesImpl1::onImageAvailableCallback(AImageReader *reader) {
	DLOGD( "FootSessionGlesImpl1::%s,%d  tid:%lu \r\n", __func__, __LINE__, pthread_self());
	if (reader != mReader) { DLOGD( "*** impossible to reach here:%s,%d \r\n", __func__, __LINE__); return ;}
	AImage *image_ = nullptr;
	media_status_t ret = AImageReader_acquireLatestImage(reader, &image_);
	if (ret == AMEDIA_OK && image_ != nullptr) {
		AImage_delete(image_);
	}
}



};
};
};

