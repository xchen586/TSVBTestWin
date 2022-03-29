#pragma once

#include "vb_sdk\sdk_factory.h"

using namespace manycam;

class CVBHandler
{
public:
	CVBHandler();
	~CVBHandler();

protected:
	bool init();
	void releaseResource();
	
protected:
	pfnCreateSDKFactory _createFactory;
	ISDKFactory * _sdkFactory;

public:
	IPipeline * _pipeline;
	IFrameFactory * _frameFactory;
	IReplacementController * _backgroundController;
	
public:
	void setBackgroundWithContentOfFile(TCHAR * imagePath);
	void resetBackgroundImage();
	bool isInitialized() {
		return _isInited;
	}

private:
	bool _isInited;
};

