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

#include "FootballConfig.h"

#include "utils/MemTrace.h"
#include "utils/StbImage_.h"


#include "pp/impl/FootballPPUtils.h"
#include "pp/impl/FootballPPVk.h"
#include "pp/FootballPPTester.h"

#include "standalone/AAssetManagerImpl_.h"

#include "VulkanMain_.hpp"       //

#include "computedemo1_1.h"

#include "FootSessionVkImpl.h"

#undef __CLASS__
#define __CLASS__ "FootSessionVkImpl"

using namespace football;

namespace computedemo1 {
namespace impl {

class Renderer_ {
public:
	static Renderer_ *build(FootSessionVkImpl *vkImpl, struct android_app_ *app);
	struct android_app_ *mApp = nullptr;
	Renderer_(struct android_app_ *app):mApp(app) {}
	virtual void set_parameter(SessionParameterTriggerData *data_) = 0;
	virtual void processFrame(AHardwareBuffer *buffer) = 0;
	virtual ~Renderer_() {
	}
};

class Renderer_Impl1: public Renderer_ {
/** test failed when many times called, result the framework to restart !!!
*/
public:
	FootSessionVkImpl *mFootSessionVkImpl = nullptr;
	Renderer_Impl1(FootSessionVkImpl *vkImpl, struct android_app_ *app):
		Renderer_(app), mFootSessionVkImpl(vkImpl) {
		::android_facade::AAssetManagerImpl_setAssetRootPath("tutorial06_texture/assets/");
		InitVulkan(mApp);
	}
	virtual void set_parameter(SessionParameterTriggerData *data_) {

	}
	virtual void processFrame(AHardwareBuffer *buffer){
		if (IsVulkanReady()) {
		#if 0
			VulkanDrawFrame();
		#else
			if (importAHardwareBufferAsTexture(buffer) == 0) {
				VulkanDrawFrame();

				//DeleteImportTexture(0x01);	// release hardwareBuffer imported !!!
				DeleteImportTexture();	

				mFootSessionVkImpl->mSessionInfo._on_frame_call();
			}
		#endif
		}

		{
			ANativeWindow * inputWindow = mFootSessionVkImpl->mSessionInfo.backlight_data;
			fill_ANativeWindow_with_color(inputWindow, 0x000000ff);
		}
	}
	virtual ~Renderer_Impl1() {
		DeleteVulkan();
	}
};
class Renderer_Impl2: public Renderer_ {
/**

*/
public:
	FootSessionVkImpl *mFootSessionVkImpl = nullptr;


	//#define VulkanExample_CLASS computedemo1_1::VulkanExample
	//#define ASSET_PATH "SaschaWillems/computeshader_foot/assets/"

	#define VulkanExample_CLASS computedemo1_1::VulkanExample
	#define VulkanExample_ASSET "SaschaWillems/computedemo1/assets/"
	
	VulkanExample_CLASS *mVulkanExample = nullptr;
	Renderer_Impl2(FootSessionVkImpl *vkImpl, struct android_app_ *app):
		Renderer_(app), mFootSessionVkImpl(vkImpl) {
		
		::android_facade::AAssetManagerImpl_setAssetRootPath(VulkanExample_ASSET);
		vks::android::getDeviceConfig();
		
		androidApp = app;

		mVulkanExample = new VulkanExample_CLASS();
		if(mFootSessionVkImpl->mType_ == 20) { // test histogram with graphic pipeline 
			mVulkanExample->setExtraInitFlag(VulkanExample_CLASS::INIT_HISTOGRAM_GRAPHIC);
		}
		else if(mFootSessionVkImpl->mType_ == 11) { // histogram with compute pipeline, AGC
			mVulkanExample->setExtraInitFlag(VulkanExample_CLASS::INIT_COMPUTE_AGC);
		}
		else if(mFootSessionVkImpl->mType_ == 12) { // histogram with compute pipeline, BHE
			mVulkanExample->setExtraInitFlag(VulkanExample_CLASS::INIT_COMPUTE_BHE);
		}
		else if(mFootSessionVkImpl->mType_ == 13) { // histogram with compute pipeline, AGC+BHE
			mVulkanExample->setExtraInitFlag(VulkanExample_CLASS::INIT_COMPUTE_AGC_BHE);
		}
		else if(mFootSessionVkImpl->mType_ == 14) {
			mVulkanExample->setExtraInitFlag(VulkanExample_CLASS::INIT_COMPUTE_MasiaEO);
		}
		else {
		}
		mVulkanExample->InitExtra();

		
		bool init_result = mVulkanExample->initVulkan();
		assert(init_result == true);
		mVulkanExample->prepare();
		assert(mVulkanExample->prepared);

		mVulkanExample->renderFrame__android_prepare();
	}
	virtual void set_parameter(SessionParameterTriggerData *data_) {
		if (data_->type == 1) {
			if (data_->param_int0) {
				mVulkanExample->setDebugWindow(1);
			}
			else {
				mVulkanExample->setDebugWindow(0);
			}
		}
		else if(data_->type == 2) {
			mVulkanExample->impl1_setBHE_factor(data_->factor0, data_->factor1);
		}
		else if(data_->type == 3) {
			mVulkanExample->setScreenMode(data_->param_int0);
		}
		else if(data_->type == 4) {
			mVulkanExample->impl1_setBHE_tuning(data_->factor0, data_->factor1);
		}

	}
	virtual void processFrame(AHardwareBuffer *buffer){

		if (mVulkanExample->importAHardwareBufferAsTexture(buffer) == 0) {
			mVulkanExample->renderFrame__android();
			mVulkanExample->deleteImportTexture();
		}
		mFootSessionVkImpl->mSessionInfo._on_frame_call();
	
		{
			ANativeWindow * inputWindow = mFootSessionVkImpl->mSessionInfo.backlight_data;
			fill_ANativeWindow_with_color(inputWindow, 0x000000ff);
		}
	}
	virtual ~Renderer_Impl2() {
		if (mVulkanExample->prepared) {
			mVulkanExample->swapChain.cleanup();
		}
		delete(mVulkanExample);
	}
};

/*static*/ Renderer_ *Renderer_::build(FootSessionVkImpl *vkImpl, struct android_app_ *app) {
	DLOGD( "Renderer_::%s, vkImpl->mType_ = %d \r\n", __func__, vkImpl->mType_);
	if (vkImpl->mType_ == 0) {
		return new Renderer_Impl1(vkImpl, app);
	}
	else {
		return new Renderer_Impl2(vkImpl, app);
	}
	return new Renderer_Impl1(vkImpl, app);
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
	
	RenderingHandler(FootSessionVkImpl *impl): mSessionImpl(impl)
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
	virtual int onParameter(int type_, void *parameter_) override {
		if (type_ == 0) {
			SessionParameterTriggerData *data_ = (SessionParameterTriggerData*) parameter_;
			mRenderer_->set_parameter(data_);
		}
		return 0;
	}
	virtual int onProcess(void *something) override {
		AHardwareBuffer *buffer = static_cast<AHardwareBuffer *>(something);
		DLOGD( "RenderingHandler::%s ...\r\n", __func__);
		mRenderer_->processFrame(buffer);
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

	FootSessionVkImpl *mSessionImpl = nullptr;
	AndroidApp_ mAndroid_app;
	Renderer_ *mRenderer_ = nullptr;

};


FootSessionVkImpl::FootSessionVkImpl(int type_, SessionInfo &session) :
	ImageReaderImageListenerWrapper(),
	mType_(type_),
	mSessionInfo(session) {
	DLOGD( "FootSessionVkImpl::%s,%d  tid:%lu \r\n", __func__, __LINE__, pthread_self());

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

#if 1
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
	mHardwareBufferReader = new football::HardwareBufferReader(this, mSessionInfo.width, mSessionInfo.height, mSessionInfo.format, 3,
					usage_, football::HardwareBufferReader::PROC_HARDWARE_BUFFER);

	mSessionInfo.input_window = mHardwareBufferReader->getANativeWindow();
	mSessionInfo.input_window = mSessionInfo.input_window;
}
#else
{
	uint64_t usage_ = 0;
	//usage_ |= AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;  // for cpu write
	usage_ |= AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER;  // for gpu framebuffer
	usage_ |= AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
	AImageReader *reader_ = nullptr;
	media_status_t ret = AImageReader_newWithUsage(
							mSessionInfo.width, mSessionInfo.height, mSessionInfo.format, usage_, 3, &reader_);
	if (ret == AMEDIA_OK && reader_ != nullptr) {
		mReader = reader_;
		AImageReader_setImageListener(mReader, this);
	
		AImageReader_getWidth(reader_, &width_);
		AImageReader_getHeight(reader_, &height_);
		AImageReader_getFormat(reader_, &format_);
		AImageReader_getMaxImages(reader_, &maxImages_);
		
		ANativeWindow *window_ = nullptr;
		ret = AImageReader_getWindow(reader_, &window_);
		if (ret == AMEDIA_OK && window_ != nullptr) {
			mSessionInfo.input_window = window_;
			session->input_window = mSessionInfo.input_window;
		}
	}
}
#endif
	DLOGD( "# session window size: %04d x %04d format:%08x / bl data size:%04d x %04d format:%08x \r\n", 
		mSessionInfo.width, mSessionInfo.height, mSessionInfo.format,
		mSessionInfo.bl_width, mSessionInfo.bl_height, mSessionInfo.bl_format);


}

FootSessionVkImpl::~FootSessionVkImpl() {
	DLOGD( "FootSessionVkImpl::%s,%d  tid:%lu \r\n", __func__, __LINE__, pthread_self());

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

	DLOGD( "FootSessionVkImpl::%s,%d  tid:%lu \r\n", __func__, __LINE__, pthread_self());
}
bool FootSessionVkImpl::isValid() {
	return mSessionInfo.input_window != nullptr ? true : false;
}
int FootSessionVkImpl::setSessionParameter(SessionParameter *parameter) {
	if (parameter->trigger_request) {
		parameter->trigger_request = 0;  // only trigger once !!!

		// for handler
		if (parameter->request_type == 1) {
			std::unique_lock<std::mutex> caller_lock(mRenderingHandler_lock);
			if (mRenderingHandler != nullptr) {
				mRenderingHandler->post_parameter(0, (void *)(&(parameter->trigger_data)));
			}
		}
	}
	mSessionParameter = *parameter;
	return 0;
}
int FootSessionVkImpl::getSessionParameter(SessionParameter *parameter) {
	*parameter = mSessionParameter;
	return 0;
}

void FootSessionVkImpl::print() {
	DLOGD( "%s \r\n", __func__);
}

// impl public football::HardwareBufferReader::CB
int FootSessionVkImpl::on_process_frame(AHardwareBuffer *buffer) {
	std::unique_lock<std::mutex> caller_lock(mRenderingHandler_lock);
	if (mRenderingHandler != nullptr) {
		mRenderingHandler->processFrame(buffer);
	}
	return 0;
}

// iml public football::ImageReaderImageListenerWrapper 
void FootSessionVkImpl::onImageAvailableCallback(AImageReader *reader) {
	DLOGD( "FootSessionVkImpl::%s,%d  tid:%lu \r\n", __func__, __LINE__, pthread_self());
	if (reader != mReader) { DLOGD( "*** impossible to reach here:%s,%d \r\n", __func__, __LINE__); return ;}

	AImage *image_ = nullptr;
	media_status_t ret = AImageReader_acquireLatestImage(reader, &image_);
	if (ret == AMEDIA_OK && image_ != nullptr) {
		AImage_delete(image_);
	}
}



};
};

