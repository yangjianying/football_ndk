/*
 * Copyright (c) 2016, 2019 The Linux Foundation. All rights reserved.
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

#ifndef __EGLIMAGE_BUFFER_H__
#define __EGLIMAGE_BUFFER_H__

#include <android/native_window.h>  // ANativeWindow, ANativeWindow_Buffer
#include <android/hardware_buffer.h>

//#include <cutils/native_handle.h>
//#include <gralloc_priv.h>
//#include <ui/GraphicBuffer.h>

#include "engine.h"

class EGLImageBuffer_KHR_ {
  // android::sp<android::GraphicBuffer> graphicBuffer;
  void *eglImageID;
  int width;
  int height;
  GLuint textureID;
  GLuint renderbufferID;
  GLuint framebufferID;

  	bool is_ext_texture_id = false;
	bool is_bindAsTexture = false;

 public:
  int getWidth();
  int getHeight();
  EGLImageBuffer_KHR_(const AHardwareBuffer *hardware_buffer, GLuint ext_texture_id = 0);
  unsigned int getTexture(int target);
  unsigned int getFramebuffer();
  void bindAsTexture(int target);
  void bindAsFramebuffer();
  ~EGLImageBuffer_KHR_();
  //static EGLImageBuffer *from(const private_handle_t *src);
  //static void clear();
};

#endif  //__EGLIMAGE_BUFFER_H_
