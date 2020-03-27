#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <math.h>

#include <vector>

#include "FootballConfig.h"

#include "MemTrace.h"
#undef __CLASS__
#define __CLASS__ "MemTrace"

using namespace std;

namespace football {

struct mem_info{
	void *ptr_ = nullptr;
	long size_ = 0;
	const char *_func_ = nullptr;
	int _line_ = 0;
};
typedef mem_info mem_info;

/*static*/ long MemTrace::allocated_block_num_ = 0;
/*static*/ long MemTrace::allocated_size_ = 0;

static vector<mem_info> s_mem_infos;

/*static*/ void *MemTrace::malloc(const char *_func_, int _line_, long size_) {
	mem_info info;
	info._func_ = _func_;
	info._line_ = _line_;
	info.size_ = size_;
	info.ptr_ = ::malloc(size_);
	assert(info.ptr_);
	s_mem_infos.push_back(info);
	allocated_block_num_++;
	allocated_size_ += size_;
	return info.ptr_;
}
/*static*/ void MemTrace::free(void *ptr_) {
	int found = 0;
	mem_info info;
	for(vector<mem_info>::iterator it=s_mem_infos.begin();it!=s_mem_infos.end();it++) {
		info = *it;
		if (ptr_ == info.ptr_) {
			found = 1;
			s_mem_infos.erase(it);
			break;
		}
	}
	assert(found);  // !!!
	if (found == 0) { DLOGD( "memory damaged! \r\n"); while(1); }

	if (found) {
		::free(info.ptr_);
	}
	allocated_size_ -= info.size_;
	allocated_block_num_--;
}
/*static*/ void MemTrace::print() {
	DLOGD( "allocated_block_num_: %ld allocated_size_:%ld /size:%d \r\n",
		allocated_block_num_, allocated_size_, (int)s_mem_infos.size());
	for(vector<mem_info>::iterator it=s_mem_infos.begin();it!=s_mem_infos.end();it++) {
		mem_info info = *it;
		DLOGD( "allocated %6ld from %s/%d \r\n", info.size_, info._func_, info._line_);
	}
}

};

