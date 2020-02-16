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

#include "MemTrace.h"
#include "StbImage_.h"

#include "FootballPPVk.h"
#include "FootSessionVkImpl.h"
#include "gpu_tonemapper/utils/sync_task.h"
#include "FootballPPTester.h"

#include "VulkanMain_.hpp"       //
#include "AAssetManagerImpl_.h"

#include "computeshaderdemo1.h"

using namespace football;
using namespace sdm;


namespace computedemo1 {
namespace impl {

class Renderer_ {
public:
	static Renderer_ *build(FootSessionVkImpl *vkImpl, struct android_app_ *app);
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
	FootSessionVkImpl *mFootSessionVkImpl = nullptr;
	Renderer_Impl1(FootSessionVkImpl *vkImpl, struct android_app_ *app):
		Renderer_(app), mFootSessionVkImpl(vkImpl) {
		::android_facade::AAssetManagerImpl_setAssetRootPath("/sdcard/data/tutorial06_texture/assets/");
		InitVulkan(mApp);
	}
	virtual ~Renderer_Impl1() {
		DeleteVulkan();
	}
	virtual void processFrame(AHardwareBuffer *buffer){
		if (IsVulkanReady()) {
			if (importAHardwareBufferAsTexture(buffer) == 0) {
				VulkanDrawFrame();
				DeleteImportTexture();	// release hardwareBuffer imported !!!
			}
		}

		{
			ANativeWindow * inputWindow = mFootSessionVkImpl->mFootSession.backlight_data;
			fill_ANativeWindow_with_color(inputWindow, 0x000000ff);
		}
	}
};
class Renderer_Impl2: public Renderer_ {
/**

*/
public:
	FootSessionVkImpl *mFootSessionVkImpl = nullptr;
	
	computeshaderdemo1::VulkanExample *mVulkanExample = nullptr;

	Renderer_Impl2(FootSessionVkImpl *vkImpl, struct android_app_ *app):
		Renderer_(app), mFootSessionVkImpl(vkImpl) {
		
		::android_facade::AAssetManagerImpl_setAssetRootPath("/sdcard/data/SaschaWillems/computeshader_foot/assets/");
		vks::android::getDeviceConfig();
		
		androidApp = app;

		mVulkanExample = new computeshaderdemo1::VulkanExample();

		bool init_result = mVulkanExample->initVulkan();
		assert(init_result == true);
		mVulkanExample->prepare();
		assert(mVulkanExample->prepared);

		mVulkanExample->renderFrame__android_prepare();
	}
	virtual ~Renderer_Impl2() {
		if (mVulkanExample->prepared) {
			mVulkanExample->swapChain.cleanup();
		}
		delete(mVulkanExample);
	}
	virtual void processFrame(AHardwareBuffer *buffer){

		mVulkanExample->renderFrame__android();

		{
			ANativeWindow * inputWindow = mFootSessionVkImpl->mFootSession.backlight_data;
			fill_ANativeWindow_with_color(inputWindow, 0x000000ff);
		}
	}
};

/*static*/ Renderer_ *Renderer_::build(FootSessionVkImpl *vkImpl, struct android_app_ *app) {
	//return new Renderer_Impl1(vkImpl, app);
	return new Renderer_Impl2(vkImpl, app);
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
enum class VkRenderTaskCode : int32_t {
  kCodeGetInstance,
  kCodeBlit,
  kCodeDestroy,
};
struct VkRenderTaskGetInstanceContext : public SyncTask<VkRenderTaskCode>::TaskContext {
	ANativeWindow *window = nullptr;
};
struct VkRenderTaskBlitContext : public SyncTask<VkRenderTaskCode>::TaskContext {
	AHardwareBuffer *hardware_buffer = nullptr;
};
class RenderingHandler : public SyncTask<VkRenderTaskCode>::TaskHandler {
public:
	RenderingHandler(FootSessionVkImpl *vkImpl):mFootSessionVkImpl(vkImpl),
		tone_map_task_(new SyncTask<VkRenderTaskCode>(*this)) {
		start(mFootSessionVkImpl->mFootSession.final_image);
	}
	~RenderingHandler() {
		fprintf(stderr, "RenderingHandler::%s ... \r\n", __func__);
		{
			std::unique_lock<std::mutex> caller_lock(caller_mutex_);
			mDestroied = true;
		}
		stop();
		if (tone_map_task_ != nullptr) {
			delete tone_map_task_;
			tone_map_task_ = nullptr;
		}
		fprintf(stderr, "RenderingHandler::%s done \r\n", __func__);
	}
	void processFrame(AHardwareBuffer *buffer) {
	 	std::unique_lock<std::mutex> caller_lock(caller_mutex_);
		if (mDestroied) {
			fprintf(stderr, "RenderingHandler::%s mDestroied !!! skip this frame .\r\n", __func__);
			return ;
		}
		fprintf(stderr, "RenderingHandler::%s ... \r\n", __func__);
		if (tone_map_task_ != nullptr) {
			VkRenderTaskBlitContext ctx = {};
			ctx.hardware_buffer = buffer;
			tone_map_task_->PerformTask(VkRenderTaskCode::kCodeBlit, &ctx);
		}
		fprintf(stderr, "RenderingHandler::%s done \r\n", __func__);
	}
	void start(ANativeWindow *window) {
		std::unique_lock<std::mutex> caller_lock(caller_mutex_);
		fprintf(stderr, "RenderingHandler::%s ...\r\n", __func__);
		if (tone_map_task_ != nullptr) {
			VkRenderTaskGetInstanceContext ctx;
			ctx.window = window;
			tone_map_task_->PerformTask(VkRenderTaskCode::kCodeGetInstance, &ctx);
		}
		fprintf(stderr, "RenderingHandler::%s done \r\n", __func__);
	}
	void stop() {
		std::unique_lock<std::mutex> caller_lock(caller_mutex_);
		fprintf(stderr, "RenderingHandler::%s ...\r\n", __func__);
		if (tone_map_task_ != nullptr) {
			tone_map_task_->PerformTask(VkRenderTaskCode::kCodeDestroy, nullptr);
		}
		fprintf(stderr, "RenderingHandler::%s done \r\n", __func__);
	}

	//
	void setup_l(ANativeWindow *window) {
		fprintf(stderr, "RenderingHandler::%s ...\r\n", __func__);
		
		mAndroid_app.setWindow(window);
		mRenderer_ = Renderer_::build(mFootSessionVkImpl, mAndroid_app.getApp());

		fprintf(stderr, "RenderingHandler::%s done \r\n", __func__);
	}
	void release_l() {
		fprintf(stderr, "RenderingHandler::%s ...\r\n", __func__);
		delete mRenderer_;
		mRenderer_ = nullptr;
		fprintf(stderr, "RenderingHandler::%s done \r\n", __func__);
	}
	void processFrame_l(AHardwareBuffer *buffer) {
		fprintf(stderr, "RenderingHandler::%s ...\r\n", __func__);
		mRenderer_->processFrame(buffer);
		fprintf(stderr, "RenderingHandler::%s done \r\n", __func__);
	}

	// TaskHandler methods implementation.
	virtual void OnTask(const VkRenderTaskCode &task_code,
						SyncTask<VkRenderTaskCode>::TaskContext *task_context) {
		fprintf(stderr, "RenderingHandler::%s, task_code:%d \r\n", __func__, (int)task_code);
	  switch (task_code) {
		case VkRenderTaskCode::kCodeGetInstance: {
			VkRenderTaskGetInstanceContext *ctx = static_cast<VkRenderTaskGetInstanceContext *>(task_context);
			// init gl
			if (mSetupFlag == false) {
				setup_l(ctx->window);
				mSetupFlag = true;
			}
		  }
		  break;

		case VkRenderTaskCode::kCodeBlit: {
			VkRenderTaskBlitContext *ctx = static_cast<VkRenderTaskBlitContext *>(task_context);
			// consume hardware_buffer
			if (mSetupFlag) {
				processFrame_l(ctx->hardware_buffer);
			}
		  }
		  break;

		case VkRenderTaskCode::kCodeDestroy: {
			// destroy gl
			if (mSetupFlag) {
				mSetupFlag = false;
				release_l();
			}
		  }
		  break;
		default:
		  break;
	  }

	}

	FootSessionVkImpl *mFootSessionVkImpl = nullptr;
	SyncTask<VkRenderTaskCode> *tone_map_task_ = nullptr;
	bool mSetupFlag = false;
	bool mDestroied = false;
	std::mutex caller_mutex_;

	AndroidApp_ mAndroid_app;

	Renderer_ *mRenderer_ = nullptr;

};

class HardwareBufferReader: public TestReader {
public:
	FootSessionVkImpl *mImpl = nullptr;
	HardwareBufferReader(FootSessionVkImpl *impl, int width, int height, int format, int maxImages = 3)
		:TestReader(width, height, format, maxImages,
				AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER
				//| AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN
				| AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE
				//| AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER
				), 
		mImpl(impl) 
	{
		fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	}
	virtual ~HardwareBufferReader() {
		fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	}
	virtual void onImageAvailableCallback(AImageReader *reader) {
		std::unique_lock<std::mutex> caller_lock(caller_mutex_);
		if (destroyed) {
			fprintf(stderr, "%s,%d destroyed skip 1 !!! \r\n", __func__, __LINE__);
			return ;
		}

		cb_is_ongoing = 1; //notify_cb_ongoing(1);
		caller_cv_.notify_one();

		fprintf(stderr, "HardwareBufferReader::%s,%d frame: %ld tid:%lu \r\n", __func__, __LINE__, mFrameIndex, pthread_self());

		if (destroyed) {
			fprintf(stderr, "%s,%d destroyed skip 2 !!! \r\n", __func__, __LINE__);
			return ;
		}

		AImage *image_ = nullptr;
		media_status_t ret = AImageReader_acquireLatestImage(reader, &image_);
		if (ret == AMEDIA_OK && image_ != nullptr) {
			// here read using CPU will failed !!!
			if (mImpl != nullptr && mImpl->mRenderingHandler != nullptr) {
				AHardwareBuffer *hardware_buffer_ = nullptr;
				ret = AImage_getHardwareBuffer(image_, &hardware_buffer_);
				if (ret == AMEDIA_OK && hardware_buffer_ != nullptr) {
					mImpl->processFrame(hardware_buffer_);
				}
				else {
					fprintf(stderr, "AImage_getHardwareBuffer failed ! \r\n");
				}
			}
			AImage_delete(image_);
		}
		incFrameNumber();
		cb_is_ongoing = 0; //notify_cb_ongoing(0);
		caller_cv_.notify_one();
	}

};


FootSessionVkImpl::FootSessionVkImpl(FootSession *session) : ImageReaderImageListenerWrapper(),
	mFootSession(*session) {
	fprintf(stderr, "FootSessionVkImpl::%s,%d  tid:%lu \r\n", __func__, __LINE__, pthread_self());

	if (session->final_image == nullptr
		|| session->backlight_data == nullptr) {
		fprintf(stderr, "%s,%d error\r\n", __func__, __LINE__);
		return ;
	}
	int32_t width_ = 0;
	int32_t height_ = 0;
	int32_t format_ = 0;
	int32_t maxImages_ = 0;

	width_ = ANativeWindow_getWidth(session->final_image);
	height_ = ANativeWindow_getHeight(session->final_image);
	format_ = ANativeWindow_getFormat(session->final_image);
	if (session->width != width_) {
		mFootSession.width = width_;
	}
	if (session->height != height_) {
		mFootSession.height = height_;
	}
	if (session->format != format_) {
		mFootSession.format = format_;
		mFootSession.format = AIMAGE_FORMAT_RGBA_8888;
	}

	width_ = ANativeWindow_getWidth(session->backlight_data);
	height_ = ANativeWindow_getHeight(session->backlight_data);
	format_ = ANativeWindow_getFormat(session->backlight_data);
	if (session->bl_width != width_) {
		mFootSession.bl_width = width_; 
	}
	if (session->bl_height != height_) {
		mFootSession.bl_height = height_; 
	}
	if (session->bl_format != format_) {
		mFootSession.bl_format = format_; 
	}

	// create rendering thread 
	{
		std::unique_lock<std::mutex> caller_lock(mRenderingHandler_lock);
		mRenderingHandler = new RenderingHandler(this);
	}

#if 1
{
	mHardwareBufferReader = new HardwareBufferReader(this, mFootSession.width, mFootSession.height, mFootSession.format);
	mFootSession.input_window = mHardwareBufferReader->getANativeWindow();
	session->input_window = mFootSession.input_window;
}
#else
{
	uint64_t usage_ = 0;
	//usage_ |= AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;  // for cpu write
	usage_ |= AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER;  // for gpu framebuffer
	usage_ |= AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
	AImageReader *reader_ = nullptr;
	//media_status_t ret = AImageReader_new(mFootSession.width, mFootSession.height, mFootSession.format, 3, &reader_);
	media_status_t ret = AImageReader_newWithUsage(
							mFootSession.width, mFootSession.height, mFootSession.format, usage_, 3, &reader_);
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
			mFootSession.input_window = window_;
			session->input_window = mFootSession.input_window;
		}
	}
}
#endif
	fprintf(stderr, "# session window size: %04d x %04d format:%08x / bl data size:%04d x %04d format:%08x \r\n", 
		mFootSession.width, mFootSession.height, mFootSession.format,
		mFootSession.bl_width, mFootSession.bl_height, mFootSession.bl_format);


}

FootSessionVkImpl::~FootSessionVkImpl() {
	fprintf(stderr, "FootSessionVkImpl::%s,%d  tid:%lu \r\n", __func__, __LINE__, pthread_self());

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

	fprintf(stderr, "FootSessionVkImpl::%s,%d  tid:%lu \r\n", __func__, __LINE__, pthread_self());
}
bool FootSessionVkImpl::isValid() {
	return mFootSession.input_window != nullptr ? true : false;
}
int FootSessionVkImpl::setSessionParameter(SessionParameter *parameter) {
	mSessionParameter = *parameter;
	return 0;
}
int FootSessionVkImpl::getSessionParameter(SessionParameter *parameter) {
	*parameter = mSessionParameter;
	return 0;
}

void FootSessionVkImpl::print() {
	fprintf(stderr, "%s \r\n", __func__);
}

void FootSessionVkImpl::processFrame(AHardwareBuffer *buffer) {
	std::unique_lock<std::mutex> caller_lock(mRenderingHandler_lock);
	if (mRenderingHandler != nullptr) {
		mRenderingHandler->processFrame(buffer);
	}
}

void FootSessionVkImpl::onImageAvailableCallback(AImageReader *reader) {
	fprintf(stderr, "FootSessionVkImpl::%s,%d  tid:%lu \r\n", __func__, __LINE__, pthread_self());
	if (reader != mReader) { fprintf(stderr, "*** impossible to reach here:%s,%d \r\n", __func__, __LINE__); return ;}

	AImage *image_ = nullptr;
	media_status_t ret = AImageReader_acquireLatestImage(reader, &image_);
	if (ret == AMEDIA_OK && image_ != nullptr) {
#if 1
		// post to rendering thread
		if (mRenderingHandler != nullptr) {
			media_status_t ret;
			AHardwareBuffer *hardware_buffer_ = nullptr;
			ret = AImage_getHardwareBuffer(image_, &hardware_buffer_);
			if (ret == AMEDIA_OK && hardware_buffer_ != nullptr) {
				{
					std::unique_lock<std::mutex> caller_lock(mRenderingHandler_lock);
					if (mRenderingHandler != nullptr) {
						mRenderingHandler->processFrame(hardware_buffer_);
					}
				}
			}
		}
#else
		fprintf(stderr, "    * drop! tid:%lu \r\n", pthread_self());
#endif
		AImage_delete(image_);
	}
}



};
};

