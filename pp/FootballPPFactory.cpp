
#include "FootballPPFactory.h"

#include "impl/FootballPPCpu.h"
#include "impl/FootballPPGles.h"
#include "impl/FootballPPVk.h"

namespace football {

/*static*/ FootballPP* FootballPPFactory::createFootballPP(int type) {
	if(PP_CPU == type) {
		return new FootballPPCpu();
	}
	else if(PP_GLES == type) {
		return new FootballPPGles();
	}
	else if(PP_VK == type) {
		return new FootballPPVk();
	}
	return nullptr;
}

};


