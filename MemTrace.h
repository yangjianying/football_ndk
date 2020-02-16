#ifndef __MEM_TRACE_H__
#define __MEM_TRACE_H__

namespace football {
class MemTrace {
public:
	static long allocated_block_num_;
	static long allocated_size_;
	static void *malloc(const char *_func_, int _line_, long size_);
	static void free(void *ptr_);
	static void print();
};

};


#if 1
#define FREE_(p) ::football::MemTrace::free(p)
#define MALLOC_(x) ::football::MemTrace::malloc(__func__, __LINE__, x)
#else
#define FREE_(p) ::free(p)
#define MALLOC_(x) ::malloc(x)
#endif

#endif
