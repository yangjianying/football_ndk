
API_PROTO(AFootballSysApi_setAssetsBasePath, int, const char *base_path_);
API_PROTO(AFootballSysApi_create, int, AFootballSysApi **api__);
API_PROTO(AFootballSysApi_release, int, AFootballSysApi *api_);
API_PROTO(AFootballSysApi_football_daemon_thread_start, int, AFootballSysApi *api_);
API_PROTO(AFootballSysApi_football_daemon_thread_stop, int, AFootballSysApi *api_);

