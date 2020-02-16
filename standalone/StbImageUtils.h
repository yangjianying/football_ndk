#ifndef __STBIMAGE_UTILS_H__
#define __STBIMAGE_UTILS_H__


int vk___stbi_write_png(char const *filename, int w, int h, int comp, const void  *data, int stride_in_bytes);

unsigned char *vk___stbi_load_from_memory(unsigned char const *buffer, int len, int *x, int *y, int *comp, int req_comp);

void vk___stbi_image_free(void *retval_from_stbi_load);


#endif

