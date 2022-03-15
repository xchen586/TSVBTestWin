#ifndef MANYCAM_VB_INCLUDE_FACTORY_H
#define MANYCAM_VB_INCLUDE_FACTORY_H

#include "frame_factory.h"
#include "pipeline.h"

namespace manycam {

class ISDKFactory : public IRelease
{
public:
	virtual IFrameFactory* createFrameFactory() = 0;
	virtual IPipeline* createPipeline() = 0;
};

#ifdef __APPLE__
typedef ISDKFactory*(*pfnCreateSDKFactory)();
#endif

#ifdef _WIN32
typedef ISDKFactory*(__cdecl *pfnCreateSDKFactory)();
#endif

}

#endif
