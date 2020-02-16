#ifndef __AASSET_MANAGER_IMPL_H__
#define __AASSET_MANAGER_IMPL_H__

#include <string>
#include <android/asset_manager.h>

namespace android_facade {
	std::string AAssetManagerImpl_setAssetRootPath(std::string path_);
};

struct AAssetManager_;
typedef struct AAssetManager_ AAssetManager_;

struct AAsset_;
typedef struct AAsset_ AAsset_;

#define AAsset AAsset_

#define __IMPL(x) __impl_##x

//AAsset* __IMPL(AAssetManager_open)(AAssetManager* mgr, const char* filename, int mode);
AAsset* __IMPL(AAssetManager_open)(const char *caller_file, int caller_line, AAssetManager* mgr, const char* filename, int mode);
int __IMPL(AAsset_read)(AAsset* asset, void* buf, size_t count);
void __IMPL(AAsset_close)(AAsset* asset);
off_t __IMPL(AAsset_getLength)(AAsset* asset);

#ifndef AAssetManagerImpl_SOURCE

//#define AAssetManager_open __IMPL(AAssetManager_open)
#define AAssetManager_open(a,b,c) __IMPL(AAssetManager_open)(__FILE__, __LINE__, a, b, c)

#define AAsset_read __IMPL(AAsset_read)
#define AAsset_close __IMPL(AAsset_close)
#define AAsset_getLength __IMPL(AAsset_getLength)
#endif

#endif
