#ifndef __FOOTBALL_PP_FACTORY_H__
#define __FOOTBALL_PP_FACTORY_H__

#include <string>
#include "FootballPP.h"

namespace football {

class FootballPPFactory {
public:
	enum {
		PP_CPU = 0,
		PP_GLES = 1,
		PP_VK = 2,
	};
	static std::string getPPTypeDesc(int type);

	static FootballPP*createFootballPP(int type);
};

};

#endif

