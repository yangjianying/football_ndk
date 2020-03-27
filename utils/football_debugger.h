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

#ifndef __FOOTBALL_DEBUGGER_H__
#define __FOOTBALL_DEBUGGER_H__

#define ATRACE_TAG (ATRACE_TAG_GRAPHICS | ATRACE_TAG_HAL)

//#include <log/log.h>
//#include <utils/Trace.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>

#include <bitset>

#include <thread>
#include <mutex>
#include <condition_variable>   // NOLINT


#include "debug_handler.h"



namespace football {

class FootballDebugHandler : public display::DebugHandler {
 public:
  FootballDebugHandler();
  static inline display::DebugHandler* Get() { return &debug_handler_; }
  static inline FootballDebugHandler* Get_FootballDebugHandler() { return &debug_handler_; }
  static const char* DumpDir() { return "/data/vendor/display"; }
#if 0
  static void DebugAll(bool enable, int verbose_level);
  static void DebugResources(bool enable, int verbose_level);
  static void DebugStrategy(bool enable, int verbose_level);
  static void DebugCompManager(bool enable, int verbose_level);
  static void DebugDriverConfig(bool enable, int verbose_level);
  static void DebugRotator(bool enable, int verbose_level);
  static void DebugScalar(bool enable, int verbose_level);
  static void DebugQdcm(bool enable, int verbose_level);
  static void DebugClient(bool enable, int verbose_level);
  static void DebugQos(bool enable, int verbose_level);
  static void DebugDisplay(bool enable, int verbose_level);
  static int  GetIdleTimeoutMs();
#endif

  virtual void Error(const char *format, ...);
  virtual void Warning(const char *format, ...);
  virtual void Info(const char *format, ...);
  virtual void Debug(const char *format, ...);
  virtual void Verbose(const char *format, ...);
  virtual void BeginTrace(const char *class_name, const char *function_name,
                          const char *custom_string);
  virtual void EndTrace();
  virtual int GetProperty(const char *property_name, int *value);
  virtual int GetProperty(const char *property_name, char *value);

	enum {
		OUT_CONSOLE = 0x01,
		OUT_LOGCAT = 0x02,
		OUT_REMOTE = 0x04,
	};
	virtual void setOutputFlags(int mask, int flags);

	static const char *priority_strs_[];
  
#define LOG_BUF_SIZE__ (1024u)
  char buf__[LOG_BUF_SIZE__];
  // be equal with android_LogPriority
  std::mutex buf_mutex_;

  void console_printf(int prio, const char *tag, const char *fmt, va_list ap);
  void logcat_printf(int prio, const char *tag, const char *fmt, va_list ap);

	class RemoteLOG_cb {
	public:
		RemoteLOG_cb() {}
		virtual ~RemoteLOG_cb() {}
		virtual void on_log(char *log_str) = 0;
	};
	void set_RemoteLOG_cb(RemoteLOG_cb *cb_) {
		std::unique_lock<std::mutex> worker_lock(remote_buf_mutex_);
		mRemoteLOG_cb = cb_;
	}

  char rebuf__[LOG_BUF_SIZE__] = {0};
  std::mutex remote_buf_mutex_;
  RemoteLOG_cb *mRemoteLOG_cb = nullptr;
  void remote_printf(int prio, const char *tag, const char *fmt, va_list ap) ;


  void __redirect__printf(int prio, const char *tag, const char *fmt, va_list ap);

 private:
  static FootballDebugHandler debug_handler_;

  std::bitset<32> log_mask_;
  int32_t verbose_level_;

  std::mutex output_flags_mutex_;
  uint32_t output_flags_ = OUT_CONSOLE;
};

}  // namespace sdm

#endif  // __HWC_DEBUGGER_H__

