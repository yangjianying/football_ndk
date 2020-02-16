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

#if 1  // 
#include <EGL/egl.h>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>

#include <GLES2/gl2.h>

// Include the latest possible header file( GL version header )
#if __ANDROID_API__ >= 24
#include <GLES3/gl32.h>
#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2ext.h>
#include <GLES3/gl3ext.h>
#endif

#include "gpu_tonemapper/EGLImageBuffer_KHR.h"

#include "screenrecord/Program.h"
#include "screenrecord/TextRenderer.h"
#include "screenrecord/EglWindow.h"

#include "MemTrace.h"
#include "StbImage_.h"

#include "FootballPPTester.h"

#include "ndk_extend/NativeHooApi.h"

#include "gpu_tonemapper/utils/sync_task.h"


#define LOG_TAG "FootballPPGles"
#include "android_logcat_.h"

#include "FootballPPGles.h"

#undef UNKNOWN_ERROR
#undef NO_ERROR
#define UNKNOWN_ERROR (-1)
#define NO_ERROR (0)

using namespace sdm;
using namespace android;

namespace football {

FootballPPGles::FootballPPGles() {

}
FootballPPGles::~FootballPPGles() {

}
int FootballPPGles::buildSession(FootSession *session, int *session_id) {
	return 0;
}
int FootballPPGles::closeSession(int session_id) {
	return 0;
}
int FootballPPGles::setSessionParameter(int session_id, SessionParameter *parameter) {
	return 0;
}
int FootballPPGles::getSessionParameter(int session_id, SessionParameter *parameter) {
	return 0;
}
std::vector<int> FootballPPGles::getSessionIds() {
	std::vector<int> ids;

	return ids;
}
int FootballPPGles::getSession(int session_id, FootSession *session) {
	return 0;
}
void FootballPPGles::print(int session_id) {

}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

enum class ToneMapTaskCode : int32_t {
  kCodeGetInstance,
  kCodeBlit,
  kCodeDestroy,
};
struct ToneMapGetInstanceContext : public SyncTask<ToneMapTaskCode>::TaskContext {
	ANativeWindow *window = nullptr; //Layer *layer = nullptr;
};

struct ToneMapBlitContext : public SyncTask<ToneMapTaskCode>::TaskContext {
	AHardwareBuffer *hardware_buffer = nullptr; //Layer *layer = nullptr;
	//int merged_fd = -1;
	//int fence_fd = -1;
};

class GlesTest1 : public SyncTask<ToneMapTaskCode>::TaskHandler {
public:
	GlesTest1(ANativeWindow *window);
	virtual ~GlesTest1();

	int start(ANativeWindow *window);
	int stop();
	int postTestFrame();
	int postHardwareBuffer(AHardwareBuffer *buffer);

	int setup_l(ANativeWindow *window);
	int release_l();


    // Convert a monotonic time stamp into a string with the current time.
    void getTimeString_l(nsecs_t monotonicNsec, char* buf, size_t bufLen);
	void processFrame_l(AHardwareBuffer *buffer);

	// TaskHandler methods implementation.
	virtual void OnTask(const ToneMapTaskCode &task_code,
						SyncTask<ToneMapTaskCode>::TaskContext *task_context);


	SyncTask<ToneMapTaskCode> *tone_map_task_ = nullptr;

	ANativeWindow *mWindow = nullptr;
	// EGL display / context / surface.
    EglWindow mEglWindow;

    // GL rendering support.
    Program mExtTexProgram;
    Program mTexProgram;

    // Text rendering.
    TextRenderer mTextRenderer;

    // External texture, updated by GLConsumer.
    GLuint mExtTextureName = 0;


	bool mUseMonotonicTimestamps = false;
	// Start time, used to map monotonic to wall-clock time.
    nsecs_t mStartMonotonicNsecs = 0;
    nsecs_t mStartRealtimeNsecs = 0;

    // Used for tracking dropped frames.
    nsecs_t mLastFrameNumber = 0;
    size_t mTotalDroppedFrames = 0;

};
GlesTest1::GlesTest1(ANativeWindow *window)
	: tone_map_task_(new SyncTask<ToneMapTaskCode>(*this)) {
	fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	start(window);
}
GlesTest1::~GlesTest1() {
	stop();
	if (tone_map_task_ != nullptr) {
		delete tone_map_task_;
	}
	fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
}
int GlesTest1::start(ANativeWindow *window) {
	mWindow = window;

    // Grab the current monotonic time and the current wall-clock time so we
    // can map one to the other.  This allows the overlay counter to advance
    // by the exact delay between frames, but if the wall clock gets adjusted
    // we won't track it, which means we'll gradually go out of sync with the
    // times in logcat.
    mStartMonotonicNsecs = 0; // systemTime(CLOCK_MONOTONIC);
    mStartRealtimeNsecs = 0; // systemTime(CLOCK_REALTIME);

#if 1
	ToneMapGetInstanceContext ctx;
	ctx.window = window;
	tone_map_task_->PerformTask(ToneMapTaskCode::kCodeGetInstance, &ctx);
#endif
	return 0;
}
int GlesTest1::stop() {
#if 1
	tone_map_task_->PerformTask(ToneMapTaskCode::kCodeDestroy, nullptr);
#endif
	return 0;
}
int GlesTest1::postTestFrame() {
	fprintf(stderr, "+ %s,%d \r\n", __func__, __LINE__);
#if 1
	ToneMapBlitContext ctx = {};
	ctx.hardware_buffer = nullptr;
	tone_map_task_->PerformTask(ToneMapTaskCode::kCodeBlit, &ctx);
#endif
	fprintf(stderr, "- %s,%d done \r\n", __func__, __LINE__);
	return 0;
}
int GlesTest1::postHardwareBuffer(AHardwareBuffer *buffer) {
	fprintf(stderr, "+ %s,%d \r\n", __func__, __LINE__);
#if 1
	ToneMapBlitContext ctx = {};
	ctx.hardware_buffer = buffer;
	tone_map_task_->PerformTask(ToneMapTaskCode::kCodeBlit, &ctx);
#endif
	fprintf(stderr, "- %s,%d done \r\n", __func__, __LINE__);
	return 0;
}

int GlesTest1::setup_l(ANativeWindow *window) {
	fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);

    status_t err;

    err = mEglWindow.createWindow(window);
    if (err != NO_ERROR) {
        return err;
    }
    mEglWindow.makeCurrent();

    int width = mEglWindow.getWidth();
    int height = mEglWindow.getHeight();

    glViewport(0, 0, width, height);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Shaders for rendering from different types of textures.
    err = mTexProgram.setup(Program::PROGRAM_TEXTURE_2D);
    if (err != NO_ERROR) {
        return err;
    }
    err = mExtTexProgram.setup(Program::PROGRAM_EXTERNAL_TEXTURE);
    if (err != NO_ERROR) {
        return err;
    }

    err = mTextRenderer.loadIntoTexture();
    if (err != NO_ERROR) {
        return err;
    }
    mTextRenderer.setScreenSize(width, height);

    // Input side (buffers from virtual display).
    glGenTextures(1, &mExtTextureName);
    if (mExtTextureName == 0) {
        ALOGE("glGenTextures failed: %#x", glGetError());
        return UNKNOWN_ERROR;
    }

	fprintf(stderr, "%s,%d done \r\n", __func__, __LINE__);
	return 0;
}
int GlesTest1::release_l() {
	fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);

	if (mExtTextureName != 0) {
		fprintf(stderr, "mExtTextureName: %d \r\n", mExtTextureName);
	  glDeleteTextures(1, &mExtTextureName);
	  mExtTextureName = 0;
	}

    mTexProgram.release();
    mExtTexProgram.release();
    mEglWindow.release();

	fprintf(stderr, "%s,%d done \r\n", __func__, __LINE__);
	return 0;
}

void GlesTest1::getTimeString_l(nsecs_t monotonicNsec, char* buf, size_t bufLen) {
    //const char* format = "%m-%d %T";    // matches log output
    const char* format = "%T";
    struct tm tm;

    if (mUseMonotonicTimestamps) {
        snprintf(buf, bufLen, "%" PRId64, monotonicNsec);
        return;
    }

    // localtime/strftime is not the fastest way to do this, but a trivial
    // benchmark suggests that the cost is negligible.
    int64_t realTime = mStartRealtimeNsecs +
            (monotonicNsec - mStartMonotonicNsecs);
    time_t secs = (time_t) (realTime / 1000000000);
    localtime_r(&secs, &tm);
    strftime(buf, bufLen, format, &tm);

    int32_t msec = (int32_t) ((realTime % 1000000000) / 1000000);
    char tmpBuf[5];
    snprintf(tmpBuf, sizeof(tmpBuf), ".%03d", msec);
    strlcat(buf, tmpBuf, bufLen);
}

static nsecs_t __s_frameNumber = 0;
void GlesTest1::processFrame_l(AHardwareBuffer *buffer) {	
	fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	// update external texture !!!

	EGLImageBuffer_KHR_ *eg_ImageBuffer = nullptr;
	GLuint ext_oes_texture_id = 0;

	float texMatrix[16] = { 1.0f, 0.0f, 0.0f, 0.0f, 
		0.0f, 1.0f, 0.0f, 0.0f, 
		0.0f, 0.0f, 1.0f, 0.0f, 
		0.0f, 0.0f, 0.0f, 1.0f, 
		};
	nsecs_t monotonicNsec = 0;
	nsecs_t frameNumber = 0;

	frameNumber = ++__s_frameNumber;
	if (buffer != nullptr) {
		eg_ImageBuffer = new EGLImageBuffer_KHR_(buffer, mExtTextureName);
		ext_oes_texture_id = eg_ImageBuffer->getTexture(GL_TEXTURE_EXTERNAL_OES);
		fprintf(stderr, "hardware_buffer oes texture id: %d \r\n", ext_oes_texture_id);
	} else {
		fprintf(stderr, "hardware_buffer == nullptr \r\n");
	}

    if (mLastFrameNumber > 0) {
        mTotalDroppedFrames += size_t(frameNumber - mLastFrameNumber) - 1;
    }
    mLastFrameNumber = frameNumber;

	// 
	mTextRenderer.setProportionalScale(35);
    if (true) {  // DEBUG - full blue background
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    }
    int width = mEglWindow.getWidth();
    int height = mEglWindow.getHeight();

#if 1  // draw external texture
	if (ext_oes_texture_id > 0) {
		fprintf(stderr, "%s draw ext oes texture! \r\n", __func__);
	    if (false) {  // DEBUG - draw inset
	        mExtTexProgram.blit(ext_oes_texture_id, texMatrix,
	                100, 100, width-200, height-200);
	    } else {
	        mExtTexProgram.blit(ext_oes_texture_id, texMatrix,
	                0, 0, width, height);
	    }
	}
	else {
		fprintf(stderr, "%s no ext oes texture! \r\n", __func__);
	}	
#endif

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    if (false) {  // DEBUG - show entire font bitmap
        mTexProgram.blit(mTextRenderer.getTextureName(), Program::kIdentity,
                100, 100, width-200, height-200);
    }

    char textBuf[64] = {0};
    getTimeString_l(monotonicNsec, textBuf, sizeof(textBuf));
    //String8 timeStr(String8::format("%s f=%" PRId64 " (%zd)",
    //        textBuf, frameNumber, mTotalDroppedFrames));
    char timeStr[256] = {0};
	snprintf(timeStr, 256, "%s f=%" PRId64 " (%zd)", textBuf, frameNumber, mTotalDroppedFrames);
	fprintf(stderr, "size:%4dx%4d timeStr:%s \r\n", width, height, timeStr);

    mTextRenderer.drawString(mTexProgram, Program::kIdentity, 200, 200, timeStr);
	glDisable(GL_BLEND);


    if (false) {  // DEBUG - add red rectangle in lower-left corner
        glEnable(GL_SCISSOR_TEST);
        glScissor(0, 0, 200, 200);
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_SCISSOR_TEST);
    }

    //mEglWindow.presentationTime(monotonicNsec);
    mEglWindow.swapBuffers();

	if (eg_ImageBuffer != nullptr) {
		delete eg_ImageBuffer;
	}
}

void GlesTest1::OnTask(const ToneMapTaskCode &task_code,
						SyncTask<ToneMapTaskCode>::TaskContext *task_context) {

	fprintf(stderr, "%s, task_code:%d \r\n", __func__, (int)task_code);

  switch (task_code) {
	case ToneMapTaskCode::kCodeGetInstance: {
		ToneMapGetInstanceContext *ctx = static_cast<ToneMapGetInstanceContext *>(task_context);
		// init gl
		setup_l(ctx->window);
	  }
	  break;

	case ToneMapTaskCode::kCodeBlit: {
		ToneMapBlitContext *ctx = static_cast<ToneMapBlitContext *>(task_context);
		// consume hardware_buffer
		processFrame_l(ctx->hardware_buffer);
	  }
	  break;

	case ToneMapTaskCode::kCodeDestroy: {
		// destroy gl
		release_l();
	  }
	  break;
	default:
	  break;
  }

}

class HardwareBufferReader: public TestReader {
public:
	GlesTest1 *mGlesTest1 = nullptr;

	HardwareBufferReader(int width, int height, int format, int maxImages = 3)
		:TestReader(width, height, format, maxImages,
				AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER
				//| AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN
				| AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE
				| AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER) {
		fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	}
	virtual ~HardwareBufferReader() {
		fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
	}
	void setGlesTest1(GlesTest1 *test1) { mGlesTest1 = test1; }

	virtual void onImageAvailableCallback(AImageReader *reader) {
		std::unique_lock<std::mutex> caller_lock(caller_mutex_);
		if (destroyed) {
			fprintf(stderr, "%s,%d destroyed skip !!! \r\n", __func__, __LINE__);
			return ;
		}

		cb_is_ongoing = 1; //notify_cb_ongoing(1);
		fprintf(stderr, "HardwareBufferReader::%s,%d frame: %ld tid:%lu \r\n", __func__, __LINE__, mFrameIndex, pthread_self());
		AImage *image_ = nullptr;
		media_status_t ret = AImageReader_acquireLatestImage(reader, &image_);
		if (ret == AMEDIA_OK && image_ != nullptr) {
			// here read using CPU will failed !!!
			if (mGlesTest1 != nullptr) {
				AHardwareBuffer *hardware_buffer_ = nullptr;
				ret = AImage_getHardwareBuffer(image_, &hardware_buffer_);
				if (ret == AMEDIA_OK && hardware_buffer_ != nullptr) {
					mGlesTest1->postHardwareBuffer(hardware_buffer_);
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

/*static*/ void FootballPPGles::test1(int numCycle, long wait_ms) {
{
	fprintf(stderr, "%s, start. numCycle:%d wait_ms:%ld seconds \r\n", __func__, numCycle, wait_ms/1000);

	for(int cyc_ = 0; cyc_ < numCycle;cyc_++) {
		int ret = 0;
		ANativeHooSurface *hooSurface = nullptr;
		ret = ANativeHooSurface_create("", 1080, 1920, 0x01, 0, &hooSurface);
		if (ret == 0 && hooSurface != nullptr) {
			fprintf(stderr, "create surface ok . \r\n");

			ANativeWindow *inputWindow = nullptr;
			ANativeHooSurface_getWindow(hooSurface, &inputWindow);
			if (inputWindow != nullptr) {
				fprintf(stderr, "get surface window ok . \r\n");
				
				GlesTest1 *gles_test1 = new GlesTest1(inputWindow);

				ANativeWindow *display_window = nullptr;
				HardwareBufferReader * mTestReader = nullptr;
				
				mTestReader = new HardwareBufferReader(1080, 1920, 0x01);
				mTestReader->setGlesTest1(gles_test1);			
				display_window = mTestReader->getANativeWindow();

				ANativeHooDisplay *display_ = nullptr;
				ret = ANativeHooDisplay_create("#test_display", 1080, 1920, display_window, &display_);
				fprintf(stderr, "ANativeHooDisplay_create ret=%d \r\n", ret);
				if (ret == 0 && display_ != nullptr) {
					usleep(wait_ms*1000);
					ANativeHooDisplay_destroy(display_);
				}

				if (mTestReader != nullptr) {
					delete mTestReader;
				}

			#if 0
				usleep(5*1000*1000);
			#else
				for(int i=0;i<10;i++) {
					gles_test1->postTestFrame();
				}
			#endif
				delete gles_test1;
			}

			usleep(100*1000);
			ANativeHooSurface_destroy(hooSurface);
		}
	}
	fprintf(stderr, "%s, done. \r\n", __func__);
}

}


};




