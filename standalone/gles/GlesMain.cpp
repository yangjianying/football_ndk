
#include <cassert>
#include <vector>

#include <android/log.h>
#include <android/asset_manager.h>
#include <android/api-level.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#if __ANDROID_API__ >= 24
#include <GLES3/gl32.h>
#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif

#include <GLES3/gl3ext.h>

#include "FootballConfig.h"


#include "StbImageUtils.h"

#include "AAssetManagerImpl_.h"

#include "GlesMain.h"

#undef __CLASS__
#define __CLASS__ "GlesMain"

// Android log function wrappers
static const char* kTAG = "native_app-gles";
#include "android_logcat_.h"


static bool initialized_ = false;
bool InitGles(android_app_* app) {
	LOGI("%s", __func__);
	DLOGD( "%s \r\n", __func__);
	initialized_ = true;

	return true;
}

void DeleteGles(void) {
	initialized_ = false;

	LOGI("%s", __func__);
	DLOGD( "%s \r\n", __func__);
	return ;
}

bool IsGlesReady(void) {
	return initialized_;
}

int importAHardwareBufferAsGlesTexture(AHardwareBuffer *hardwareBuffer) {
	LOGI("%s", __func__);
	DLOGD( "%s \r\n", __func__);
	return 0;
}
void DeleteImportedGlesTexture(int flags) {
	LOGI("%s", __func__);
	DLOGD( "%s \r\n", __func__);
	return ;
}

bool GlesDrawFrame(void) {

	LOGI("%s", __func__);
	DLOGD( "%s \r\n", __func__);
	return true;
}



