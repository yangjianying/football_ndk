
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

#include "StbImageUtils.h"

#include "ImageReaderHolder.h"

#include "ndk_extend/NativeHooApi.h"

#define kTAG_ "native_app_main"


#define TEST_WIDTH (1080)
#define TEST_HEIGHT (1920)


/////////////////////////////////////////////////////////////////////////////////////////

static int __reader_cb_num = 0;


class MainWindowReader: public ImageReaderHolder {
public:
	MainWindowReader(int width, int height, int32_t format_, uint64_t usage_) :
		ImageReaderHolder(width, height, format_, usage_) {
	}
	virtual ~MainWindowReader() {

	}
	virtual void onPendingImageReady() override {
		AImage * image_ = lockPendingImage();

		{

			fprintf(stderr, ">>> %s ... \r\n", __func__);
			// saving as png
			int32_t w,h,f;
			AImage_getWidth(image_, &w);
			AImage_getHeight(image_, &h);
			AImage_getFormat(image_, &f);

			int32_t numPlanes = 0, pixelStride = 0, rowStride = 0;
			AImage_getNumberOfPlanes(image_, &numPlanes);
			AImage_getPlanePixelStride(image_, 0, &pixelStride);
			AImage_getPlaneRowStride(image_, 0, &rowStride);

			uint8_t* data = nullptr;
			/*out*/int dataLength = 0;
			AImage_getPlaneData(image_, 0, &data, &dataLength);

			fprintf(stderr, "%s size:%4dx%4d f:0x%08x num:%d pixelStride:%d rowStride:%d dataLength:%d \r\n",
				__func__, w, h, f, numPlanes, pixelStride, rowStride, dataLength);

#if 0
			{
				// /sdcard/data/tutorial06_texture/
				char path_[256] = {0};
				snprintf(path_, 255, "/sdcard/data/tutorial06_texture/cb_%04d.png", __reader_cb_num);
				int n = 4;
				vk___stbi_write_png(path_, w, h, n, data, rowStride);

				fprintf(stderr, "saving done! \r\n");
			}
#endif
			__reader_cb_num++;

		}
		unlockPendingImage_andDelete();
	}
};


#define RENDER_INTO_SURFACE_EN (1)

#define TEST_TIMES_WINDOW 1 // (10000)
#define TEST_TIMES_vulkan 1 // (100)

//int main(int argc, char *argv[]) 
int main(PFN_android_main_ pfunc_main_, int cycle_num, int wait_ms) 
{
    printf("Hello,world! \r\n");


    ANativeActivityCallbacks _callbacks = {NULL};
    ANativeActivity nativeActivity;
    ANativeActivity *pNativeActivity = &nativeActivity;

    for(int i = 0;i<
		//TEST_TIMES_WINDOW
		cycle_num
		;i++) {
		fprintf(stderr, "############################################ cycle : %d \r\n", i);

        ImageReaderHolder *aWindowHolder = nullptr;
		ANativeWindow *aNativeWindow = nullptr;


		ANativeHooSurface *hooSurface = nullptr;

	#if RENDER_INTO_SURFACE_EN
		{
			int ret = 0;
			ret = ANativeHooSurface_create("", TEST_WIDTH, TEST_HEIGHT, 0x01, 0, &hooSurface);
			if (ret == 0 && hooSurface != nullptr) {
				fprintf(stderr, "create surface ok . \r\n");
				ANativeWindow *inputWindow = nullptr;
				ANativeHooSurface_getWindow(hooSurface, &inputWindow);
				if (inputWindow != nullptr) {
					aNativeWindow = inputWindow;
				}
			}
		}
	#endif

		if (aNativeWindow == nullptr) {
			aWindowHolder = new MainWindowReader(TEST_WIDTH, TEST_HEIGHT, AIMAGE_FORMAT_RGBA_8888,
															   AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN
															   | AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER);
			aNativeWindow = aWindowHolder->getWindow();
		}

		for(int j=0;j<TEST_TIMES_vulkan;j++) {

	        pNativeActivity->callbacks = &_callbacks;
	        pNativeActivity->vm = NULL;
	        pNativeActivity->env = NULL;
	        pNativeActivity->clazz = NULL;
	        pNativeActivity->internalDataPath = "/sdcard/data/";
	        pNativeActivity->externalDataPath = "/sdcard/data/";
	        pNativeActivity->sdkVersion = 0;
	        pNativeActivity->instance = NULL;  // for saving struct android_app
	        pNativeActivity->assetManager = NULL;        // this must not be nullptr

	        pNativeActivity->obbPath = "/sdcard/data/";

	        fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	        ANativeActivity_onCreate_(pNativeActivity, NULL, 0, pfunc_main_);		// start app thread

	        fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	        pNativeActivity->callbacks->onInputQueueCreated(pNativeActivity, NULL);

	        fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	        pNativeActivity->callbacks->onNativeWindowCreated(pNativeActivity, aNativeWindow);  // init window

	        fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	        pNativeActivity->callbacks->onStart(pNativeActivity);
	        pNativeActivity->callbacks->onResume(pNativeActivity);

	        int focused = 1;
	        pNativeActivity->callbacks->onWindowFocusChanged(pNativeActivity, focused);

	        usleep(50 * 1000);

		#if 0
	        {
				// fill any color data into the texture !!!
				struct android_app_* app = (struct android_app_*)pNativeActivity->instance;
				ImageReaderHolder * holder = (ImageReaderHolder *)app->userData;
				// fill one frame, wait the rendering finished , then fill another frame , back and forth
				long after_wait_ms = 33;
				for(int i=0;i<frame_num;i++) {
					holder->test_fill_pattern();
					if (after_wait_ms > 0) {
						usleep(after_wait_ms*1000);  // let the process have time to be done !
					}
					//
				}
			}
		#else
		
		#endif

			fprintf(stderr, "*** wait %d milli-seconds ! \r\n", wait_ms);
			usleep(wait_ms * 1000);
		#if 1
			//usleep(5 * 1000 * 1000);
				// wait enough time for the last rendering finished, 
				// then the rendered image is saved already!
			usleep(50*1000);	
		#endif

	        focused = 0;
	        pNativeActivity->callbacks->onWindowFocusChanged(pNativeActivity, focused);

	        fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	        pNativeActivity->callbacks->onPause(pNativeActivity);
	        pNativeActivity->callbacks->onStop(pNativeActivity);

	        fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	        size_t outLen = 0;
	        pNativeActivity->callbacks->onSaveInstanceState(pNativeActivity, &outLen);
	        pNativeActivity->callbacks->onLowMemory(pNativeActivity);
	        pNativeActivity->callbacks->onConfigurationChanged(pNativeActivity);

	        fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	        pNativeActivity->callbacks->onNativeWindowDestroyed(pNativeActivity, NULL);  // term window

	        pNativeActivity->callbacks->onInputQueueDestroyed(pNativeActivity, NULL);

	        fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	        pNativeActivity->callbacks->onDestroy(pNativeActivity);

			fprintf(stderr, "*** %s, window test:%d , vulkan test:%d \r\n", __func__, i, j);

		}

		{
			if (aWindowHolder != nullptr) {
				delete aWindowHolder;
			}
		}
		if (hooSurface != nullptr) {
			ANativeHooSurface_destroy(hooSurface);
		}
        
    }

    fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
    return 0;
}
/*
2020-01-28 23:30:27.073 617-6245/? E/ion: ioctl:ION_IOC_MAP_IOMMU c0504908 failed with code -1: Invalid argument
2020-01-28 23:30:27.073 617-6245/? E/ion: map iommu failed!
2020-01-28 23:30:27.073 617-6245/? E/GRALLOC: BackendAlloc:266: ion_map_iommu( 8 ) failed
2020-01-28 23:30:27.073 617-6245/? E/GRALLOC: AllocateBuffer:199: alloc buffer failed ret=-1
2020-01-28 23:30:27.073 617-6245/? I/GRALLOC: Alloc Data:Width=640, Height=480, mReqWidth=640, mReqHeight=480, mUvHeight=0, Format=0x1, mInReqFormat=0x1, ProducerUsage=0x306, ComsumerUsage=0x306
    	 FormatType=0, InternalFormat=0x1, XRes=1080,YRes=2340, Size=1228800
    	 ByteStride=2560, PixelStride=640, IsAfbc=0, mIsHfbc=0, mIs10BitFormat=0, Bpp=4, AfbcHeaderSize=0, AfbcHeaderStride=0
    	 AfbcPayloadStride=0, AfbcScramble=0, mHfbcHeaderYsize=0, mHfbcPayloadYsize=0, mHfbcHeaderUvsize=0,mHfbcPayloadUvsize=0
    	 mHfbcScramble=0
2020-01-28 23:30:27.074 10412-10412/? I/CVLog: HwVisionHandlerManager: init
2020-01-28 23:30:27.074 22261-30290/? E/GraphicBufferAllocator: Failed to allocate (640 x 480) layerCount 1 format 1 usage 303: 5
2020-01-28 23:30:27.074 22261-30290/? D/GraphicBufferAllocator: Allocated buffers:
2020-01-28 23:30:27.074 22261-30290/? D/GraphicBufferAllocator: 0x7e07b38000: 1200.00 KiB |  640 ( 640) x  480 |    1 |        1 | 0x303 | ImageReader-640x480f1u515m3-22261-85
2020-01-28 23:30:27.074 22261-30290/? D/GraphicBufferAllocator: 0x7e07b38140: 1200.00 KiB |  640 ( 640) x  480 |    1 |        1 | 0x303 | ImageReader-640x480f1u515m3-22261-85

2020-01-28 23:30:27.075 22261-30290/? D/GraphicBufferAllocator: Total allocated (estimate): 120000.00 KB
2020-01-28 23:30:27.075 22261-30290/? E/BufferQueueProducer: [ImageReader-640x480f1u515m3-22261-475] dequeueBuffer: createGraphicBuffer failed
2020-01-28 23:30:27.075 22261-30290/? E/vulkan: dequeueBuffer[0] failed: Out of memory (-12)
2020-01-28 23:30:27.075 22261-30290/? E/Tutorial: Vulkan error. File[D:\vulkan\android-vulkan-tutorials\android-vulkan-tutorials\tutorial06_texture\app\src\main\cpp\standalone\VulkanMain_.cpp], line[278]
 * */
