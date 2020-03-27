#include <android/log.h>

#include <cassert>
#include <vector>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include <stb/stb_image.h>

#if 1
#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#endif

#include "FootballConfig.h"

#include "StbImageUtils.h"

#undef __CLASS__
#define __CLASS__ "StbImageUtils"

int vk___stbi_write_png(char const *filename, int w, int h, int comp, const void  *data, int stride_in_bytes) {
	return stbi_write_png(filename, w, h, comp, data, stride_in_bytes);
}
unsigned char *vk___stbi_load_from_memory(unsigned char const *buffer, int len, int *x, int *y, int *comp, int req_comp) {
	return stbi_load_from_memory(buffer, len, x, y, comp, req_comp);
}

void vk___stbi_image_free(void *retval_from_stbi_load) {
	stbi_image_free(retval_from_stbi_load);
}


