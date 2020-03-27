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

#ifndef SCREENRECORD_EGL_WINDOW_H
#define SCREENRECORD_EGL_WINDOW_H

//#include <gui/BufferQueue.h>
//#include <utils/Errors.h>

#include <android/native_window.h>  // ANativeWindow, ANativeWindow_Buffer

#include <EGL/egl.h>

namespace android {
#undef status_t
#define status_t int32_t
#undef nsecs_t
#define nsecs_t int64_t

/*
 * Wraps EGL display, context, surface, config for a window surface.
 *
 * Not thread safe.
 */
class EglWindow {
public:
    EglWindow() :
        mEglDisplay(EGL_NO_DISPLAY),
        mEglContext(EGL_NO_CONTEXT),
        mEglSurface(EGL_NO_SURFACE),
        mEglConfig(NULL),
        mWidth(0),
        mHeight(0)
        {}
    ~EglWindow() { eglRelease(); }

    // Creates an EGL window for the supplied surface.
    status_t createWindow(/*const sp<IGraphicBufferProducer>& surface*/ ANativeWindow *window);

    // Creates an EGL pbuffer surface.
    status_t createPbuffer(int width, int height);

    // Release anything we created.
    void release() { eglRelease(); }


	////////////////////////////////////////////////////////////////////////////
	// frankie, note, for default EGLSurface

    // Return width and height values (obtained from IGBP).
    int getWidth() const { return mWidth; }
    int getHeight() const { return mHeight; }

    // Make this context current.
    status_t makeCurrent() const;

    // Sets the presentation time on the current EGL buffer.
    void presentationTime(nsecs_t whenNsec) const;

    // Swaps the EGL buffer.
    void swapBuffers() const;


	////////////////////////////////////////////////////////////////////////////
	// frankie, add for extra EGLSurface create/makeCurrent/presentationTime/swapBuffers

	status_t createEglSurface_(/*const sp<IGraphicBufferProducer>& surface,*/ ANativeWindow *window, 
		int *outWidth, int *outHeight, EGLSurface *outSurface);
#if 0
	status_t createEglSurface_(const sp<Surface>& surface,
		int *outWidth, int *outHeight, EGLSurface *outSurface);
#endif
	status_t createPbuffer_(int width, int height, EGLSurface* outSurface);

	void destroyEglSurface_(EGLSurface* eglSurface);

	status_t makeCurrent(EGLSurface mEglSurface) const ;
	// Sets the presentation time on the current EGL buffer.
	void presentationTime(EGLSurface mEglSurface, nsecs_t whenNsec) const ;
	// Swaps the EGL buffer.
	void swapBuffers(EGLSurface mEglSurface) const ;

private:
    EglWindow(const EglWindow&);
    EglWindow& operator=(const EglWindow&);

    // Init display, create config and context.
    status_t eglSetupContext(bool forPbuffer);
    void eglRelease();

    // Basic EGL goodies.
    EGLDisplay mEglDisplay;
    EGLContext mEglContext;
    EGLSurface mEglSurface;
    EGLConfig mEglConfig;

    // Surface dimensions.
    int mWidth;
    int mHeight;
};

}; // namespace android

#endif /*SCREENRECORD_EGL_WINDOW_H*/
