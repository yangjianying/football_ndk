/*
 * Copyright (c) 2016-2017, 2019 The Linux Foundation. All rights reserved.
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
#include "glengine.h"

#include "EGLImageBuffer_KHR.h"
//#include <cutils/native_handle.h>
//#include <gralloc_priv.h>
//#include <ui/GraphicBuffer.h>
#include <map>
//#include "EGLImageWrapper.h"

//-----------------------------------------------------------------------------
static EGLImageKHR create_eglImage(const AHardwareBuffer *hardware_buffer)
//-----------------------------------------------------------------------------
{
	AHardwareBuffer_Desc desc_ = {0};
	AHardwareBuffer_describe(hardware_buffer, &desc_);
  //bool isProtected = (graphicBuffer->getUsage() & GRALLOC_USAGE_PROTECTED);
  bool isProtected = false; // (desc_.usage & GRALLOC_USAGE_PROTECTED);
  EGLint attrs[] = {EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
                    isProtected ? EGL_PROTECTED_CONTENT_EXT : EGL_NONE,
                    isProtected ? EGL_TRUE : EGL_NONE, EGL_NONE};

  //EGLImageKHR eglImage = eglCreateImageKHR(
  //    eglGetCurrentDisplay(), (EGLContext)EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
  //    (EGLClientBuffer)(graphicBuffer->getNativeBuffer()), attrs);

	// typedef EGLClientBuffer (EGLAPIENTRYP PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC) (const struct AHardwareBuffer *buffer);
	// EGLAPI EGLClientBuffer EGLAPIENTRY eglGetNativeClientBufferANDROID (const struct AHardwareBuffer *buffer);
  EGLImageKHR eglImage = eglCreateImageKHR(
      eglGetCurrentDisplay(), (EGLContext)EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
      (EGLClientBuffer)(eglGetNativeClientBufferANDROID(hardware_buffer)), attrs);

  return eglImage;
}

//-----------------------------------------------------------------------------
EGLImageBuffer_KHR_::EGLImageBuffer_KHR_(const AHardwareBuffer *hardware_buffer, GLuint ext_texture_id)
//-----------------------------------------------------------------------------
{
	fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
  // this->graphicBuffer = graphicBuffer;
  this->eglImageID = create_eglImage(hardware_buffer);

 	AHardwareBuffer_Desc desc_ = {0};
	AHardwareBuffer_describe(hardware_buffer, &desc_);
  this->width = desc_.width; // graphicBuffer->getWidth();
  this->height = desc_.height; // graphicBuffer->getHeight();
  	fprintf(stderr, "  desc_ size:%4d x %4d \r\n", desc_.width, desc_.height);

	is_ext_texture_id = false;
	if (ext_texture_id > 0) {
		is_ext_texture_id = true;
	}
  textureID = ext_texture_id;
  renderbufferID = 0;
  framebufferID = 0;
}

//-----------------------------------------------------------------------------
EGLImageBuffer_KHR_::~EGLImageBuffer_KHR_()
//-----------------------------------------------------------------------------
{
if (is_ext_texture_id == false) {  // frankie, add
  if (textureID != 0) {
    GL(glDeleteTextures(1, &textureID));
    textureID = 0;
  }
} else {
	fprintf(stderr, "%s ext texture not release here !\r\n", __func__);
}

  if (renderbufferID != 0) {
    GL(glDeleteRenderbuffers(1, &renderbufferID));
    renderbufferID = 0;
  }

  if (framebufferID != 0) {
    GL(glDeleteFramebuffers(1, &framebufferID));
    framebufferID = 0;
  }

  // Delete the eglImage
  if (eglImageID != 0)
  {
      eglDestroyImageKHR(eglGetCurrentDisplay(), eglImageID);
      eglImageID = 0;
  }
  fprintf(stderr, "%s,%d \r\n", __func__, __LINE__);
}

//-----------------------------------------------------------------------------
int EGLImageBuffer_KHR_::getWidth()
//-----------------------------------------------------------------------------
{
  return width;
}

//-----------------------------------------------------------------------------
int EGLImageBuffer_KHR_::getHeight()
//-----------------------------------------------------------------------------
{
  return height;
}

//-----------------------------------------------------------------------------
unsigned int EGLImageBuffer_KHR_::getTexture(int target)
//-----------------------------------------------------------------------------
{
  if (is_bindAsTexture == false) {
  	is_bindAsTexture = true;
    bindAsTexture(target);
  }

  return textureID;
}

//-----------------------------------------------------------------------------
unsigned int EGLImageBuffer_KHR_::getFramebuffer()
//-----------------------------------------------------------------------------
{
  if (framebufferID == 0) {
    bindAsFramebuffer();
  }

  return framebufferID;
}

//-----------------------------------------------------------------------------
void EGLImageBuffer_KHR_::bindAsTexture(int target)
//-----------------------------------------------------------------------------
{
#if 1
	if (textureID == 0) {
	  GL(glGenTextures(1, &textureID));
	}
    GL(glBindTexture(target, textureID));
    GL(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL(glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL(glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    GL(glEGLImageTargetTexture2DOES(target, eglImageID));
#else
  if (textureID == 0) {
    GL(glGenTextures(1, &textureID));
    GL(glBindTexture(target, textureID));
    GL(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL(glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL(glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    GL(glEGLImageTargetTexture2DOES(target, eglImageID));
  }
#endif
  GL(glBindTexture(target, textureID));
}

//-----------------------------------------------------------------------------
void EGLImageBuffer_KHR_::bindAsFramebuffer()
//-----------------------------------------------------------------------------
{
  if (renderbufferID == 0) {
    GL(glGenFramebuffers(1, &framebufferID));
    GL(glGenRenderbuffers(1, &renderbufferID));

    GL(glBindRenderbuffer(GL_RENDERBUFFER, renderbufferID));
    GL(glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, eglImageID));

    GL(glBindFramebuffer(GL_FRAMEBUFFER, framebufferID));
    GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                 renderbufferID));
    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (result != GL_FRAMEBUFFER_COMPLETE) {
      //ALOGI("%s Framebuffer Invalid***************", __FUNCTION__);
      fprintf(stderr, "%s Framebuffer Invalid***************\r\n", __FUNCTION__);
    }
  }

  GL(glBindFramebuffer(GL_FRAMEBUFFER, framebufferID));
}
