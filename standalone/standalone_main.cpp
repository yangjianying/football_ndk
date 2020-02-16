

#define main standalone_main
#include "main.cpp"

#if 1

void android_main_vk(struct android_app_* app);
void android_main_gles(struct android_app_* app);
void SaschaWillems_computeshader_foot(android_app_* state);
void SaschaWillems_vulkanscene(struct android_app_* app);
void SaschaWillems_computeparticles(struct android_app_* app);
void SaschaWillems_computeraytracing(struct android_app_* app);
void SaschaWillems_computedemo1(struct android_app_* app);

static PFN_android_main_ android_main_funcs[] = {
	//android_main_vk,
	//android_main_gles,
	//SaschaWillems_computeshader_foot,
	//SaschaWillems_vulkanscene,
	//SaschaWillems_computeparticles,
	//SaschaWillems_computeraytracing,
	SaschaWillems_computedemo1,
};
#define android_main_funcs_SIZE (sizeof(android_main_funcs)/sizeof(android_main_funcs[0]))

void standalone_main_run_all() {
	fprintf(stderr, "%s, android_main_funcs_SIZE:%d \r\n", __func__, (int)android_main_funcs_SIZE);
	for(int i=0;i<android_main_funcs_SIZE;i++) {
		standalone_main(android_main_funcs[i], 1, 5000);
	}
}
	

#endif

