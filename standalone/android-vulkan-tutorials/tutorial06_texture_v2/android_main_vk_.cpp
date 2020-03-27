#include <stdio.h>
#include <stdlib.h>

#include <android/log.h>

#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/native_window.h>

#include <android/hardware_buffer.h>

#include <media/NdkImage.h>
#include <media/NdkImageReader.h>
#include <unistd.h>

#include "FootballConfig.h"

#include "native_app_glue.h"

#include "ImageReaderHolder.h"

#include "VulkanMain_.hpp"       //
#include "AAssetManagerImpl_.h"

#define kTAG_ "android_main_vk"

#undef __CLASS__
#define __CLASS__ "android_main_vk"

#define IMAGE_INCOMMING (APP_CMD_USER_START_ + 1)

class ImageReaderHolder_vk: public ImageReaderHolder {
public:
	ImageReaderHolder_vk(android_app_ *app, int width, int height, int32_t format_, uint64_t usage_) :
		ImageReaderHolder(width, height, format_, usage_), mApp(app) {
	}
	virtual ~ImageReaderHolder_vk() {

	}
	virtual void onPendingImageReady() override {
		android_app_write_cmd_(mApp, IMAGE_INCOMMING);
	}
	android_app_ *mApp = nullptr;
};
static void init_image_reader(android_app_* app) {
	int width_ = ANativeWindow_getWidth(app->window);
	int height_ = ANativeWindow_getHeight(app->window);
	ImageReaderHolder *holder = new ImageReaderHolder_vk(app, width_, height_, AIMAGE_FORMAT_RGBA_8888, 
		AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN
		| AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE);
	app->userData = holder;
}
static void delete_image_reader(android_app_* app) {
	ImageReaderHolder *holder = (ImageReaderHolder *)app->userData;
	if (holder != nullptr) {
		delete holder;
    }
}

// Process the next main command.
static void handle_cmd(android_app_* app, int32_t cmd) {

    __android_log_print(ANDROID_LOG_INFO, kTAG_,
                        "handle_cmd: %d/%s", cmd, get_app_cmd_desc(cmd));
	
	DLOGD( "%s %d/%s \r\n", __func__, cmd, get_app_cmd_desc(cmd));
    switch (cmd) {
        case APP_CMD_INIT_WINDOW_:
            // The window is being shown, get it ready.
            init_image_reader(app);
            InitVulkan(app);
            break;
        case APP_CMD_TERM_WINDOW_:
            // The window is being hidden or closed, clean it up.
            DeleteVulkan();
			delete_image_reader(app);
            break;
		case IMAGE_INCOMMING :
		{
			ImageReaderHolder *holder = (ImageReaderHolder *)app->userData;
			AImage * image_incomming = holder->lockPendingImage();

			AHardwareBuffer *hardwareBuffer = nullptr;
			media_status_t ret = AImage_getHardwareBuffer(image_incomming, &hardwareBuffer);

			if (hardwareBuffer != nullptr) {
				if (importAHardwareBufferAsTexture(hardwareBuffer) == 0) {
#if 1
					// render if vulkan is ready
					if (IsVulkanReady()) {
					  VulkanDrawFrame();
					}
#endif

					DeleteImportTexture();	// release hardwareBuffer imported !!!

				}
			}

			holder->unlockPendingImage_andDelete();

		}
			break;
        default:
            //__android_log_print(ANDROID_LOG_INFO, kTAG_,
            //                    "event not handled: %d/%s", cmd, get_app_cmd_desc(cmd));
            break;
    }
	DLOGD( "%s %d done! \r\n", __func__, cmd);
}

void android_main_vk(struct android_app_* app) {
	DLOGD( ">> %s << \r\n", __func__);

	::android_facade::AAssetManagerImpl_setAssetRootPath("tutorial06_texture/assets/");

    // Set the callback to process system events
    app->onAppCmd = handle_cmd;

    // Used to poll the events in the main loop
    int events;
    android_poll_source_* source;

    // Main loop
    do {
        if (ALooper_pollAll(
			//IsVulkanReady() ? 1 : 0
			1
			, nullptr
			,&events, (void**)&source) >= 0) {
            if (source != NULL) source->process(app, source);
        }

	#if 0
        // render if vulkan is ready
	    if (IsVulkanReady()) {
	      VulkanDrawFrame();
	    }
	#endif
    } while (app->destroyRequested == 0);

    DLOGD( "%s done.\r\n", __func__);
}

