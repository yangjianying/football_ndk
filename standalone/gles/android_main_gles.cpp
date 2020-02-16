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

#include "native_app_glue.h"

#include "ImageReaderHolder.h"

#include "GlesMain.h"
#include "AAssetManagerImpl_.h"

#define kTAG_ "android_main_gles"


#define TEST_WIDTH (1080)
#define TEST_HEIGHT (1920)


#define IMAGE_INCOMMING (APP_CMD_USER_START_ + 1)

class ImageReaderHolder_vk: public ImageReaderHolder {
public:
	ImageReaderHolder_vk(android_app_ *app, int width, int height, int32_t format_, uint64_t usage_) :
		ImageReaderHolder(width, height, format_, usage_), mApp(app) {
	}
	virtual ~ImageReaderHolder_vk() {

	}
	virtual void onPendingImageReady() override {
		fprintf(stderr, "%s ...\r\n", __func__);
		android_app_write_cmd_(mApp, IMAGE_INCOMMING);
	}
	android_app_ *mApp = nullptr;
};
static void init_image_reader(android_app_* app) {
	fprintf(stderr, "%s ...\r\n", __func__);
	ImageReaderHolder *holder = new ImageReaderHolder_vk(app, TEST_WIDTH, TEST_HEIGHT, AIMAGE_FORMAT_RGBA_8888, 
		AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN
		| AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE);
	app->userData = holder;
	fprintf(stderr, "%s done\r\n", __func__);
}
static void delete_image_reader(android_app_* app) {
	fprintf(stderr, "%s ...\r\n", __func__);
	ImageReaderHolder *holder = (ImageReaderHolder *)app->userData;
	if (holder != nullptr) {
		delete holder;
    }
	fprintf(stderr, "%s done\r\n", __func__);
}

// Process the next main command.
static void handle_cmd(android_app_* app, int32_t cmd) {

    __android_log_print(ANDROID_LOG_INFO, kTAG_,
                        "handle_cmd: %d/%s", cmd, get_app_cmd_desc(cmd));
	
	fprintf(stderr, "%s %d/%s \r\n", __func__, cmd, get_app_cmd_desc(cmd));
    switch (cmd) {
        case APP_CMD_INIT_WINDOW_:
            // The window is being shown, get it ready.
            init_image_reader(app);
            InitGles(app);
            break;
        case APP_CMD_TERM_WINDOW_:
            // The window is being hidden or closed, clean it up.
            DeleteGles();
			delete_image_reader(app);
            break;
		case IMAGE_INCOMMING :
		{
			ImageReaderHolder *holder = (ImageReaderHolder *)app->userData;
			AImage * image_incomming = holder->lockPendingImage();

			AHardwareBuffer *hardwareBuffer = nullptr;
			media_status_t ret = AImage_getHardwareBuffer(image_incomming, &hardwareBuffer);

			if (hardwareBuffer != nullptr) {
				if (importAHardwareBufferAsGlesTexture(hardwareBuffer) == 0) {
#if 1
					// render if vulkan is ready
					if (IsGlesReady()) {
					  GlesDrawFrame();
					}
#endif

					DeleteImportedGlesTexture();	// release hardwareBuffer imported !!!

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
	fprintf(stderr, "%s %d done! \r\n", __func__, cmd);
}

void android_main_gles(struct android_app_* app) {
	fprintf(stderr, ">> %s << \r\n", __func__);

	::android_facade::AAssetManagerImpl_setAssetRootPath("/sdcard/data/tutorial06_texture/assets/");

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

    fprintf(stderr, "%s done.\r\n", __func__);
}




