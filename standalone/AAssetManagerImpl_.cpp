
#include <stdlib.h>
#include <stdio.h>

#include <string>

#include <android/log.h>

#include <android/asset_manager.h>
#include <assert.h>

#include "FootballConfig.h"
#undef __CLASS__
#define __CLASS__ "AAssetManagerImpl_"

typedef struct AAssetManager AAssetManager_android;
typedef struct AAsset AAsset_android;

static AAsset* AAssetManager_open__(AAssetManager* mgr, const char* filename, int mode) {
    return AAssetManager_open(mgr, filename, mode);
}
static int AAsset_read__(AAsset* asset, void* buf, size_t count) {
    return AAsset_read(asset, buf, count);
}
static void AAsset_close__(AAsset* asset) {
    return AAsset_close(asset);
}
static off_t AAsset_getLength__(AAsset* asset) {
    return AAsset_getLength(asset);
}

#define AAssetManagerImpl_SOURCE
#include "AAssetManagerImpl_.h"

#if 1  // frankie, add
// Android log function wrappers
static const char* kTAG = "AAssetManagerImpl";

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))
#endif

/*
struct AAssetManager;
typedef struct AAssetManager AAssetManager;
struct AAssetDir;
typedef struct AAssetDir AAssetDir;
struct AAsset;
typedef struct AAsset AAsset;

AAssetDir* AAssetManager_openDir(AAssetManager* mgr, const char* dirName);
AAsset* AAssetManager_open(AAssetManager* mgr, const char* filename, int mode);
const char* AAssetDir_getNextFileName(AAssetDir* assetDir);
void AAssetDir_rewind(AAssetDir* assetDir);
void AAssetDir_close(AAssetDir* assetDir);

int AAsset_read(AAsset* asset, void* buf, size_t count);
off_t AAsset_seek(AAsset* asset, off_t offset, int whence)
    __RENAME_IF_FILE_OFFSET64(AAsset_seek64);
off64_t AAsset_seek64(AAsset* asset, off64_t offset, int whence);
void AAsset_close(AAsset* asset);

const void* AAsset_getBuffer(AAsset* asset);

off_t AAsset_getLength(AAsset* asset)
    __RENAME_IF_FILE_OFFSET64(AAsset_getLength64);
off64_t AAsset_getLength64(AAsset* asset);
off_t AAsset_getRemainingLength(AAsset* asset)
    __RENAME_IF_FILE_OFFSET64(AAsset_getRemainingLength64);
off64_t AAsset_getRemainingLength64(AAsset* asset);

int AAsset_openFileDescriptor(AAsset* asset, off_t* outStart, off_t* outLength)
    __RENAME_IF_FILE_OFFSET64(AAsset_openFileDescriptor64);
int AAsset_openFileDescriptor64(AAsset* asset, off64_t* outStart, off64_t* outLength);
int AAsset_isAllocated(AAsset* asset);

 * */

static std::string android_facade_AssetBasePath = AAssetManagerImpl_DEFAULT_BASEPATH;

static std::string android_facade_AssetRootPath = AAssetManagerImpl_DEFAULT_PATH;
namespace android_facade {
	std::string AAssetManagerImpl_setAssetBasePath(std::string base_path_) {
		std::string r = android_facade_AssetBasePath;
		android_facade_AssetBasePath = base_path_;
		DLOGD( "AssetBasePath:%s \r\n", android_facade_AssetBasePath.c_str());
		return r;
	}
	std::string AAssetManagerImpl_setAssetRootPath(std::string path_, bool with_base_path) {
		std::string r = android_facade_AssetRootPath;
		if (with_base_path == false) {
			android_facade_AssetRootPath = path_;
		} else {
			android_facade_AssetRootPath = android_facade_AssetBasePath + path_;
		}
		DLOGD( "AssetRootPath:%s \r\n", android_facade_AssetRootPath.c_str());
		return r;
	}
};

struct AAssetManager_ {
    AAssetManager_android *aAssetManager_android = nullptr;
};
//#define AAssetManager AAssetManager_      // !!! should not redirect !!!

struct AAsset_ {
    AAsset_android *aAsset_android = nullptr;
    std::string filename;
    FILE *fp = nullptr;
};

#if 1  // from /sdcard/data/tutorial06_texture
//AAsset* __IMPL(AAssetManager_open)(AAssetManager* mgr, const char* filename, int mode)
AAsset* __IMPL(AAssetManager_open)(const char *caller_file, int caller_line, AAssetManager* mgr, const char* filename, int mode) 
{
	DLOGD( "%s from:%s/%d \r\n", __func__, caller_file, caller_line);
	LOGW("%s from:%s/%d \r\n", __func__, caller_file, caller_line);

	DLOGD( "%s, filename:%s ", __func__, filename);
    LOGW("%s, filename:%s ", __func__, filename);

    AAsset_ *aAsset_ = new AAsset_();
    aAsset_->filename = android_facade_AssetRootPath + filename;
    aAsset_->fp = fopen(aAsset_->filename.c_str(), "rb");
    //assert(aAsset_->fp != NULL);
    if (aAsset_->fp == nullptr) {
		delete aAsset_;
		DLOGD( "aAsset_->filename:%s not found!\r\n", aAsset_->filename.c_str());
		LOGW("aAsset_->filename:%s not found!\r\n", aAsset_->filename.c_str());
		return nullptr;
	}
	DLOGD( "aAsset_->filename:%s fopen ok!\r\n", aAsset_->filename.c_str());
	LOGW("aAsset_->filename:%s fopen ok!\r\n", aAsset_->filename.c_str());
    fseek(aAsset_->fp, 0, SEEK_SET);
    return aAsset_;
}
int __IMPL(AAsset_read)(AAsset* asset, void* buf, size_t count) {
    LOGW("%s, %d ", __func__, __LINE__);
    return fread(buf, 1, count, asset->fp);
}
void __IMPL(AAsset_close)(AAsset* asset) {
    LOGW("%s, %d ", __func__, __LINE__);
    fclose(asset->fp);
    delete asset;
    return ;
}
off_t __IMPL(AAsset_getLength)(AAsset* asset) {
    LOGW("%s, %d ", __func__, __LINE__);
    long  old_ = ftell(asset->fp);
    fseek(asset->fp, 0, SEEK_END);
    long len = ftell(asset->fp);
    fseek(asset->fp, old_, SEEK_SET);
    return len;
}
#else  // android
AAsset* __IMPL(AAssetManager_open)(AAssetManager* mgr, const char* filename, int mode) {
    LOGW("%s, %d ", __func__, __LINE__);
    AAsset_android *aAsset_android = AAssetManager_open__(mgr, filename, mode);
    AAsset_ *aAsset_ = new AAsset_();
    aAsset_->aAsset_android = aAsset_android;
    return aAsset_;
}

int __IMPL(AAsset_read)(AAsset* asset, void* buf, size_t count) {
    LOGW("%s, %d ", __func__, __LINE__);
    AAsset_android *aAsset_android = asset->aAsset_android;
    return AAsset_read__(aAsset_android, buf, count);
}
void __IMPL(AAsset_close)(AAsset* asset) {
    LOGW("%s, %d ", __func__, __LINE__);
    AAsset_android *aAsset_android = asset->aAsset_android;
    AAsset_close__(aAsset_android);
    delete asset;
    return ;
}
off_t __IMPL(AAsset_getLength)(AAsset* asset) {
    LOGW("%s, %d ", __func__, __LINE__);
    AAsset_android *aAsset_android = asset->aAsset_android;
    return AAsset_getLength__(aAsset_android);
}
#endif
