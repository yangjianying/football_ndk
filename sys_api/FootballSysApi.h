#ifndef __FOOTBALL_SYSAPI_H__
#define __FOOTBALL_SYSAPI_H__


#ifdef __cplusplus
extern "C" {
#endif

struct AFootballSysApi;
typedef struct AFootballSysApi AFootballSysApi;

#ifndef FootballSysApi_NO_PROTOTYPES

int AFootballSysApi_setAssetsBasePath(const char *base_path_);
int AFootballSysApi_create(AFootballSysApi **api__);
int AFootballSysApi_release(AFootballSysApi *api_);
int AFootballSysApi_football_daemon_thread_start(AFootballSysApi *api_);
int AFootballSysApi_football_daemon_thread_stop(AFootballSysApi *api_);

#endif


#ifdef __cplusplus
};
#endif

#endif

