/*
 * Copyright (c) 2016, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 *
 * Copyright 2015 The Android Open Source Project
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

#ifndef __TONEMAPPER_GLENGINE_H__
#define __TONEMAPPER_GLENGINE_H__


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

#include <EGL/egl.h>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>


#if defined(CHECK_GL_ERRORS)
#define GL(func) func;
#define EGL(func) func;
#else
#define GL(func) \
  func;          \
  checkGlError(__FILE__, __LINE__);
#define EGL(func) \
  func;           \
  checkEglError(__FILE__, __LINE__);
#endif

#define EGL_PROTECTED_CONTENT_EXT 0x32C0

void checkGlError(const char *file, int line);
void checkEglError(const char *file, int line);

#endif  //__TONEMAPPER_GLENGINE_H__
