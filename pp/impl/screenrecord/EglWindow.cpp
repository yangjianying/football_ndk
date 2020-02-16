/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#undef LOG_TAG  // frankie, 2019.08.14

#define LOG_TAG "ScreenRecord"
//#define LOG_NDEBUG 0
//#include <utils/Log.h>
#include <android/log.h>

#ifndef EGL_EGLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#endif

//#include <gui/BufferQueue.h>
//#include <gui/Surface.h>

#include "EglWindow.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <assert.h>

#undef LOG_TAG
#undef ALOGE
#undef ALOGV

#define LOG_TAG "GLES3JNI"
#include "android_logcat_.h"


#undef UNKNOWN_ERROR
#undef NO_ERROR
#define UNKNOWN_ERROR (-1)
#define NO_ERROR (0)

using namespace android;


status_t EglWindow::createWindow(/*const sp<IGraphicBufferProducer>& surface*/ ANativeWindow *window) {
    if (mEglSurface != EGL_NO_SURFACE) {
        ALOGE("surface already created");
        return UNKNOWN_ERROR;
    }
    status_t err = eglSetupContext(false);
    if (err != NO_ERROR) {
        return err;
    }

    // Cache the current dimensions.  We're not expecting these to change.
    mWidth = ANativeWindow_getWidth(window); // surface->query(NATIVE_WINDOW_WIDTH, &mWidth);
    mHeight = ANativeWindow_getHeight(window); // surface->query(NATIVE_WINDOW_HEIGHT, &mHeight);

    // Output side (EGL surface to draw on).
    //sp<ANativeWindow> anw = new Surface(surface);
    mEglSurface = eglCreateWindowSurface(mEglDisplay, mEglConfig, /*anw.get(),*/ window,
            NULL);
    if (mEglSurface == EGL_NO_SURFACE) {
        ALOGE("eglCreateWindowSurface error: %#x", eglGetError());
        eglRelease();
        return UNKNOWN_ERROR;
    }

    return NO_ERROR;
}

status_t EglWindow::createPbuffer(int width, int height) {
    if (mEglSurface != EGL_NO_SURFACE) {
        ALOGE("surface already created");
        return UNKNOWN_ERROR;
    }
    status_t err = eglSetupContext(true);
    if (err != NO_ERROR) {
        return err;
    }

    mWidth = width;
    mHeight = height;

    EGLint pbufferAttribs[] = {
            EGL_WIDTH, width,
            EGL_HEIGHT, height,
            EGL_NONE
    };
    mEglSurface = eglCreatePbufferSurface(mEglDisplay, mEglConfig, pbufferAttribs);
    if (mEglSurface == EGL_NO_SURFACE) {
        ALOGE("eglCreatePbufferSurface error: %#x", eglGetError());
        eglRelease();
        return UNKNOWN_ERROR;
    }

    return NO_ERROR;
}


status_t EglWindow::eglSetupContext(bool forPbuffer) {
    EGLBoolean result;

    mEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (mEglDisplay == EGL_NO_DISPLAY) {
        ALOGE("eglGetDisplay failed: %#x", eglGetError());
        return UNKNOWN_ERROR;
    }

    EGLint majorVersion, minorVersion;
    result = eglInitialize(mEglDisplay, &majorVersion, &minorVersion);
    if (result != EGL_TRUE) {
        ALOGE("eglInitialize failed: %#x", eglGetError());
        return UNKNOWN_ERROR;
    }
    ALOGV("EglWindow::eglSetupContext EGL v%d.%d", majorVersion, minorVersion);

    EGLint numConfigs = 0;
    EGLint windowConfigAttribs[] = {
            //EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
            //EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
            EGL_RECORDABLE_ANDROID, 1,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8, // window type can have no alpha channel
            // no alpha
            EGL_NONE
    };
    EGLint pbufferConfigAttribs[] = {
            //EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
            //EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_NONE
    };
    result = eglChooseConfig(mEglDisplay,
            forPbuffer ? pbufferConfigAttribs : windowConfigAttribs,
            &mEglConfig, 1, &numConfigs);
    if (result != EGL_TRUE) {
        ALOGE("eglChooseConfig error: %#x", eglGetError());
        return UNKNOWN_ERROR;
    }

    EGLint contextAttribs[] = {
        //EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };
    mEglContext = eglCreateContext(mEglDisplay, mEglConfig, EGL_NO_CONTEXT,
            contextAttribs);
    if (mEglContext == EGL_NO_CONTEXT) {
        ALOGE("eglCreateContext error: %#x", eglGetError());
        return UNKNOWN_ERROR;
    }

    return NO_ERROR;
}

void EglWindow::eglRelease() {
    ALOGV("EglWindow::eglRelease");
    if (mEglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE,
                EGL_NO_CONTEXT);

        if (mEglContext != EGL_NO_CONTEXT) {
            eglDestroyContext(mEglDisplay, mEglContext);
        }

        if (mEglSurface != EGL_NO_SURFACE) {
            eglDestroySurface(mEglDisplay, mEglSurface);
        }
    }

    mEglDisplay = EGL_NO_DISPLAY;
    mEglContext = EGL_NO_CONTEXT;
    mEglSurface = EGL_NO_SURFACE;
    mEglConfig = NULL;

    eglReleaseThread();
}

status_t EglWindow::makeCurrent() const {
    if (!eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext)) {
        ALOGE("eglMakeCurrent failed: %#x", eglGetError());
        return UNKNOWN_ERROR;
    }
    return NO_ERROR;
}

// Sets the presentation time on the current EGL buffer.
void EglWindow::presentationTime(nsecs_t whenNsec) const {
    eglPresentationTimeANDROID(mEglDisplay, mEglSurface, whenNsec);
}

// Swaps the EGL buffer.
void EglWindow::swapBuffers() const {
    eglSwapBuffers(mEglDisplay, mEglSurface);
}

/////////////////////////////////////////////////////////////////////

status_t EglWindow::createEglSurface_(/*const sp<IGraphicBufferProducer>& surface,*/ ANativeWindow *window, 
		int *outWidth, int *outHeight, EGLSurface *outSurface) {
    if (mEglDisplay == EGL_NO_DISPLAY || mEglConfig == EGL_NO_CONTEXT) {
        ALOGE("gl context not created yet !!!");
        return UNKNOWN_ERROR;
    }

	EGLSurface mEglSurface = EGL_NO_SURFACE;
	int width = 0;
	int height = 0;

    // Cache the current dimensions.  We're not expecting these to change.
    width = ANativeWindow_getWidth(window); // surface->query(NATIVE_WINDOW_WIDTH, &width);
    height = ANativeWindow_getHeight(window); // surface->query(NATIVE_WINDOW_HEIGHT, &height);
	*outWidth = width;
	*outHeight = height;

    // Output side (EGL surface to draw on).
    //sp<ANativeWindow> anw = new Surface(surface);
    mEglSurface = eglCreateWindowSurface(mEglDisplay, mEglConfig, /*anw.get()*/window,
            NULL);
    if (mEglSurface == EGL_NO_SURFACE) {
        ALOGE("eglCreateWindowSurface error: %#x", eglGetError());
        eglRelease();
        return UNKNOWN_ERROR;
    }
	*outSurface = mEglSurface;
    return NO_ERROR;
}
#if 0
status_t EglWindow::createEglSurface_(const sp<Surface>& surface,
	int *outWidth, int *outHeight, EGLSurface *outSurface) {
	if (mEglDisplay == EGL_NO_DISPLAY || mEglConfig == EGL_NO_CONTEXT) {
		ALOGE("gl context not created yet !!!");
		return UNKNOWN_ERROR;
	}

	EGLSurface mEglSurface = EGL_NO_SURFACE;
	int width = 0;
	int height = 0;

	// Cache the current dimensions.  We're not expecting these to change.
	surface->query(NATIVE_WINDOW_WIDTH, &width);
	surface->query(NATIVE_WINDOW_HEIGHT, &height);
	*outWidth = width;
	*outHeight = height;

	// Output side (EGL surface to draw on).
	mEglSurface = eglCreateWindowSurface(mEglDisplay, mEglConfig, surface.get(),
			NULL);
	if (mEglSurface == EGL_NO_SURFACE) {
		ALOGE("eglCreateWindowSurface error: %#x", eglGetError());
		eglRelease();
		return UNKNOWN_ERROR;
	}
	*outSurface = mEglSurface;
	return NO_ERROR;
}
#endif
status_t EglWindow::createPbuffer_(int width, int height, EGLSurface* outSurface) {
	if (mEglDisplay == EGL_NO_DISPLAY || mEglConfig == EGL_NO_CONTEXT) {
		ALOGE("gl context not created yet !!!");
		return UNKNOWN_ERROR;
	}

	EGLSurface mEglSurface = EGL_NO_SURFACE;
    mWidth = width;
    mHeight = height;

    EGLint pbufferAttribs[] = {
            EGL_WIDTH, width,
            EGL_HEIGHT, height,
            EGL_NONE
    };
    mEglSurface = eglCreatePbufferSurface(mEglDisplay, mEglConfig, pbufferAttribs);
    if (mEglSurface == EGL_NO_SURFACE) {
        ALOGE("eglCreatePbufferSurface error: %#x", eglGetError());
        eglRelease();
        return UNKNOWN_ERROR;
    }
	*outSurface = mEglSurface;
    return NO_ERROR;
}
void EglWindow::destroyEglSurface_(EGLSurface* eglSurface) {
	if (*eglSurface != EGL_NO_SURFACE) {
		eglDestroySurface(mEglDisplay, *eglSurface);
		*eglSurface = EGL_NO_SURFACE;
	}
}

status_t EglWindow::makeCurrent(EGLSurface mEglSurface) const {
    if (!eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext)) {
        ALOGE("eglMakeCurrent failed: %#x", eglGetError());
        return UNKNOWN_ERROR;
    }
    return NO_ERROR;
}

// Sets the presentation time on the current EGL buffer.
void EglWindow::presentationTime(EGLSurface mEglSurface, nsecs_t whenNsec) const {
    eglPresentationTimeANDROID(mEglDisplay, mEglSurface, whenNsec);
}

// Swaps the EGL buffer.
void EglWindow::swapBuffers(EGLSurface mEglSurface) const {
    eglSwapBuffers(mEglDisplay, mEglSurface);
}

