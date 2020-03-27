/*
* Copyright (c) 2014 - 2018, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of The Linux Foundation nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//#include <utils/constants.h>
//#include <cutils/properties.h>
//#include <display_properties.h>

#include <android/log.h>

#include "football_debugger.h"

#undef LOG_TAG
#define LOG_TAG "football"

#undef __CLASS__
#define __CLASS__ "FootballDebugHandler"

namespace football {

/*static*/FootballDebugHandler FootballDebugHandler::debug_handler_;

FootballDebugHandler::FootballDebugHandler() {
  DebugHandler::Set(FootballDebugHandler::Get());
}

#if 0  // 
void FootballDebugHandler::DebugAll(bool enable, int verbose_level) {
  if (enable) {
    debug_handler_.log_mask_ = 0x7FFFFFFF;
    if (verbose_level) {
      // Enable verbose scalar logs only when explicitly enabled
      debug_handler_.log_mask_[kTagScalar] = 0;
    }
    debug_handler_.verbose_level_ = verbose_level;
  } else {
    debug_handler_.log_mask_ = 0x1;   // kTagNone should always be printed.
    debug_handler_.verbose_level_ = 0;
  }

  DebugHandler::SetLogMask(debug_handler_.log_mask_);
}

void FootballDebugHandler::DebugResources(bool enable, int verbose_level) {
  if (enable) {
    debug_handler_.log_mask_[kTagResources] = 1;
    debug_handler_.verbose_level_ = verbose_level;
  } else {
    debug_handler_.log_mask_[kTagResources] = 0;
    debug_handler_.verbose_level_ = 0;
  }

  DebugHandler::SetLogMask(debug_handler_.log_mask_);
}

void FootballDebugHandler::DebugStrategy(bool enable, int verbose_level) {
  if (enable) {
    debug_handler_.log_mask_[kTagStrategy] = 1;
    debug_handler_.verbose_level_ = verbose_level;
  } else {
    debug_handler_.log_mask_[kTagStrategy] = 0;
    debug_handler_.verbose_level_ = 0;
  }

  DebugHandler::SetLogMask(debug_handler_.log_mask_);
}

void FootballDebugHandler::DebugCompManager(bool enable, int verbose_level) {
  if (enable) {
    debug_handler_.log_mask_[kTagCompManager] = 1;
    debug_handler_.verbose_level_ = verbose_level;
  } else {
    debug_handler_.log_mask_[kTagCompManager] = 0;
    debug_handler_.verbose_level_ = 0;
  }

  DebugHandler::SetLogMask(debug_handler_.log_mask_);
}

void FootballDebugHandler::DebugDriverConfig(bool enable, int verbose_level) {
  if (enable) {
    debug_handler_.log_mask_[kTagDriverConfig] = 1;
    debug_handler_.verbose_level_ = verbose_level;
  } else {
    debug_handler_.log_mask_[kTagDriverConfig] = 0;
    debug_handler_.verbose_level_ = 0;
  }

  DebugHandler::SetLogMask(debug_handler_.log_mask_);
}

void FootballDebugHandler::DebugRotator(bool enable, int verbose_level) {
  if (enable) {
    debug_handler_.log_mask_[kTagRotator] = 1;
    debug_handler_.verbose_level_ = verbose_level;
  } else {
    debug_handler_.log_mask_[kTagRotator] = 0;
    debug_handler_.verbose_level_ = 0;
  }

  DebugHandler::SetLogMask(debug_handler_.log_mask_);
}

void FootballDebugHandler::DebugScalar(bool enable, int verbose_level) {
  if (enable) {
    debug_handler_.log_mask_[kTagScalar] = 1;
    debug_handler_.verbose_level_ = verbose_level;
  } else {
    debug_handler_.log_mask_[kTagScalar] = 0;
    debug_handler_.verbose_level_ = 0;
  }

  DebugHandler::SetLogMask(debug_handler_.log_mask_);
}

void FootballDebugHandler::DebugQdcm(bool enable, int verbose_level) {
  if (enable) {
    debug_handler_.log_mask_[kTagQDCM] = 1;
    debug_handler_.verbose_level_ = verbose_level;
  } else {
    debug_handler_.log_mask_[kTagQDCM] = 0;
    debug_handler_.verbose_level_ = 0;
  }

  DebugHandler::SetLogMask(debug_handler_.log_mask_);
}

void FootballDebugHandler::DebugClient(bool enable, int verbose_level) {
  if (enable) {
    debug_handler_.log_mask_[kTagClient] = 1;
    debug_handler_.verbose_level_ = verbose_level;
  } else {
    debug_handler_.log_mask_[kTagClient] = 0;
    debug_handler_.verbose_level_ = 0;
  }

  DebugHandler::SetLogMask(debug_handler_.log_mask_);
}

void FootballDebugHandler::DebugDisplay(bool enable, int verbose_level) {
  if (enable) {
    debug_handler_.log_mask_[kTagDisplay] = 1;
    debug_handler_.verbose_level_ = verbose_level;
  } else {
    debug_handler_.log_mask_[kTagDisplay] = 0;
    debug_handler_.verbose_level_ = 0;
  }

  DebugHandler::SetLogMask(debug_handler_.log_mask_);
}

void FootballDebugHandler::DebugQos(bool enable, int verbose_level) {
  if (enable) {
    debug_handler_.log_mask_[kTagQOSClient] = 1;
    // TODO(user): add qos impl log mask when logging available
    debug_handler_.verbose_level_ = verbose_level;
  } else {
    debug_handler_.log_mask_[kTagQOSClient] = 0;
    // TODO(user): add qos impl log mask when logging available
    debug_handler_.verbose_level_ = 0;
  }

  DebugHandler::SetLogMask(debug_handler_.log_mask_);
}
int  FootballDebugHandler::GetIdleTimeoutMs() {
  int value = IDLE_TIMEOUT_DEFAULT_MS;
  debug_handler_.GetProperty(IDLE_TIME_PROP, &value);

  return value;
}

#endif

// be equal with android_LogPriority
/*static*/ const char *FootballDebugHandler::priority_strs_[] = {"U", "A", "V", "D","I","W","E", "F", "S"};

void FootballDebugHandler::console_printf(int prio, const char *tag, const char *fmt, va_list ap) {
	std::unique_lock<std::mutex> worker_lock(buf_mutex_);
	//memset(buf__, 0, LOG_BUF_SIZE__);
	vsnprintf(buf__, LOG_BUF_SIZE__ - 1, fmt, ap);
	fprintf(stderr, "[%s], -%s, %s \r\n", priority_strs_[prio], tag, buf__);
}

// android_LogPriority
void FootballDebugHandler::logcat_printf(int prio, const char *tag, const char *fmt, va_list ap) {
	__android_log_vprint(prio, tag, fmt, ap);
}
void FootballDebugHandler::remote_printf(int prio, const char *tag, const char *fmt, va_list ap) {
	std::unique_lock<std::mutex> worker_lock(remote_buf_mutex_);
	//memset(rebuf__, 0, LOG_BUF_SIZE__);
	vsnprintf(rebuf__, LOG_BUF_SIZE__ - 1, fmt, ap);
	//
	if (mRemoteLOG_cb != nullptr) {
		mRemoteLOG_cb->on_log(rebuf__);
	}
}
void FootballDebugHandler::__redirect__printf(int prio, const char *tag, const char *fmt, va_list ap) {
	uint32_t flags__ = OUT_LOGCAT;
	{std::unique_lock<std::mutex> worker_lock(output_flags_mutex_); flags__ = output_flags_; }

	if (flags__& OUT_CONSOLE) {
		console_printf(prio, tag, fmt, ap);
	}
	if (flags__ & OUT_LOGCAT) {
		logcat_printf(prio, tag, fmt, ap);
	}
	if (flags__ & OUT_REMOTE) {
		remote_printf(prio, tag, fmt, ap);
	}

}

void FootballDebugHandler::Error(const char *format, ...) {
  va_list list;
  va_start(list, format);
  //__android_log_vprint(ANDROID_LOG_ERROR, LOG_TAG, format, list);
  __redirect__printf(ANDROID_LOG_ERROR, LOG_TAG, format, list);
}

void FootballDebugHandler::Warning(const char *format, ...) {
  va_list list;
  va_start(list, format);
  __android_log_vprint(ANDROID_LOG_WARN, LOG_TAG, format, list);
  __redirect__printf(ANDROID_LOG_WARN, LOG_TAG, format, list);
}

void FootballDebugHandler::Info(const char *format, ...) {
  va_list list;
  va_start(list, format);
  __android_log_vprint(ANDROID_LOG_INFO, LOG_TAG, format, list);
  __redirect__printf(ANDROID_LOG_INFO, LOG_TAG, format, list);
}

void FootballDebugHandler::Debug(const char *format, ...) {
  va_list list;
  va_start(list, format);
  __android_log_vprint(ANDROID_LOG_DEBUG, LOG_TAG, format, list);
  __redirect__printf(ANDROID_LOG_DEBUG, LOG_TAG, format, list);
}

void FootballDebugHandler::Verbose(const char *format, ...) {
  if (debug_handler_.verbose_level_) {
    va_list list;
    va_start(list, format);
    __android_log_vprint(ANDROID_LOG_VERBOSE, LOG_TAG, format, list);
	__redirect__printf(ANDROID_LOG_VERBOSE, LOG_TAG, format, list);
  }
}

void FootballDebugHandler::BeginTrace(const char *class_name, const char *function_name,
                                 const char *custom_string) {
#if 0
  if (atrace_is_tag_enabled(ATRACE_TAG)) {
    char name[PATH_MAX] = {0};
    snprintf(name, sizeof(name), "%s::%s::%s", class_name, function_name, custom_string);
    atrace_begin(ATRACE_TAG, name);
  }
#endif
}

void FootballDebugHandler::EndTrace() {
#if 0
  atrace_end(ATRACE_TAG);
#endif
}
#if 0
int FootballDebugHandler::GetProperty(const char *property_name, int *value) {
  char property[PROPERTY_VALUE_MAX];

  if (property_get(property_name, property, NULL) > 0) {
    *value = atoi(property);
    return kErrorNone;
  }

  return kErrorNotSupported;
}

int FootballDebugHandler::GetProperty(const char *property_name, char *value) {
  if (property_get(property_name, value, NULL) > 0) {
    return kErrorNone;
  }

  return kErrorNotSupported;
}
#else

int FootballDebugHandler::GetProperty(const char *property_name, int *value) { return 0; }
int FootballDebugHandler::GetProperty(const char *property_name, char *value) { return 0; }

#endif

void FootballDebugHandler::setOutputFlags(int mask, int flags) {
	std::unique_lock<std::mutex> worker_lock(output_flags_mutex_);

	uint32_t old = output_flags_;
	old &= ~(mask);
	old |= flags;
	output_flags_ = old;
}


}  // namespace sdm

