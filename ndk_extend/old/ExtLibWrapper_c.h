#ifndef JNI_EXTLIBWRAPPER_C_H_
#define JNI_EXTLIBWRAPPER_C_H_

#include <stddef.h>
#include <dlfcn.h>

typedef struct lib_entry_name_s {
    const char *symbol;
    void *func;
} lib_entry_name_t;

#define LIB_ENTRY(x) {x, nullptr}

#define DECLARE_ExtLibWrapper(class_name_)                                   \
    class class_name_ {                                               \
     public: \
        class_name_();                                                       \
        ~class_name_();                                                     \
        int load(const char *path, lib_entry_name_t *entries, int count);          \
        void unload();                                                              \
        void *getFunc(int index);                                                   \
                                                                                    \
        int mCount;                                                                 \
        lib_entry_name_t *mEntries;                                                 \
                                                                                    \
        void *mLibHandle;                                                           \
        static class_name_ * makeInstance();                                 \
        static int releaseInstance(class_name_ *instance_ptr);	\
    };

#define IMPL_ExtLibWrapper(class_name_)          \
    class_name_ * class_name_::makeInstance() {                     \
        return new class_name_();          \
    }                                                                   \
    int class_name_::releaseInstance(class_name_ *instance_ptr) { \
    	if (instance_ptr != nullptr) { \
    		instance_ptr->unload(); \
			delete instance_ptr; \
    	}\
		return 0;\
	} \
    class_name_::class_name_():                                   \
        mCount(-1), mEntries(NULL), mLibHandle(NULL) {           \
    }                                                                           \
    class_name_::~class_name_() {                                \
        unload();                                                               \
    }                                                                           \
    int class_name_::load(                                              \
        const char *path, lib_entry_name_t *entries, int count) {               \
        \
        int r = 0; \
        const char* error = NULL;                                               \
        int i = 0;                                                              \
        /*FrLOGD(__TAG, "%s start! \r\n", __func__);*/                          \
            mCount = count;                                                          \
            mEntries = entries;                                                      \
        mLibHandle = dlopen(path, RTLD_NOW);                                    \
        if (!mLibHandle) {                                                          \
            error = dlerror();                                                       \
            ALOGE("dlopen failed with error %s \r\n", error ? error : "");   \
            return -1; \
        } else {                                                                       \
            for (i=0; i < mCount; i++) {                                                  \
                void *__funcp = dlsym(mLibHandle, entries[i].symbol);                \
                if (!__funcp) {                                                      \
                    mEntries[i].func = NULL;                                         \
                    error = dlerror();                                               \
                    ALOGE("dlsym(%s) failed with error: %s \r\n",            \
                        entries[i].symbol, error ? error: ""); \
                    r = -2; \
                } else {                                                             \
                    mEntries[i].func = __funcp;                                      \
                }                                                                    \
            }                                                                        \
            ALOGV("load done! \r\n");                                        \
        }                                                           \
        /*FrLOGD(__TAG, "%s done! \r\n", __func__);*/                                \
        return r; \
    }                                                                                \
    void * class_name_::getFunc(int index) {                                  \
        if (index < mCount && mEntries[index].func != NULL) {                         \
            return mEntries[index].func;                                             \
        }                                                                            \
        ALOGE("symbol:%s invalid! \r\n",                                    \
            index < mCount ? mEntries[index].symbol : "none");   \
        return NULL;                                    \
    }                                                   \
    void class_name_::unload() {                 \
        if (mEntries != NULL) {                          \
            mEntries = NULL;                            \
        }                                               \
        mCount = 0;                                     \
        if (mLibHandle != NULL) {                        \
            dlclose(mLibHandle);                        \
            mLibHandle = NULL;                          \
        }                                               \
        ALOGV("unload done! \r\n");                                        \
    }

#define CALLLIB_FUNC_VOID(wrapper, index, func_type, ...)   \
	if (wrapper == NULL) { \
		fprintf(stderr, "### lib wrapper is not initialized !!! \r\n"); \
		abort(); \
	} \
    void *func_ = wrapper->getFunc(index);                   \
    if (func_ != NULL) {                                     \
        func_type func = (func_type)func_;                  \
        (*func)(__VA_ARGS__);                               \
    }
#define CALLLIB_FUNC_R(wrapper, index, func_type, ...)  \
	if (wrapper == NULL) { \
		fprintf(stderr, "### lib wrapper is not initialized !!! \r\n"); \
		abort(); \
	} \
    void *func_ = wrapper->getFunc(index);               \
    if (func_ != NULL) {                                 \
        func_type func = (func_type)func_;              \
        return (*func)(__VA_ARGS__);                    \
    }

#undef SIZEOF_
#define SIZEOF_(a) sizeof(a)/sizeof(a[0])

#endif  // JNI_EXTLIBWRAPPER_C_H_

