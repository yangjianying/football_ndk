#ifndef __GLES_MAIN_H__
#define __GLES_MAIN_H__


#include "native_app_glue.h"



bool InitGles(android_app_* app);

void DeleteGles(void);

bool IsGlesReady(void);

int importAHardwareBufferAsGlesTexture(AHardwareBuffer *hardwareBuffer);
void DeleteImportedGlesTexture(int flags = 0x00);

bool GlesDrawFrame(void);


#endif

