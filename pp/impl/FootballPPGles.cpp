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

#include "FootballConfig.h"

#include "gpu_tonemapper/EGLImageBuffer_KHR.h"

#include "screenrecord/Program.h"
#include "screenrecord/TextRenderer.h"
#include "screenrecord/EglWindow.h"

#include "MemTrace.h"
#include "StbImage_.h"

#include "FootballPPTester.h"

#include "ndk_extend/NativeHooApi_Loader.h"

#define LOG_TAG "FootballPPGles"
#include "android_logcat_.h"


#include "FootballPPUtils.h"

#include "FootSessionGlesImpl1.h"

#include "FootballPPGles.h"

#undef __CLASS__
#define __CLASS__ "FootballPPGles"
#include "gpu_tonemapper/utils/sync_task.h"

#undef UNKNOWN_ERROR
#undef NO_ERROR
#define UNKNOWN_ERROR (-1)
#define NO_ERROR (0)

using namespace android;

namespace football {

#define SESSION_IMPL_CLS ::gles::session::impl::FootSessionGlesImpl1

FootballPPGles::FootballPPGles() {
	DLOGD( "%s,%d \r\n", __func__, __LINE__);
}
FootballPPGles::~FootballPPGles() {
	DLOGD( "%s,%d \r\n", __func__, __LINE__);
	// should close all sessions
}
int FootballPPGles::buildSession(int session_type, SessionInfo &session, int *session_id) {
	DLOGD( "FootballPPGles::%s,  session_type: %d \r\n", __func__, session_type);

	SESSION_IMPL_CLS * session_ = new SESSION_IMPL_CLS(session);
	if (!session_->isValid()) {
		delete session_;
		DLOGD( "%s,%d not valid\r\n", __func__, __LINE__);
		return -1;
	}
	session_->mId = addSession(session_);
	*session_id = session_->mId;
	DLOGD( "%s id: %d \r\n", __func__, session_->mId);
	return 0;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class GlesTestRenderer {
public:
	GlesTestRenderer(ANativeWindow *window);
	~GlesTestRenderer();
	void renderFrame(AHardwareBuffer *buffer);

	int setup_l(ANativeWindow *window);
	int release_l();
    // Convert a monotonic time stamp into a string with the current time.
    void getTimeString_l(nsecs_t monotonicNsec, char* buf, size_t bufLen);
	void processFrame_l(AHardwareBuffer *buffer);


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

////////////////////////////////////////////////////////////////////////////////////
GlesTestRendererWrapper1::GlesTestRendererWrapper1(ANativeWindow *window) {
	GlesTestRenderer *test = new GlesTestRenderer(window);
	mImpl = static_cast<void *>(test);
}
GlesTestRendererWrapper1::~GlesTestRendererWrapper1() {
	if (mImpl != nullptr) {
		GlesTestRenderer *test = static_cast<GlesTestRenderer *>(mImpl);
		delete test;
		mImpl = nullptr;
	}
}
void GlesTestRendererWrapper1::renderFrame(AHardwareBuffer *buffer) {
	if (mImpl != nullptr) {
		GlesTestRenderer *test = static_cast<GlesTestRenderer *>(mImpl);
		test->renderFrame(buffer);
	}
}
////////////////////////////////////////////////////////////////////////////////////

GlesTestRenderer::GlesTestRenderer(ANativeWindow *window) {
	mWindow = window;

    // Grab the current monotonic time and the current wall-clock time so we
    // can map one to the other.  This allows the overlay counter to advance
    // by the exact delay between frames, but if the wall clock gets adjusted
    // we won't track it, which means we'll gradually go out of sync with the
    // times in logcat.
    mStartMonotonicNsecs = 0; // systemTime(CLOCK_MONOTONIC);
    mStartRealtimeNsecs = 0; // systemTime(CLOCK_REALTIME);

	setup_l(mWindow);
}
GlesTestRenderer::~GlesTestRenderer() {
	release_l();
}

int GlesTestRenderer::setup_l(ANativeWindow *window) {
	DLOGD( "%s,%d \r\n", __func__, __LINE__);

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

	DLOGD( "%s,%d done \r\n", __func__, __LINE__);
	return 0;
}
int GlesTestRenderer::release_l() {
	DLOGD( "%s,%d \r\n", __func__, __LINE__);

	if (mExtTextureName != 0) {
		DLOGD( "mExtTextureName: %d \r\n", mExtTextureName);
	  glDeleteTextures(1, &mExtTextureName);
	  mExtTextureName = 0;
	}

    mTexProgram.release();
    mExtTexProgram.release();
    mEglWindow.release();

	DLOGD( "%s,%d done \r\n", __func__, __LINE__);
	return 0;
}

void GlesTestRenderer::getTimeString_l(nsecs_t monotonicNsec, char* buf, size_t bufLen) {
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
void GlesTestRenderer::processFrame_l(AHardwareBuffer *buffer) {	
	DLOGD( "%s,%d \r\n", __func__, __LINE__);
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
		DLOGD( "hardware_buffer oes texture id: %d \r\n", ext_oes_texture_id);
	} else {
		DLOGD( "hardware_buffer == nullptr \r\n");
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
		DLOGD( "%s draw ext oes texture! \r\n", __func__);
	    if (false) {  // DEBUG - draw inset
	        mExtTexProgram.blit(ext_oes_texture_id, texMatrix,
	                100, 100, width-200, height-200);
	    } else {
	        mExtTexProgram.blit(ext_oes_texture_id, texMatrix,
	                0, 0, width, height
	                , true
	                );
	    }
	}
	else {
		DLOGD( "%s no ext oes texture! \r\n", __func__);
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

    char timeStr[256] = {0};
	snprintf(timeStr, 256, "%s f=%" PRId64 " (%zd)", textBuf, frameNumber, mTotalDroppedFrames);
	DLOGD( "size:%4dx%4d timeStr:%s \r\n", width, height, timeStr);

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

void GlesTestRenderer::renderFrame(AHardwareBuffer *buffer) {
	processFrame_l(buffer);
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

class GlesTest1 : public SyncTaskHandler
	, public HardwareBufferReader::CB
	{
public:
	GlesTest1(ANativeWindow *window);
	virtual ~GlesTest1();


	int postTestFrame();
	
	int postHardwareBuffer(AHardwareBuffer *buffer);

	// impl public HardwareBufferReader::CB
	virtual int on_process_frame(AHardwareBuffer *hardware_buffer) override;

	virtual int onStart(void *ctx_) override ;
	virtual int onParameter(int type_, void *parameter_) override { return 0; }
	virtual int onProcess(void *something) override;
	virtual int onStop() override ;

	GlesTestRenderer *mGlesTestRenderer = nullptr;
};
GlesTest1::GlesTest1(ANativeWindow *window) {
	SyncTaskHandler::init(static_cast<void *>(window));
	DLOGD( "%s,%d \r\n", __func__, __LINE__);
}
GlesTest1::~GlesTest1() {
	SyncTaskHandler::deinit();
	DLOGD( "%s,%d \r\n", __func__, __LINE__);
}

int GlesTest1::postTestFrame() {
	DLOGD( "+ %s,%d \r\n", __func__, __LINE__);
	process(nullptr);
	DLOGD( "- %s,%d done \r\n", __func__, __LINE__);
	return 0;
}
int GlesTest1::postHardwareBuffer(AHardwareBuffer *buffer) {
	DLOGD( "+ %s,%d \r\n", __func__, __LINE__);
	process(static_cast<void *>(buffer));
	DLOGD( "- %s,%d done \r\n", __func__, __LINE__);
	return 0;
}
int GlesTest1::on_process_frame(AHardwareBuffer *hardware_buffer) {
	return postHardwareBuffer(hardware_buffer);
}

int GlesTest1::onStart(void *ctx_) {
	ANativeWindow *window = static_cast<ANativeWindow *>(ctx_);
	if (mGlesTestRenderer == nullptr) {
		mGlesTestRenderer = new GlesTestRenderer(window);
	}
	return 0;
}
int GlesTest1::onProcess(void *something) {
	AHardwareBuffer *hardware_buffer = static_cast<AHardwareBuffer *>(something);
	if (mGlesTestRenderer != nullptr) {
		mGlesTestRenderer->renderFrame(hardware_buffer);
	}
	return 0;
}
int GlesTest1::onStop() {
	if (mGlesTestRenderer != nullptr) {
		delete mGlesTestRenderer;
		mGlesTestRenderer = nullptr;
	}
	return 0;
}



/*static*/ void FootballPPGles::test1(int numCycle, long wait_ms) {
{
	DLOGD( "%s, start. numCycle:%d wait_ms:%ld seconds \r\n", __func__, numCycle, wait_ms/1000);

	for(int cyc_ = 0; cyc_ < numCycle;cyc_++) {
		int ret = 0;
		ANativeHooSurface *hooSurface = nullptr;
		ret = ANativeHooSurface_create("", 1080, 1920, AIMAGE_FORMAT_RGBA_8888, 0, &hooSurface);
		if (ret == 0 && hooSurface != nullptr) {
			DLOGD( "create surface ok . \r\n");

			ANativeWindow *inputWindow = nullptr;
			ANativeHooSurface_getWindow(hooSurface, &inputWindow);
			if (inputWindow != nullptr) {
				DLOGD( "get surface window ok . \r\n");
				
				GlesTest1 *gles_test1 = new GlesTest1(inputWindow);

				ANativeWindow *display_window = nullptr;
				HardwareBufferReader * mTestReader = nullptr;
				
				mTestReader = new HardwareBufferReader(gles_test1, 1080, 1920, AIMAGE_FORMAT_RGBA_8888);			
				display_window = mTestReader->getANativeWindow();

				ANativeHooDisplay *display_ = nullptr;
				ret = ANativeHooDisplay_create("#test_display", 1080, 1920, display_window, &display_);
				DLOGD( "ANativeHooDisplay_create ret=%d \r\n", ret);
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
	DLOGD( "%s, done. \r\n", __func__);
}

}


};




