#include "stdafx.h"
#include "CVBHandler.h"
#include "samplegrab.h"


CVBHandler::CVBHandler()
{
	_createFactory = nullptr;
	_sdkFactory = nullptr;
	_pipeline = nullptr;
	_frameFactory = nullptr;

	_isInited = init();
}


CVBHandler::~CVBHandler()
{
	releaseResource();
}

bool CVBHandler::init()
{

	HMODULE handle = LoadLibrary(_T("mcvb.dll"));
	if (!handle) {
		return false;
	}

	_createFactory = reinterpret_cast<pfnCreateSDKFactory>(
		GetProcAddress(handle, "createSDKFactory")
		);
	if (!_createFactory) {
		return false;
	}

	_sdkFactory = _createFactory();
	if (!_sdkFactory) {
		return false;
	}

	_pipeline = _sdkFactory->createPipeline();
	if (!_pipeline) {
		return false;
	}

	_frameFactory = _sdkFactory->createFrameFactory();
	if (!_frameFactory) {
		return false;
	}
	

	PipelineError error = PipelineErrorCode::ok;
	_backgroundController = NULL;
	error = _pipeline->enableReplaceBackground(&_backgroundController);
	if ((error != PipelineErrorCode::ok)
		|| !_backgroundController) {
		IPipelineConfiguration * pConfig = _pipeline->copyConfiguration();
		if (pConfig) {
			Backend backend = (Backend)pConfig->getBackend();
			backend = Backend::CPU;
			pConfig->setBackend(backend);
			error = _pipeline->setConfiguration(pConfig);
			if (error != PipelineErrorCode::ok) {
				return false;
			}
			error = _pipeline->enableReplaceBackground(&_backgroundController);
			if ((error != PipelineErrorCode::ok)
				|| !_backgroundController) {
				return false;
			}
		}
		else {
			return false;
		}
	}

	//_pipeline->enableBlurBackground(0.8);
	//_pipeline->enableBlurBackground(1.0);

	if (_sdkFactory) {
		_sdkFactory->release();
		_sdkFactory = nullptr;
	}

	return true;
}

void CVBHandler::releaseResource()
{
	if (_sdkFactory) {
		_sdkFactory->release();
		_sdkFactory = nullptr;
	}
	if (_pipeline) {
		_pipeline->release();
		_pipeline = nullptr;
	}
	if (_frameFactory) {
		_frameFactory->release();
		_frameFactory = nullptr;
	}
}

void CVBHandler::setBackgroundWithContentOfFile(TCHAR * imagePath)
{
	if (!imagePath) {
		return;
	}
	if (!_backgroundController) {
		return;
	}

	char path[MAX_PATH];
	size_t size = wcstombs(path, imagePath, MAX_PATH);
	IFrame * background = _frameFactory->loadImage(path);
	if (background) {
		_backgroundController->setBackgroundImage(background);
	}
}

void CVBHandler::resetBackgroundImage()
{
	if (_backgroundController) {
		_backgroundController->clearBackgroundImage();
	}
}