#ifndef __FOOTBALL_PP_FACTORY_H__
#define __FOOTBALL_PP_FACTORY_H__

#include "FootballPP.h"

namespace football {

class FootballPPFactory {
public:
	enum {
		PP_CPU,
		PP_GLES,
		PP_VK,
	};
	static FootballPP*createFootballPP(int type);
};

};

#endif

