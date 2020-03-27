#ifndef __FOOT_THREAD_H__
#define __FOOT_THREAD_H__

#include <thread>
#include <mutex>
#include <condition_variable>   // NOLINT

namespace football {

class FootThread {
public:
	FootThread() {
	}
	void __start() {
		std::unique_lock<std::mutex> caller_lock(start_mutex_);
		std::thread worker_thread(__foot_thread__, this);
		worker_thread_.swap(worker_thread);
		start_cv_.wait(caller_lock);
	}
	virtual ~FootThread() {
		{
			set_worker_thread_exit_(true);
		}
		worker_thread_.join();
	}
	static void __foot_thread__(FootThread *thread_) {
	  if (thread_) {
		thread_->OnThreadCallback();
	  }
	}
	void OnThreadCallback() {
		// New scope to limit scope of caller lock to this block.
		{
			//DLOGD( "%s,%d started \r\n", __func__, __LINE__);
		  // Signal caller thread that worker thread is ready to listen to events.
		  std::unique_lock<std::mutex> caller_lock(start_mutex_);
		  start_cv_.notify_one();
		}

		while (!get_worker_thread_exit_()) {
			if (thread_loop__() == 0) {
				break;
			}
		}

	}
	void set_worker_thread_exit_(bool f) {
		std::unique_lock<std::mutex> caller_lock(exit_mutex_);
		worker_thread_exit_ = f;
		exit_cv_.notify_one();
	}
	bool get_worker_thread_exit_() {
		std::unique_lock<std::mutex> caller_lock(exit_mutex_);
		return worker_thread_exit_;
	}
	int thread_loop__() {
		return thread_loop();
	}
	
	virtual int thread_loop() = 0;
	
	std::thread worker_thread_;

	std::mutex start_mutex_;
	std::condition_variable start_cv_;

	std::mutex exit_mutex_;
	std::condition_variable exit_cv_;

	bool worker_thread_exit_ = false;

};

};


#endif

