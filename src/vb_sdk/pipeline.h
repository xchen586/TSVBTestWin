#ifndef MANYCAM_VB_INCLUDE_PIPELINE_H
#define MANYCAM_VB_INCLUDE_PIPELINE_H

#include "frame.h"
#include "pipeline_config.h"

namespace manycam {

enum PipelineErrorCode
{
	ok = 0,
	invalidArguemnt = 1,
	noFeaturesEnabled = 2,
	engineInitializationError = 3,
	resourceAllocationError = 4
};

typedef int PipelineError;

class IReplacementController : public IRelease
{
public:
	virtual void setBackgroundImage(const IFrame* image) = 0;
	virtual void clearBackgroundImage() = 0;
};

class IPipeline : public IRelease
{
public:
	virtual PipelineError setConfiguration(const IPipelineConfiguration* config) = 0;
	virtual IPipelineConfiguration* copyConfiguration() const = 0;
	virtual IPipelineConfiguration* copyDefaultConfiguration() const = 0;

	virtual PipelineError enableBlurBackground(float blurPower) = 0;
	virtual void disableBackgroundBlur() = 0;
	virtual bool getBlurBackgroundState(float* blurPower) const = 0;

	virtual PipelineError enableReplaceBackground(IReplacementController** controller) = 0;
	virtual void disableReplaceBackground() = 0;
	virtual bool getReplaceBackgroundState() const = 0;
	
	virtual IFrame* process(const IFrame* input, PipelineError* error) = 0;
};

}

#endif
